#include "gui/GuiWindowCore.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

bool GuiWindow::shouldShowErrorPopup(const std::string& text) {
    return text.find("[ERROR]") != std::string::npos ||
           text.find("[FATAL]") != std::string::npos ||
           text.rfind("Error:", 0) == 0 ||
           text.find("\nError:") != std::string::npos;
}

GuiWindow::GuiWindow()
    : inputBuffer(),
      outputBuffer(),
      streamRedirector(),
      backendSession(),
      sessionThread(),
      snapshotMutex(),
      snapshot(),
      hasSnapshot(false),
      outputMutex(),
      outputText(),
      modalMutex(),
      modalCondition(),
      modal(),
      pendingErrorMutex(),
      pendingErrorPopups(),
      shuttingDown(false),
      exitRequested(false),
      inspectedPlayerIndex(-1),
      commandScrollColumn(0),
      commandScrollMaxColumn(0),
      logScrollLine(0),
      logAutoScroll(true),
      logScrollbarDragging(false),
      logScrollbarGrabOffset(0.0F),
      visibleCommandIndices{{-1, -1, -1, -1, -1, -1}},
      quickButtonLabels{{"", "", "", "", "", ""}},
      quickButtonEnabled{{false, false, false, false, false, false}},
      manualEnabled(false),
      gameOverPopupDismissed(false),
      lastAnnouncedActivePlayerIndex(-1),
      turnPopupPlayerIndex(-1),
      turnPopupPlayerName(),
      turnPopupTimer(0.0F),
      georgiaFont{},
      modalPosition{0.0F, 0.0F},
      modalDragOffset{0.0F, 0.0F},
      modalDragging(false),
      modalPositionInitialized(false) {}

GuiWindow::~GuiWindow() { stopSession(); }

int GuiWindow::run() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(GuiWindowInternal::kWindowWidth, GuiWindowInternal::kWindowHeight, "NIMONSPOLI GUI");
    SetTargetFPS(60);
    SetTextLineSpacing(-8);

    // Load high-quality font from system with optimized size
    const char* fontPaths[] = {
        // Windows paths
        "C:/Windows/Fonts/georgia.ttf",
        "C:/Windows/Fonts/georgiab.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        // Linux / WSL paths
        "/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSerif.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
    };
    bool fontLoaded = false;
    const int baseFontSize = 48;
    for (int fontIdx = 0; fontIdx < 10 && !fontLoaded; fontIdx++) {
        georgiaFont =
            GuiWindowInternal::loadSystemFontFromMemory(fontPaths[fontIdx], baseFontSize, nullptr, 0);
        if (georgiaFont.texture.id != 0 && georgiaFont.glyphCount > 0) {
            fontLoaded = true;
            SetTextureFilter(georgiaFont.texture, TEXTURE_FILTER_BILINEAR);
            std::cout << "✓ Font loaded: " << fontPaths[fontIdx] << std::endl;
            break;
        }
    }

    outputBuffer.setCallback(
        [this](const std::string& text) { appendOutput(text); });

    backendSession.setSnapshotCallback(
        [this](const GameSnapshot& nextSnapshot) { applySnapshot(nextSnapshot); });

    InputHandler::setPromptProvider(
        [this](const InputPromptRequest& request) {
            return requestBackendPrompt(request);
        });

    streamRedirector =
        std::make_unique<StdStreamRedirector>(inputBuffer, outputBuffer);
    startSession();

    while (!WindowShouldClose() && !exitRequested.load()) {
        GameSnapshot currentSnapshot;
        {
            std::lock_guard<std::mutex> lock(snapshotMutex);
            currentSnapshot = snapshot;
        }

        if (!currentSnapshot.gameStarted) {
            inspectedPlayerIndex = -1;
        } else if (effectiveInspectedPlayerIndex(currentSnapshot) < 0) {
            inspectedPlayerIndex = currentSnapshot.activePlayerIndex;
        }

        updateQuickButtons(currentSnapshot);
        const Layout layout =
            computeLayout(GetScreenWidth(), GetScreenHeight(), currentSnapshot);
        updateFrame(layout, currentSnapshot);

        BeginDrawing();
        ClearBackground(GuiWindowInternal::kPaper);
        drawFrame(layout, currentSnapshot);
        drawTurnChangePopup(currentSnapshot);
        drawGameOverPopup(currentSnapshot);
        EndDrawing();
    }

    stopSession();
    streamRedirector.reset();
    InputHandler::clearPromptProvider();
    
    // Unload custom font if it was loaded (not the default)
    if (georgiaFont.texture.id != 0) {
        Font defaultFont = GetFontDefault();
        // Check if it's not the default font before unloading
        if (georgiaFont.texture.id != defaultFont.texture.id && 
            georgiaFont.glyphCount > 0) {
            UnloadFont(georgiaFont);
        }
    }
    
    CloseWindow();
    return 0;
}

