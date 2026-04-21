#include "GuiWindow.hpp"
#include "GuiWindowInternal.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>



using namespace gui_internal;

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
      shuttingDown(false),
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
      georgiaFont{},
      modalPosition{0.0F, 0.0F},
      modalDragOffset{0.0F, 0.0F},
      modalDragging(false),
      modalPositionInitialized(false) {}

GuiWindow::~GuiWindow() { stopSession(); }

int GuiWindow::run() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(kWindowWidth, kWindowHeight, "NIMONSPOLI GUI");
    SetTargetFPS(60);
    SetTextLineSpacing(-8);

    // Load high-quality font from system with optimized size
    // Try best fonts with correct Windows font file names
    const char* fontPaths[] = {
        "C:/Windows/Fonts/georgia.ttf",
        "C:/Windows/Fonts/georgiab.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
    };
    bool fontLoaded = false;
    const int baseFontSize = 48;
    for (int fontIdx = 0; fontIdx < 4 && !fontLoaded; fontIdx++) {
        georgiaFont =
            loadSystemFontFromMemory(fontPaths[fontIdx], baseFontSize, nullptr, 0);
        if (georgiaFont.texture.id != 0 && georgiaFont.glyphCount > 0) {
            fontLoaded = true;
            SetTextureFilter(georgiaFont.texture, TEXTURE_FILTER_BILINEAR);
            std::cout << "âœ“ Font loaded: " << fontPaths[fontIdx] << std::endl;
            break;
        }
    }
    
    
    if (!fontLoaded) {
        georgiaFont = GetFontDefault();
        SetTextureFilter(georgiaFont.texture, TEXTURE_FILTER_BILINEAR);
        std::cout << "âš  All font loading failed, using default raylib font" << std::endl;
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

    while (!WindowShouldClose()) {
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
        ClearBackground(kPaper);
        drawFrame(layout, currentSnapshot);
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
}

void GuiWindow::applySnapshot(const GameSnapshot& nextSnapshot) {
    std::lock_guard<std::mutex> lock(snapshotMutex);
    snapshot = nextSnapshot;
    hasSnapshot = true;
}