void GuiWindow::startSession() {
    sessionThread = std::thread([this]() { backendSession.run(); });
}

void GuiWindow::stopSession() {
    if (shuttingDown) {
        return;
    }

    shuttingDown = true;
    inputBuffer.close();
    backendSession.requestStop();

    {
        std::lock_guard<std::mutex> lock(modalMutex);
        modal.active = false;
        modal.backendResolved = true;
    }
    modalCondition.notify_all();

    if (sessionThread.joinable()) {
        sessionThread.join();
    }
}

void GuiWindow::submitInputLine(const std::string& line) {
    if (line.empty()) {
        return;
    }

    appendOutput("> " + line + "\n");
    inputBuffer.submitLine(line);
}

void GuiWindow::appendOutput(const std::string& text) {
    if (text.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(outputMutex);
    outputText += text;
    constexpr std::size_t kMaxOutputSize = 40000;
    if (outputText.size() > kMaxOutputSize) {
        outputText.erase(0, outputText.size() - kMaxOutputSize);
    }

    queueErrorPopupIfNeeded(text);
}

void GuiWindow::queueErrorPopupIfNeeded(const std::string& text) {
    if (!shouldShowErrorPopup(text)) {
        return;
    }

    std::string message = GuiWindowInternal::trimWhitespace(text);
    if (message.empty()) {
        return;
    }

    constexpr std::size_t kMaxErrorPopups = 8;
    std::lock_guard<std::mutex> lock(pendingErrorMutex);
    if (pendingErrorPopups.size() >= kMaxErrorPopups) {
        pendingErrorPopups.erase(pendingErrorPopups.begin());
    }
    pendingErrorPopups.push_back(message);
}

void GuiWindow::openPendingErrorPopup() {
    {
        std::lock_guard<std::mutex> modalLock(modalMutex);
        if (modal.active) {
            return;
        }
    }

    std::string message;
    {
        std::lock_guard<std::mutex> errorLock(pendingErrorMutex);
        if (pendingErrorPopups.empty()) {
            return;
        }
        message = pendingErrorPopups.front();
        pendingErrorPopups.erase(pendingErrorPopups.begin());
    }

    std::lock_guard<std::mutex> modalLock(modalMutex);
    if (modal.active) {
        std::lock_guard<std::mutex> errorLock(pendingErrorMutex);
        pendingErrorPopups.insert(pendingErrorPopups.begin(), message);
        return;
    }

    modal = ModalState{};
    modal.active = true;
    modal.backendOwned = false;
    modal.localType = LocalDialogType::ErrorMessage;
    modal.title = "Error";
    modal.prompt = message;
    modalDragging = false;
    modalPositionInitialized = false;
}

void GuiWindow::applySnapshot(const GameSnapshot& nextSnapshot) {
    std::lock_guard<std::mutex> lock(snapshotMutex);

    if (!nextSnapshot.gameOver) {
        gameOverPopupDismissed = false;
    } else if (!hasSnapshot || !snapshot.gameOver) {
        gameOverPopupDismissed = false;
    }

    if (nextSnapshot.gameStarted &&
        nextSnapshot.activePlayerIndex >= 0 &&
        nextSnapshot.activePlayerIndex <
            static_cast<int>(nextSnapshot.players.size()) &&
        hasSnapshot && snapshot.gameStarted &&
        nextSnapshot.activePlayerIndex != lastAnnouncedActivePlayerIndex) {
        turnPopupPlayerIndex = nextSnapshot.activePlayerIndex;
        turnPopupPlayerName =
            nextSnapshot
                .players[static_cast<std::size_t>(nextSnapshot.activePlayerIndex)]
                .name;
        turnPopupTimer = 2.2F;
    }

    if (nextSnapshot.gameStarted) {
        lastAnnouncedActivePlayerIndex = nextSnapshot.activePlayerIndex;
    } else {
        lastAnnouncedActivePlayerIndex = -1;
        turnPopupPlayerIndex = -1;
        turnPopupPlayerName.clear();
        turnPopupTimer = 0.0F;
    }

    snapshot = nextSnapshot;
    hasSnapshot = true;
}

