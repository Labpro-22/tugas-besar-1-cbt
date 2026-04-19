#include "GuiWindow.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>
#include <sstream>
#include <utility>

namespace {

constexpr int kWindowWidth = 1520;
constexpr int kWindowHeight = 940;
constexpr Color kPaper = {244, 239, 226, 255};
constexpr Color kPanel = {249, 246, 236, 255};
constexpr Color kBoardField = {225, 234, 220, 255};
constexpr Color kAccent = {118, 10, 16, 255};
constexpr Color kAccentDark = {76, 7, 11, 255};
constexpr Color kInk = {45, 26, 22, 255};
constexpr Color kMuted = {113, 98, 88, 255};
constexpr Color kPanelBorder = {198, 186, 173, 255};
constexpr Color kWhitePanel = {255, 252, 245, 255};
constexpr Color kInspected = {243, 236, 220, 255};
constexpr Color kDisabled = {220, 214, 205, 255};

enum class TileSide {
    BottomRightCorner,
    Bottom,
    BottomLeftCorner,
    Left,
    TopLeftCorner,
    Top,
    TopRightCorner,
    Right,
};

struct SideDistribution {
    int bottomInner = 0;
    int leftInner = 0;
    int topInner = 0;
    int rightInner = 0;
};

enum class StartedQuickAction {
    PrintBoard,
    PrintProperties,
    RollDice,
    PrintLog,
    SetDice,
    PrintDeed,
    Mortgage,
    Redeem,
    Build,
    UseAbility,
    Save,
};

struct StartedQuickActionSpec {
    const char* label;
    StartedQuickAction action;
};

constexpr std::array<StartedQuickActionSpec, 11> kStartedQuickActions = {{
    {"CETAK_PAPAN", StartedQuickAction::PrintBoard},
    {"CETAK_PROPERTI", StartedQuickAction::PrintProperties},
    {"LEMPAR_DADU", StartedQuickAction::RollDice},
    {"CETAK_LOG", StartedQuickAction::PrintLog},
    {"ATUR_DADU", StartedQuickAction::SetDice},
    {"CETAK_AKTA", StartedQuickAction::PrintDeed},
    {"GADAI", StartedQuickAction::Mortgage},
    {"TEBUS", StartedQuickAction::Redeem},
    {"BANGUN", StartedQuickAction::Build},
    {"GUNAKAN_KEMAMPUAN", StartedQuickAction::UseAbility},
    {"SIMPAN", StartedQuickAction::Save},
}};

int clampInt(const int value, const int minimum, const int maximum) {
    return std::max(minimum, std::min(value, maximum));
}

float clampFloat(const float value, const float minimum, const float maximum) {
    return std::max(minimum, std::min(value, maximum));
}

Rectangle insetRect(const Rectangle& rect, const float dx, const float dy) {
    return Rectangle{rect.x + dx, rect.y + dy, rect.width - 2 * dx,
                     rect.height - 2 * dy};
}

bool pointInsideRect(const Rectangle& rect, const Vector2 point) {
    return CheckCollisionPointRec(point, rect);
}

std::string trimWhitespace(std::string text) {
    const std::string whitespace = " \t\r\n";
    const std::size_t start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }
    const std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(start, end - start + 1);
}

Color colorFromKey(const std::string& key) {
    if (key == "CK") return Color{156, 104, 71, 255};
    if (key == "BM") return Color{158, 204, 224, 255};
    if (key == "PK") return Color{235, 103, 176, 255};
    if (key == "OR") return Color{243, 145, 24, 255};
    if (key == "MR") return Color{226, 34, 34, 255};
    if (key == "KN") return Color{244, 228, 16, 255};
    if (key == "HJ") return Color{42, 161, 78, 255};
    if (key == "BT") return Color{32, 119, 201, 255};
    if (key == "AB") return Color{181, 183, 178, 255};
    if (key == "FV") return Color{234, 201, 142, 255};
    return Color{215, 222, 208, 255};
}

std::string playerStateLabel(const PlayerSnapshot& player) {
    if (player.bankrupt) return "BANKRUPT";
    if (player.jailed) return "JAILED";
    if (player.active) return "ACTIVE";
    return "WAITING";
}

std::string ownerLabel(const TileSnapshot& tile) {
    if (tile.owner.empty()) {
        return "BANK";
    }
    if (tile.owner.size() <= 10) {
        return tile.owner;
    }
    return tile.owner.substr(0, 10);
}

std::string tileFooter(const TileSnapshot& tile) {
    std::ostringstream oss;

    if (!tile.owner.empty()) {
        oss << ownerLabel(tile);
    }
    if (tile.mortgaged) {
        if (oss.tellp() > 0) {
            oss << " ";
        }
        oss << "[M]";
    }
    if (tile.buildingCount > 0) {
        if (oss.tellp() > 0) {
            oss << " ";
        }
        if (tile.buildingCount >= 5) {
            oss << "HOTEL";
        } else {
            for (int i = 0; i < tile.buildingCount; ++i) {
                oss << "^";
            }
        }
    }
    if (tile.festivalMultiplier > 1) {
        if (oss.tellp() > 0) {
            oss << " ";
        }
        oss << "x" << tile.festivalMultiplier;
    }
    return oss.str();
}

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::stringstream ss(text);
    std::string line;
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    if (text.empty() || (!text.empty() && text.back() == '\n')) {
        lines.push_back("");
    }
    if (lines.empty()) {
        lines.push_back("");
    }
    return lines;
}

std::string truncateText(const Font& font, const std::string& text,
                         const float fontSize, const float spacing,
                         const float maxWidth) {
    if (text.empty()) {
        return "";
    }

    if (MeasureTextEx(font, text.c_str(), fontSize, spacing).x <= maxWidth) {
        return text;
    }

    std::string shortened = text;
    while (!shortened.empty()) {
        shortened.pop_back();
        const std::string candidate = shortened + "...";
        if (MeasureTextEx(font, candidate.c_str(), fontSize, spacing).x <=
            maxWidth) {
            return candidate;
        }
    }

    return "...";
}

std::vector<std::string> wrapText(const Font& font, const std::string& text,
                                  const float fontSize, const float spacing,
                                  const float maxWidth,
                                  const int maxLines = 1000) {
    std::vector<std::string> wrapped;
    if (maxWidth <= 0.0F) {
        wrapped.push_back(text);
        return wrapped;
    }

    for (const std::string& rawLine : splitLines(text)) {
        std::istringstream iss(rawLine);
        std::string word;
        std::string current;

        while (iss >> word) {
            std::string candidate = current.empty() ? word : current + " " + word;
            if (MeasureTextEx(font, candidate.c_str(), fontSize, spacing).x <=
                    maxWidth ||
                current.empty()) {
                current = candidate;
                continue;
            }

            wrapped.push_back(current);
            if (static_cast<int>(wrapped.size()) >= maxLines) {
                wrapped.back() =
                    truncateText(font, wrapped.back(), fontSize, spacing, maxWidth);
                return wrapped;
            }
            current = word;
        }

        if (!current.empty()) {
            wrapped.push_back(current);
            if (static_cast<int>(wrapped.size()) >= maxLines) {
                wrapped.back() =
                    truncateText(font, wrapped.back(), fontSize, spacing, maxWidth);
                return wrapped;
            }
        } else if (rawLine.empty()) {
            wrapped.push_back("");
        }
    }

    if (wrapped.empty()) {
        wrapped.push_back("");
    }

    if (static_cast<int>(wrapped.size()) > maxLines) {
        wrapped.resize(static_cast<std::size_t>(maxLines));
        wrapped.back() =
            truncateText(font, wrapped.back(), fontSize, spacing, maxWidth);
    }

    return wrapped;
}

void drawWrappedText(const Font& font, const std::string& text,
                     const Rectangle& rect, const float fontSize,
                     const float spacing, const Color color,
                     const int maxLines = 1000) {
    const std::vector<std::string> lines =
        wrapText(font, text, fontSize, spacing, rect.width, maxLines);
    const float lineHeight = fontSize + 4.0F;
    float y = rect.y;

    for (const std::string& line : lines) {
        if (y + lineHeight > rect.y + rect.height + 1.0F) {
            break;
        }
        DrawTextEx(font, line.c_str(), Vector2{rect.x, y}, fontSize, spacing,
                   color);
        y += lineHeight;
    }
}

void drawTextRight(const Font& font, const std::string& text,
                   const Rectangle& rect, const float fontSize,
                   const float spacing, const Color color) {
    const Vector2 size = MeasureTextEx(font, text.c_str(), fontSize, spacing);
    const Vector2 position = {rect.x + rect.width - size.x,
                              rect.y + (rect.height - size.y) / 2.0F};
    DrawTextEx(font, text.c_str(), position, fontSize, spacing, color);
}

void drawTextCentered(const Font& font, const std::string& text,
                      const Rectangle& rect, const float fontSize,
                      const float spacing, const Color color) {
    const Vector2 size = MeasureTextEx(font, text.c_str(), fontSize, spacing);
    const Vector2 position = {rect.x + (rect.width - size.x) / 2.0F,
                              rect.y + (rect.height - size.y) / 2.0F};
    DrawTextEx(font, text.c_str(), position, fontSize, spacing, color);
}

void drawPanelFrame(const Rectangle& rect) {
    DrawRectangleRec(rect, kPanel);
    DrawRectangleLinesEx(rect, 1.0F, kPanelBorder);
    DrawRectangleRec(Rectangle{rect.x, rect.y, rect.width, 5.0F}, kAccent);
}

void drawButton(const Font& font, const Rectangle& rect, const std::string& label,
                const bool enabled, const bool active = false) {
    const Vector2 mouse = GetMousePosition();
    const bool hovered = enabled && pointInsideRect(rect, mouse);
    Color fill = enabled ? kWhitePanel : kDisabled;
    Color border = kPanelBorder;
    Color textColor = kAccentDark;

    if (active) {
        fill = kAccent;
        border = kAccentDark;
        textColor = kWhitePanel;
    } else if (hovered) {
        fill = Color{245, 239, 228, 255};
        border = kAccentDark;
    }

    DrawRectangleRec(rect, fill);
    DrawRectangleLinesEx(rect, 1.0F, border);
    drawTextCentered(font, label, rect, 20.0F, 1.0F,
                     enabled ? textColor : kMuted);
}

bool isButtonPressed(const Rectangle& rect, const bool enabled) {
    return enabled && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) &&
           pointInsideRect(rect, GetMousePosition());
}

SideDistribution computeSideDistribution(const int totalTiles) {
    SideDistribution distribution;
    if (totalTiles <= 4) {
        distribution.bottomInner = 1;
        distribution.leftInner = 1;
        distribution.topInner = 1;
        distribution.rightInner = 1;
        return distribution;
    }

    const int innerTiles = totalTiles - 4;
    const int base = innerTiles / 4;
    const int remainder = innerTiles % 4;
    distribution.bottomInner = std::max(1, base + (remainder > 0 ? 1 : 0));
    distribution.leftInner = std::max(1, base + (remainder > 1 ? 1 : 0));
    distribution.topInner = std::max(1, base + (remainder > 2 ? 1 : 0));
    distribution.rightInner = std::max(
        1, innerTiles - distribution.bottomInner - distribution.leftInner -
               distribution.topInner);
    return distribution;
}

TileSide resolveTileSide(const int position, const int totalTiles) {
    const SideDistribution distribution = computeSideDistribution(totalTiles);
    if (position == 0) {
        return TileSide::BottomRightCorner;
    }

    if (position <= distribution.bottomInner) {
        return TileSide::Bottom;
    }
    if (position == distribution.bottomInner + 1) {
        return TileSide::BottomLeftCorner;
    }
    if (position < distribution.bottomInner + distribution.leftInner + 2) {
        return TileSide::Left;
    }
    if (position == distribution.bottomInner + distribution.leftInner + 2) {
        return TileSide::TopLeftCorner;
    }
    if (position <
        distribution.bottomInner + distribution.leftInner + distribution.topInner +
            3) {
        return TileSide::Top;
    }
    if (position ==
        distribution.bottomInner + distribution.leftInner + distribution.topInner +
            3) {
        return TileSide::TopRightCorner;
    }
    return TileSide::Right;
}

Rectangle calculateTileRect(const Rectangle& rect, const int position,
                            const int totalTiles) {
    const SideDistribution distribution = computeSideDistribution(totalTiles);
    const int borderThickness =
        clampInt(static_cast<int>(std::min(rect.width, rect.height) / 7.0F), 64,
                 108);
    const float innerWidth = rect.width - borderThickness * 2.0F;
    const float innerHeight = rect.height - borderThickness * 2.0F;
    const float innerLeft = rect.x + borderThickness;
    const float innerRight = rect.x + rect.width - borderThickness;
    const float innerTop = rect.y + borderThickness;
    const float innerBottom = rect.y + rect.height - borderThickness;

    if (position == 0) {
        return Rectangle{innerRight, innerBottom, borderThickness * 1.0F,
                         borderThickness * 1.0F};
    }

    if (position <= distribution.bottomInner) {
        const int indexFromRight = position - 1;
        const float left =
            innerRight - ((indexFromRight + 1) * innerWidth) / distribution.bottomInner;
        const float right =
            innerRight - (indexFromRight * innerWidth) / distribution.bottomInner;
        return Rectangle{left, innerBottom, right - left,
                         rect.y + rect.height - innerBottom};
    }

    const int bottomLeftCorner = distribution.bottomInner + 1;
    if (position == bottomLeftCorner) {
        return Rectangle{rect.x, innerBottom, innerLeft - rect.x,
                         rect.y + rect.height - innerBottom};
    }

    const int leftStart = bottomLeftCorner + 1;
    const int topLeftCorner = distribution.bottomInner + distribution.leftInner + 2;
    if (position < topLeftCorner) {
        const int indexFromBottom = position - leftStart;
        const float top = innerBottom -
                          ((indexFromBottom + 1) * innerHeight) / distribution.leftInner;
        const float bottom = innerBottom -
                             (indexFromBottom * innerHeight) / distribution.leftInner;
        return Rectangle{rect.x, top, innerLeft - rect.x, bottom - top};
    }

    if (position == topLeftCorner) {
        return Rectangle{rect.x, rect.y, innerLeft - rect.x, innerTop - rect.y};
    }

    const int topStart = topLeftCorner + 1;
    const int topRightCorner =
        distribution.bottomInner + distribution.leftInner + distribution.topInner +
        3;
    if (position < topRightCorner) {
        const int indexFromLeft = position - topStart;
        const float left =
            innerLeft + (indexFromLeft * innerWidth) / distribution.topInner;
        const float right =
            innerLeft + ((indexFromLeft + 1) * innerWidth) / distribution.topInner;
        return Rectangle{left, rect.y, right - left, innerTop - rect.y};
    }

    if (position == topRightCorner) {
        return Rectangle{innerRight, rect.y, rect.x + rect.width - innerRight,
                         innerTop - rect.y};
    }

    const int rightStart = topRightCorner + 1;
    const int indexFromTop = position - rightStart;
    const float top =
        innerTop + (indexFromTop * innerHeight) / distribution.rightInner;
    const float bottom =
        innerTop + ((indexFromTop + 1) * innerHeight) / distribution.rightInner;
    return Rectangle{innerRight, top, rect.x + rect.width - innerRight,
                     bottom - top};
}

Rectangle calculateRosterCardRect(const Rectangle& rect,
                                  const GameSnapshot& currentSnapshot,
                                  const int index) {
    if (index < 0 || currentSnapshot.players.empty() ||
        index >= static_cast<int>(currentSnapshot.players.size())) {
        return Rectangle{};
    }

    const int playerCount = static_cast<int>(currentSnapshot.players.size());
    const float gap = playerCount >= 4 ? 8.0F : 10.0F;
    const float gapCount = static_cast<float>(std::max(0, playerCount - 1));
    const float cardHeight = clampFloat((rect.height - gap * gapCount) / playerCount,
                                        60.0F, 84.0F);

    return Rectangle{rect.x, rect.y + index * (cardHeight + gap), rect.width,
                     cardHeight};
}

void drawPip(const Vector2 center, const float radius) {
    DrawCircleV(center, radius, Color{30, 28, 26, 255});
}

void drawDieFace(const Rectangle& rect, const int value, const Font& font) {
    DrawRectangleRec(rect, kWhitePanel);
    DrawRectangleLinesEx(rect, 2.0F, Color{194, 182, 169, 255});

    if (value < 1 || value > 6) {
        drawTextCentered(font, "?", rect, 28.0F, 1.0F, kMuted);
        return;
    }

    const float centerX = rect.x + rect.width / 2.0F;
    const float centerY = rect.y + rect.height / 2.0F;
    const float offsetX = rect.width / 4.0F;
    const float offsetY = rect.height / 4.0F;
    const float radius = std::max(3.0F, rect.width / 16.0F);

    const Vector2 leftTop{centerX - offsetX, centerY - offsetY};
    const Vector2 rightTop{centerX + offsetX, centerY - offsetY};
    const Vector2 center{centerX, centerY};
    const Vector2 leftBottom{centerX - offsetX, centerY + offsetY};
    const Vector2 rightBottom{centerX + offsetX, centerY + offsetY};
    const Vector2 leftMid{centerX - offsetX, centerY};
    const Vector2 rightMid{centerX + offsetX, centerY};

    if (value == 1 || value == 3 || value == 5) drawPip(center, radius);
    if (value >= 2) {
        drawPip(leftTop, radius);
        drawPip(rightBottom, radius);
    }
    if (value >= 4) {
        drawPip(rightTop, radius);
        drawPip(leftBottom, radius);
    }
    if (value == 6) {
        drawPip(leftMid, radius);
        drawPip(rightMid, radius);
    }
}

}  // namespace

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
      visibleCommandIndices{{-1, -1, -1, -1, -1, -1}},
      quickButtonLabels{{"", "", "", "", "", ""}},
      quickButtonEnabled{{false, false, false, false, false, false}},
      manualEnabled(false) {}

GuiWindow::~GuiWindow() { stopSession(); }

int GuiWindow::run() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(kWindowWidth, kWindowHeight, "NIMONSPOLI GUI");
    SetTargetFPS(60);
    SetTextLineSpacing(-8);  // Slightly negative for better compact rendering

    // Load high-quality font from system with optimized size
    // Try best fonts with correct Windows font file names
    const char* fontPaths[] = {
        "C:\\Windows\\Fonts\\segoeuib.ttf",   // Segoe UI Bold
        "C:\\Windows\\Fonts\\segoeuil.ttf",   // Segoe UI Light
        "C:\\Windows\\Fonts\\segoeui.ttf",    // Segoe UI
        "C:\\Windows\\Fonts\\georgia.ttf",    // Georgia
        "C:\\Windows\\Fonts\\arial.ttf",      // Arial
    };
    bool fontLoaded = false;
    int baseFontSize = 48;  // Larger base size for better quality
    
    // Create glyph ranges for better coverage
    int codepointCount = 224;
    int* codepoints = new int[codepointCount];
    // Load printable ASCII + extended characters
    for (int i = 0; i < 224; i++) {
        codepoints[i] = 32 + i;  // ASCII 32-255
    }
    
    // Try loading each font path
    for (int fontIdx = 0; fontIdx < 5 && !fontLoaded; fontIdx++) {
        georgiaFont = LoadFontEx(fontPaths[fontIdx], baseFontSize, codepoints, codepointCount);
        if (georgiaFont.texture.id != 0 && georgiaFont.glyphCount > 0) {
            fontLoaded = true;
            // Debug output to console
            std::cout << "✓ Font loaded: " << fontPaths[fontIdx] << std::endl;
            break;
        }
    }
    
    delete[] codepoints;  // Free allocated memory
    
    if (!fontLoaded) {
        // Final fallback to default
        georgiaFont = GetFontDefault();
        std::cout << "⚠ All font loading failed, using default raylib font" << std::endl;
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

void GuiWindow::updateQuickButtons(const GameSnapshot& currentSnapshot) {
    visibleCommandIndices.fill(-1);
    quickButtonLabels.fill("-");
    quickButtonEnabled.fill(false);
    manualEnabled = currentSnapshot.gameStarted;

    if (!currentSnapshot.gameStarted) {
        commandScrollColumn = 0;
        commandScrollMaxColumn = 0;
        if (currentSnapshot.startupMode == "PLAYER_COUNT") {
            quickButtonLabels = {"2 PEMAIN", "3 PEMAIN", "4 PEMAIN", "-", "-", "-"};
            quickButtonEnabled = {true, true, true, false, false, false};
        } else if (currentSnapshot.startupMode == "LOAD_FILE") {
            quickButtonLabels = {"LOAD GAME", "-", "-", "-", "-", "-"};
            quickButtonEnabled = {false, false, false, false, false, false};
        } else if (currentSnapshot.startupMode == "USERNAME") {
            quickButtonLabels = {"INPUT NAMA", "-", "-", "-", "-", "-"};
            quickButtonEnabled = {false, false, false, false, false, false};
        } else {
            quickButtonLabels = {"NEW GAME", "LOAD GAME", "EXIT", "-", "-", "-"};
            quickButtonEnabled = {true, true, true, false, false, false};
        }
        return;
    }

    const int totalCommands = static_cast<int>(kStartedQuickActions.size());
    const int totalColumns = (totalCommands + 1) / 2;
    const int visibleColumns = 3;
    commandScrollMaxColumn = std::max(0, totalColumns - visibleColumns);
    commandScrollColumn = clampInt(commandScrollColumn, 0, commandScrollMaxColumn);

    for (int column = 0; column < visibleColumns; ++column) {
        const int topSlot = column;
        const int bottomSlot = column + 3;
        const int topIndex = (commandScrollColumn + column) * 2;
        const int bottomIndex = topIndex + 1;

        if (topIndex < totalCommands) {
            quickButtonLabels[static_cast<std::size_t>(topSlot)] =
                kStartedQuickActions[static_cast<std::size_t>(topIndex)].label;
            quickButtonEnabled[static_cast<std::size_t>(topSlot)] = true;
            visibleCommandIndices[static_cast<std::size_t>(topSlot)] = topIndex;
        }
        if (bottomIndex < totalCommands) {
            quickButtonLabels[static_cast<std::size_t>(bottomSlot)] =
                kStartedQuickActions[static_cast<std::size_t>(bottomIndex)].label;
            quickButtonEnabled[static_cast<std::size_t>(bottomSlot)] = true;
            visibleCommandIndices[static_cast<std::size_t>(bottomSlot)] = bottomIndex;
        }
    }
}

void GuiWindow::executeStartedCommand(const int specIndex) {
    if (specIndex < 0 || specIndex >= static_cast<int>(kStartedQuickActions.size())) {
        return;
    }

    const StartedQuickAction action =
        kStartedQuickActions[static_cast<std::size_t>(specIndex)].action;
    switch (action) {
        case StartedQuickAction::PrintBoard:
            submitInputLine("CETAK_PAPAN");
            return;
        case StartedQuickAction::PrintProperties:
            submitInputLine("CETAK_PROPERTI");
            return;
        case StartedQuickAction::RollDice:
            submitInputLine("LEMPAR_DADU");
            return;
        case StartedQuickAction::PrintLog:
            openLocalDialog(LocalDialogType::LogCount, "Cetak Log",
                            "Masukkan jumlah entri opsional.\nKosongkan untuk cetak semua log.");
            return;
        case StartedQuickAction::SetDice:
            openLocalDialog(LocalDialogType::SetDice, "Atur Dadu",
                            "Masukkan dua nilai dadu 1-6.\nContoh: 2 5");
            return;
        case StartedQuickAction::PrintDeed:
            submitInputLine("CETAK_AKTA");
            return;
        case StartedQuickAction::Mortgage:
            submitInputLine("GADAI");
            return;
        case StartedQuickAction::Redeem:
            submitInputLine("TEBUS");
            return;
        case StartedQuickAction::Build:
            submitInputLine("BANGUN");
            return;
        case StartedQuickAction::UseAbility:
            submitInputLine("GUNAKAN_KEMAMPUAN");
            return;
        case StartedQuickAction::Save:
            openLocalDialog(LocalDialogType::SaveFile, "Simpan Game",
                            "Masukkan nama file save.\nContoh: sesi1.txt");
            return;
    }
}

void GuiWindow::openLocalDialog(const LocalDialogType type,
                                const std::string& title,
                                const std::string& prompt,
                                const std::string& initialText) {
    std::lock_guard<std::mutex> lock(modalMutex);
    if (modal.active) {
        return;
    }

    modal = ModalState{};
    modal.active = true;
    modal.backendOwned = false;
    modal.localType = type;
    modal.title = title;
    modal.prompt = prompt;
    modal.inputText = initialText;
}

void GuiWindow::confirmLocalDialog() {
    ModalState current;
    {
        std::lock_guard<std::mutex> lock(modalMutex);
        if (!modal.active) {
            return;
        }
        current = modal;
    }

    if (current.backendOwned) {
        std::lock_guard<std::mutex> lock(modalMutex);
        modal.response.accepted = true;
        modal.response.value = current.inputText;
        modal.backendResolved = true;
        modal.active = false;
        modalCondition.notify_all();
        return;
    }

    const std::string value = trimWhitespace(current.inputText);

    switch (current.localType) {
        case LocalDialogType::ManualCommand:
            if (value.empty()) {
                std::lock_guard<std::mutex> lock(modalMutex);
                modal.errorText = "Perintah tidak boleh kosong.";
                return;
            }
            submitInputLine(value);
            break;
        case LocalDialogType::LogCount:
            if (value.empty()) {
                submitInputLine("CETAK_LOG");
            } else {
                submitInputLine("CETAK_LOG " + value);
            }
            break;
        case LocalDialogType::SetDice: {
            std::istringstream iss(value);
            int d1 = 0;
            int d2 = 0;
            std::string extra;
            if (!(iss >> d1 >> d2) || (iss >> extra) || d1 < 1 || d1 > 6 ||
                d2 < 1 || d2 > 6) {
                std::lock_guard<std::mutex> lock(modalMutex);
                modal.errorText = "Format harus dua angka 1-6. Contoh: 2 5";
                return;
            }
            submitInputLine("ATUR_DADU " + std::to_string(d1) + " " +
                            std::to_string(d2));
            break;
        }
        case LocalDialogType::SaveFile:
            if (value.empty()) {
                std::lock_guard<std::mutex> lock(modalMutex);
                modal.errorText = "Nama file tidak boleh kosong.";
                return;
            }
            submitInputLine("SIMPAN " + value);
            break;
        case LocalDialogType::None:
            break;
    }

    std::lock_guard<std::mutex> lock(modalMutex);
    modal = ModalState{};
}

void GuiWindow::cancelLocalDialog() {
    std::lock_guard<std::mutex> lock(modalMutex);
    if (!modal.active) {
        return;
    }

    if (modal.backendOwned) {
        modal.response.accepted = false;
        modal.response.value.clear();
        modal.backendResolved = true;
        modal.active = false;
        modalCondition.notify_all();
        return;
    }

    modal = ModalState{};
}

InputPromptResponse GuiWindow::requestBackendPrompt(
    const InputPromptRequest& request) {
    std::unique_lock<std::mutex> lock(modalMutex);
    modalCondition.wait(lock, [this]() { return shuttingDown || !modal.active; });
    if (shuttingDown) {
        return {};
    }

    modal = ModalState{};
    modal.active = true;
    modal.backendOwned = true;
    modal.yesNo = request.kind == InputPromptKind::YesNo;
    modal.request = request;
    modal.title = request.title.empty() ? "Input" : request.title;
    modal.prompt = request.prompt;
    modal.inputText = request.initialValue;
    modal.backendResolved = false;
    lock.unlock();

    std::unique_lock<std::mutex> waitLock(modalMutex);
    modalCondition.wait(waitLock,
                        [this]() { return shuttingDown || modal.backendResolved; });
    InputPromptResponse response = modal.response;
    modal = ModalState{};
    waitLock.unlock();
    modalCondition.notify_all();
    return response;
}

std::string GuiWindow::currentStatusText(
    const GameSnapshot& currentSnapshot) const {
    if (!currentSnapshot.gameStarted) {
        return currentSnapshot.statusText;
    }

    const int inspectedIndex = effectiveInspectedPlayerIndex(currentSnapshot);
    if (inspectedIndex >= 0 &&
        inspectedIndex < static_cast<int>(currentSnapshot.players.size())) {
        return currentSnapshot
            .players[static_cast<std::size_t>(inspectedIndex)]
            .detailText;
    }

    return currentSnapshot.statusText;
}

int GuiWindow::effectiveInspectedPlayerIndex(
    const GameSnapshot& currentSnapshot) const {
    if (!currentSnapshot.gameStarted || currentSnapshot.players.empty()) {
        return -1;
    }

    if (inspectedPlayerIndex >= 0 &&
        inspectedPlayerIndex < static_cast<int>(currentSnapshot.players.size())) {
        return inspectedPlayerIndex;
    }

    if (currentSnapshot.activePlayerIndex >= 0 &&
        currentSnapshot.activePlayerIndex <
            static_cast<int>(currentSnapshot.players.size())) {
        return currentSnapshot.activePlayerIndex;
    }

    return 0;
}

GuiWindow::Layout GuiWindow::computeLayout(const int screenWidth,
                                           const int screenHeight,
                                           const GameSnapshot& currentSnapshot) const {
    Layout layout;
    const float outerPadding = 18.0F;
    const float gap = 16.0F;
    const float headerHeight = 78.0F;
    const float commandHeight = 154.0F;
    const float availableWidth = screenWidth - outerPadding * 2.0F - gap * 2.0F;

    float sideWidth =
        clampFloat(availableWidth / 5.0F, 240.0F, 320.0F);
    if (availableWidth - sideWidth * 2.0F < 420.0F) {
        sideWidth = std::max(200.0F, (availableWidth - 420.0F) / 2.0F);
    }

    layout.headerRect =
        Rectangle{outerPadding, outerPadding, screenWidth - outerPadding * 2.0F,
                  headerHeight};

    const float contentTop = layout.headerRect.y + layout.headerRect.height + gap;
    layout.commandRect = Rectangle{outerPadding,
                                   screenHeight - outerPadding - commandHeight,
                                   screenWidth - outerPadding * 2.0F,
                                   commandHeight};
    const float contentBottom = layout.commandRect.y - gap;

    layout.leftPanelRect =
        Rectangle{outerPadding, contentTop, sideWidth, contentBottom - contentTop};
    layout.rightPanelRect = Rectangle{screenWidth - outerPadding - sideWidth,
                                      contentTop, sideWidth,
                                      contentBottom - contentTop};
    layout.boardRect = Rectangle{
        outerPadding + sideWidth + gap,
        contentTop,
        screenWidth - outerPadding * 2.0F - sideWidth * 2.0F - gap * 2.0F,
        screenHeight - outerPadding - commandHeight - gap - contentTop,
    };

    const float panelPadding = 22.0F;
    const float panelHeaderSpace = 112.0F;

    layout.statusRect = Rectangle{
        layout.leftPanelRect.x + panelPadding,
        layout.leftPanelRect.y + panelHeaderSpace,
        layout.leftPanelRect.width - panelPadding * 2.0F,
        layout.leftPanelRect.height - panelHeaderSpace - panelPadding,
    };

    const float rosterHeight = clampFloat(layout.rightPanelRect.height * 0.40F,
                                          220.0F, 336.0F);
    layout.rosterRect =
        Rectangle{layout.rightPanelRect.x + panelPadding,
                  layout.rightPanelRect.y + panelHeaderSpace,
                  layout.rightPanelRect.width - panelPadding * 2.0F, rosterHeight};

    layout.logRect = Rectangle{
        layout.rightPanelRect.x + panelPadding,
        layout.rosterRect.y + layout.rosterRect.height + 58.0F,
        layout.rightPanelRect.width - panelPadding * 2.0F,
        layout.rightPanelRect.y + layout.rightPanelRect.height -
            (layout.rosterRect.y + layout.rosterRect.height + 58.0F) - panelPadding,
    };

    const float buttonPadding = 8.0F;
    const float buttonGap = 10.0F;
    const float buttonHeight = 26.0F;
    const float rowGap = 6.0F;
    const float buttonWidth =
        (layout.commandRect.width - buttonPadding * 2.0F - buttonGap * 2.0F) / 3.0F;

    for (int index = 0; index < 6; ++index) {
        const int row = index / 3;
        const int col = index % 3;
        layout.quickButtonRects[static_cast<std::size_t>(index)] =
            Rectangle{layout.commandRect.x + buttonPadding +
                          col * (buttonWidth + buttonGap),
                      layout.commandRect.y + buttonPadding +
                          row * (buttonHeight + rowGap),
                      buttonWidth, buttonHeight};
    }

    layout.scrollLeftRect = Rectangle{layout.commandRect.x + buttonPadding,
                                      layout.commandRect.y + 72.0F, 30.0F, 20.0F};
    layout.scrollRightRect =
        Rectangle{layout.commandRect.x + layout.commandRect.width - buttonPadding -
                      30.0F,
                  layout.commandRect.y + 72.0F, 30.0F, 20.0F};
    layout.manualButtonRect = Rectangle{
        layout.commandRect.x + buttonPadding,
        layout.commandRect.y + layout.commandRect.height - buttonPadding - 32.0F,
        layout.commandRect.width - buttonPadding * 2.0F,
        32.0F,
    };

    if (currentSnapshot.gameStarted) {
        const Rectangle boardSurface = insetRect(layout.boardRect, 10.0F, 10.0F);
        const int borderThickness =
            clampInt(static_cast<int>(std::min(boardSurface.width, boardSurface.height) /
                                      7.0F),
                     64, 108);
        const Rectangle centerField{
            boardSurface.x + borderThickness,
            boardSurface.y + borderThickness,
            boardSurface.width - borderThickness * 2.0F,
            boardSurface.height - borderThickness * 2.0F,
        };

        const float buttonWidth2 =
            clampFloat(centerField.width / 3.0F, 190.0F, 310.0F);
        layout.rollButtonRect = Rectangle{
            centerField.x + (centerField.width - buttonWidth2) / 2.0F,
            centerField.y + centerField.height - 112.0F, buttonWidth2, 66.0F};
    }

    return layout;
}

void GuiWindow::updateFrame(const Layout& layout,
                            const GameSnapshot& currentSnapshot) {
    updateModalInput();

    ModalState currentModal;
    {
        std::lock_guard<std::mutex> lock(modalMutex);
        currentModal = modal;
    }

    if (currentModal.active) {
        if (!currentModal.backendOwned || !currentModal.yesNo) {
            int character = GetCharPressed();
            while (character > 0) {
                if (character >= 32 && character <= 126 &&
                    currentModal.inputText.size() < 128) {
                    std::lock_guard<std::mutex> lock(modalMutex);
                    modal.inputText.push_back(static_cast<char>(character));
                    modal.errorText.clear();
                }
                character = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                std::lock_guard<std::mutex> lock(modalMutex);
                if (!modal.inputText.empty()) {
                    modal.inputText.pop_back();
                }
                modal.errorText.clear();
            }
        }

        const Rectangle dialogRect{
            GetScreenWidth() / 2.0F - 250.0F, GetScreenHeight() / 2.0F - 130.0F,
            500.0F, 260.0F};
        const Rectangle okRect{dialogRect.x + dialogRect.width - 210.0F,
                               dialogRect.y + dialogRect.height - 58.0F, 90.0F,
                               34.0F};
        const Rectangle cancelRect{dialogRect.x + dialogRect.width - 110.0F,
                                   dialogRect.y + dialogRect.height - 58.0F, 90.0F,
                                   34.0F};

        if (currentModal.backendOwned && currentModal.yesNo) {
            if (isButtonPressed(okRect, true)) {
                std::lock_guard<std::mutex> lock(modalMutex);
                modal.response.accepted = true;
                modal.response.value = "y";
                modal.backendResolved = true;
                modal.active = false;
                modalCondition.notify_all();
            } else if (isButtonPressed(cancelRect, true)) {
                std::lock_guard<std::mutex> lock(modalMutex);
                modal.response.accepted = true;
                modal.response.value = "n";
                modal.backendResolved = true;
                modal.active = false;
                modalCondition.notify_all();
            }
        } else {
            if (IsKeyPressed(KEY_ENTER) || isButtonPressed(okRect, true)) {
                confirmLocalDialog();
            } else if (IsKeyPressed(KEY_ESCAPE) || isButtonPressed(cancelRect, true)) {
                cancelLocalDialog();
            }
        }
        return;
    }

    const Vector2 mouse = GetMousePosition();
    if (pointInsideRect(layout.rosterRect, mouse) &&
        IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        for (std::size_t i = 0; i < currentSnapshot.players.size(); ++i) {
            const Rectangle cardRect =
                calculateRosterCardRect(layout.rosterRect, currentSnapshot,
                                        static_cast<int>(i));
            if (pointInsideRect(cardRect, mouse)) {
                inspectedPlayerIndex = static_cast<int>(i);
                return;
            }
        }
    }

    if (currentSnapshot.gameStarted) {
        if (isButtonPressed(layout.rollButtonRect, true)) {
            submitInputLine("LEMPAR_DADU");
            return;
        }

        if (isButtonPressed(layout.scrollLeftRect, commandScrollColumn > 0)) {
            commandScrollColumn--;
            return;
        }
        if (isButtonPressed(layout.scrollRightRect,
                            commandScrollColumn < commandScrollMaxColumn)) {
            commandScrollColumn++;
            return;
        }

        for (std::size_t i = 0; i < layout.quickButtonRects.size(); ++i) {
            if (isButtonPressed(layout.quickButtonRects[i], quickButtonEnabled[i])) {
                executeStartedCommand(visibleCommandIndices[i]);
                return;
            }
        }

        if (isButtonPressed(layout.manualButtonRect, manualEnabled)) {
            openLocalDialog(LocalDialogType::ManualCommand, "Perintah Manual",
                            "Ketik command lengkap.\nContoh: ATUR_DADU 2 5");
            return;
        }
        return;
    }

    for (std::size_t i = 0; i < layout.quickButtonRects.size(); ++i) {
        if (!isButtonPressed(layout.quickButtonRects[i], quickButtonEnabled[i])) {
            continue;
        }

        if (currentSnapshot.startupMode == "PLAYER_COUNT") {
            if (i == 0) submitInputLine("2");
            if (i == 1) submitInputLine("3");
            if (i == 2) submitInputLine("4");
            return;
        }

        if (i == 0) submitInputLine("1");
        if (i == 1) submitInputLine("2");
        if (i == 2) submitInputLine("0");
        return;
    }
}

void GuiWindow::updateModalInput() {}

void GuiWindow::drawFrame(const Layout& layout,
                          const GameSnapshot& currentSnapshot) {
    drawHeader(layout, currentSnapshot);
    drawPanel(layout.leftPanelRect, "ESTATE OFFICE",
              currentSnapshot.gameStarted ? "Archivist's Terminal"
                                          : "Session Setup");
    drawPanel(layout.rightPanelRect, "GRUP TAKTIS", "Estate standings");
    drawPanel(layout.commandRect, "", "");
    drawStatusPanel(layout, currentSnapshot);
    drawBoard(layout, currentSnapshot);
    drawRoster(layout, currentSnapshot);
    drawLogPanel(layout);
    drawActionBar(layout, currentSnapshot);
    drawModal();
}

void GuiWindow::drawHeader(const Layout& layout,
                           const GameSnapshot& currentSnapshot) {
    const Font& font = georgiaFont;
    DrawRectangleRec(layout.headerRect, kPanel);
    DrawRectangleRec(
        Rectangle{layout.headerRect.x, layout.headerRect.y + layout.headerRect.height -
                                           1.0F,
                  layout.headerRect.width, 1.0F},
        Color{204, 191, 178, 255});

    drawTextCentered(font, "", layout.headerRect, 20.0F, 1.0F, kInk);
    DrawTextEx(font, "The Grand Tactician",
               Vector2{layout.headerRect.x + 28.0F, layout.headerRect.y + 22.0F},
               30.0F, 1.0F, kAccentDark);

    const std::string navText = currentSnapshot.gameStarted
                                    ? "ESTATE LEDGER      THE BOARD      CARTOGRAPHY      ARCHIVES"
                                    : "NEW GAME      LOAD GAME      CONFIGURATION      ARCHIVES";
    DrawTextEx(font, navText.c_str(),
               Vector2{layout.headerRect.x + layout.headerRect.width / 2.0F - 250.0F,
                       layout.headerRect.y + 28.0F},
               18.0F, 1.0F, kMuted);

    DrawRectangleRec(Rectangle{layout.headerRect.x + layout.headerRect.width / 2.0F +
                                   10.0F,
                               layout.headerRect.y + layout.headerRect.height - 8.0F,
                               150.0F, 4.0F},
                     kAccent);

    if (currentSnapshot.gameStarted && currentSnapshot.activePlayerIndex >= 0 &&
        currentSnapshot.activePlayerIndex <
            static_cast<int>(currentSnapshot.players.size())) {
        const std::string turnText =
            "Giliran: " +
            currentSnapshot
                .players[static_cast<std::size_t>(currentSnapshot.activePlayerIndex)]
                .name;
        drawTextRight(font, turnText,
                      Rectangle{layout.headerRect.x + layout.headerRect.width - 280.0F,
                                layout.headerRect.y + 12.0F, 180.0F, 52.0F},
                      14.0F, 1.0F, kAccentDark);
    }

    DrawTextEx(font, "[ ] #",
               Vector2{layout.headerRect.x + layout.headerRect.width - 72.0F,
                       layout.headerRect.y + 22.0F},
               26.0F, 1.0F, kAccentDark);
}

void GuiWindow::drawPanel(const Rectangle& rect, const std::string& title,
                          const std::string& subtitle) const {
    const Font& font = georgiaFont;
    drawPanelFrame(rect);

    if (!title.empty()) {
        const Rectangle titleRect = {rect.x + 18.0F, rect.y + 12.0F,
                                     rect.width - 36.0F, 70.0F};
        drawWrappedText(font, title, titleRect, 24.0F, 1.0F, kAccentDark, 2);
    }

    if (!subtitle.empty()) {
        DrawTextEx(font, subtitle.c_str(),
                   Vector2{rect.x + 18.0F, rect.y + 64.0F}, 18.0F, 1.0F, kMuted);
    }
}

void GuiWindow::drawStatusPanel(const Layout& layout,
                                const GameSnapshot& currentSnapshot) const {
    const Font& font = georgiaFont;
    BeginScissorMode(static_cast<int>(layout.statusRect.x),
                     static_cast<int>(layout.statusRect.y),
                     static_cast<int>(layout.statusRect.width),
                     static_cast<int>(layout.statusRect.height));
    drawWrappedText(font, currentStatusText(currentSnapshot), layout.statusRect, 18.0F,
                    1.0F, kInk, 40);
    EndScissorMode();
}

void GuiWindow::drawRoster(const Layout& layout,
                           const GameSnapshot& currentSnapshot) const {
    const Font& font = georgiaFont;
    if (currentSnapshot.players.empty()) {
        drawTextCentered(font, "Belum ada pemain aktif.", layout.rosterRect, 18.0F,
                         1.0F, kMuted);
        return;
    }

    const bool compactRoster = currentSnapshot.players.size() >= 3;

    for (std::size_t i = 0; i < currentSnapshot.players.size(); ++i) {
        const Rectangle cardRect = calculateRosterCardRect(
            layout.rosterRect, currentSnapshot, static_cast<int>(i));
        const PlayerSnapshot& player = currentSnapshot.players[i];
        const bool isActive =
            static_cast<int>(i) == currentSnapshot.activePlayerIndex;
        const bool isInspected = static_cast<int>(i) == inspectedPlayerIndex;

        Color cardColor = player.bankrupt ? kDisabled : kWhitePanel;
        Color textColor = kInk;
        Color detailColor = kMuted;
        if (isActive) {
            cardColor = kAccent;
            textColor = kWhitePanel;
            detailColor = kWhitePanel;
        } else if (isInspected) {
            cardColor = kInspected;
        }

        DrawRectangleRec(cardRect, cardColor);
        DrawRectangleLinesEx(cardRect, isInspected ? 2.0F : 1.0F,
                             isActive || isInspected ? kAccentDark : kPanelBorder);

        const Rectangle nameRect{cardRect.x + 14.0F, cardRect.y + 8.0F,
                                 cardRect.width - 28.0F, 24.0F};
        const std::string nameText =
            truncateText(font, player.name, compactRoster ? 18.0F : 22.0F, 1.0F,
                         nameRect.width);
        DrawTextEx(font, nameText.c_str(), Vector2{nameRect.x, nameRect.y},
                   compactRoster ? 18.0F : 22.0F, 1.0F, textColor);

        if (compactRoster) {
            std::ostringstream summary;
            summary << "Rp " << player.cash << " | " << playerStateLabel(player)
                    << " | " << player.propertyCount << "p | " << player.cardCount
                    << "k";
            const std::string summaryText =
                truncateText(font, summary.str(), 15.0F, 1.0F, cardRect.width - 28.0F);
            DrawTextEx(font, summaryText.c_str(),
                       Vector2{cardRect.x + 14.0F, cardRect.y + 36.0F}, 15.0F, 1.0F,
                       detailColor);
        } else {
            const std::string cashText = "Rp " + std::to_string(player.cash);
            DrawTextEx(font, cashText.c_str(),
                       Vector2{cardRect.x + 14.0F, cardRect.y + 36.0F}, 18.0F, 1.0F,
                       textColor);
            std::ostringstream detail;
            detail << playerStateLabel(player) << " | " << player.propertyCount
                   << " properti | " << player.cardCount << " kartu";
            const std::string detailText =
                truncateText(font, detail.str(), 14.0F, 1.0F, cardRect.width - 28.0F);
            DrawTextEx(font, detailText.c_str(),
                       Vector2{cardRect.x + 14.0F, cardRect.y + 58.0F}, 14.0F, 1.0F,
                       detailColor);
        }
    }
}

void GuiWindow::drawBoard(const Layout& layout,
                          const GameSnapshot& currentSnapshot) const {
    const Font& font = georgiaFont;
    DrawRectangleRec(layout.boardRect, kAccent);
    const Rectangle boardSurface = insetRect(layout.boardRect, 10.0F, 10.0F);
    DrawRectangleRec(boardSurface, kPanel);
    DrawRectangleLinesEx(boardSurface, 2.0F, kAccentDark);

    if (!hasSnapshot || currentSnapshot.tiles.empty()) {
        drawTextCentered(font, "Menunggu sesi game...", boardSurface, 28.0F, 1.0F,
                         kAccentDark);
        return;
    }

    const int totalTiles = static_cast<int>(currentSnapshot.tiles.size());
    const int borderThickness =
        clampInt(static_cast<int>(std::min(boardSurface.width, boardSurface.height) /
                                  7.0F),
                 64, 108);
    const Rectangle centerField{boardSurface.x + borderThickness,
                                boardSurface.y + borderThickness,
                                boardSurface.width - borderThickness * 2.0F,
                                boardSurface.height - borderThickness * 2.0F};

    DrawRectangleRec(centerField, kBoardField);
    DrawRectangleLinesEx(centerField, 1.0F, Color{184, 173, 160, 255});

    for (const TileSnapshot& tile : currentSnapshot.tiles) {
        const Rectangle tileRect =
            calculateTileRect(boardSurface, tile.position, totalTiles);
        const TileSide side = resolveTileSide(tile.position, totalTiles);

        DrawRectangleRec(tileRect, Color{252, 249, 242, 255});
        DrawRectangleLinesEx(tileRect, 1.0F, Color{60, 53, 46, 255});

        Rectangle stripRect = tileRect;
        constexpr float stripThickness = 16.0F;
        switch (side) {
            case TileSide::BottomRightCorner:
            case TileSide::Bottom:
            case TileSide::BottomLeftCorner:
                stripRect.height = stripThickness;
                break;
            case TileSide::Left:
                stripRect.x = stripRect.x + stripRect.width - stripThickness;
                stripRect.width = stripThickness;
                break;
            case TileSide::TopLeftCorner:
            case TileSide::Top:
            case TileSide::TopRightCorner:
                stripRect.y = stripRect.y + stripRect.height - stripThickness;
                stripRect.height = stripThickness;
                break;
            case TileSide::Right:
                stripRect.width = stripThickness;
                break;
        }
        DrawRectangleRec(stripRect, colorFromKey(tile.colorKey));

        drawTextCentered(font, tile.code,
                         Rectangle{tileRect.x + 6.0F, tileRect.y + 4.0F,
                                   tileRect.width - 12.0F, 18.0F},
                         16.0F, 1.0F, kAccentDark);

        const std::string nameText =
            truncateText(font, tile.name, 14.0F, 1.0F, tileRect.width - 10.0F);
        drawTextCentered(font, nameText,
                         Rectangle{tileRect.x + 5.0F, tileRect.y + 24.0F,
                                   tileRect.width - 10.0F, tileRect.height - 42.0F},
                         14.0F, 1.0F, kInk);

        const std::string footer = tileFooter(tile);
        if (!footer.empty() && tileRect.width >= 52.0F) {
            const std::string footerText =
                truncateText(font, footer, 12.0F, 1.0F, tileRect.width - 10.0F);
            drawTextCentered(font, footerText,
                             Rectangle{tileRect.x + 5.0F,
                                       tileRect.y + tileRect.height - 20.0F,
                                       tileRect.width - 10.0F, 16.0F},
                             12.0F, 1.0F, kMuted);
        }
    }

    const std::array<Color, 4> playerColors = {
        Color{211, 47, 47, 255}, Color{30, 96, 190, 255},
        Color{33, 135, 85, 255}, Color{231, 123, 21, 255}};
    for (std::size_t i = 0; i < currentSnapshot.players.size(); ++i) {
        const PlayerSnapshot& player = currentSnapshot.players[i];
        const Rectangle tileRect =
            calculateTileRect(boardSurface, player.position, totalTiles);
        const float markerSize = 18.0F;
        const float markerGap = 5.0F;
        const int row = static_cast<int>(i) / 2;
        const int column = static_cast<int>(i) % 2;
        const float markerX = tileRect.x + 7.0F + column * (markerSize + markerGap);
        const float markerY = tileRect.y + 7.0F + row * (markerSize + markerGap);
        DrawCircle(markerX + markerSize / 2.0F, markerY + markerSize / 2.0F,
                   markerSize / 2.0F,
                   playerColors[i % playerColors.size()]);
        drawTextCentered(font, std::to_string(static_cast<int>(i + 1)),
                         Rectangle{markerX, markerY, markerSize, markerSize}, 13.0F,
                         1.0F, kWhitePanel);
    }

    DrawTextEx(font, "NIMONSPOLI",
               Vector2{centerField.x + centerField.width / 2.0F - 185.0F,
                       centerField.y + 70.0F},
               52.0F, 2.0F, kAccentDark);

    std::ostringstream info;
    if (currentSnapshot.gameStarted) {
        info << "TURN " << currentSnapshot.currentTurn << " / "
             << currentSnapshot.maxTurn << "\n";
        if (currentSnapshot.activePlayerIndex >= 0 &&
            currentSnapshot.activePlayerIndex <
                static_cast<int>(currentSnapshot.players.size())) {
            info << "Aktif: "
                 << currentSnapshot
                        .players[static_cast<std::size_t>(
                            currentSnapshot.activePlayerIndex)]
                        .name
                 << "\n";
        }
        info << "Papan aktif: " << totalTiles << " petak";
    } else {
        info << currentSnapshot.startupPrompt << "\n";
        info << "Konfigurasi papan: " << totalTiles << " petak";
    }
    drawWrappedText(font, info.str(),
                    Rectangle{centerField.x + centerField.width / 2.0F - 150.0F,
                              centerField.y + 150.0F, 300.0F, 120.0F},
                    22.0F, 1.0F, kMuted, 4);

    if (currentSnapshot.gameStarted) {
        const float diceSize = clampFloat(centerField.width / 8.0F, 64.0F, 88.0F);
        const float diceGap = clampFloat(diceSize / 3.0F, 18.0F, 28.0F);
        const float diceTop =
            std::max(centerField.y + 210.0F,
                     layout.rollButtonRect.y - diceSize - 30.0F);
        const Rectangle dieOneRect{
            centerField.x + (centerField.width - (diceSize * 2.0F + diceGap)) / 2.0F,
            diceTop, diceSize, diceSize};
        const Rectangle dieTwoRect{
            dieOneRect.x + dieOneRect.width + diceGap, diceTop, diceSize, diceSize};

        drawDieFace(dieOneRect, currentSnapshot.hasDiceResult ? currentSnapshot.die1 : 0, font);
        drawDieFace(dieTwoRect, currentSnapshot.hasDiceResult ? currentSnapshot.die2 : 0, font);

        drawButton(font, layout.rollButtonRect, "KOCOK DADU", true, true);
    }
}

void GuiWindow::drawActionBar(const Layout& layout,
                              const GameSnapshot& currentSnapshot) const {
    const Font& font = georgiaFont;
    for (std::size_t i = 0; i < layout.quickButtonRects.size(); ++i) {
        drawButton(font, layout.quickButtonRects[i], quickButtonLabels[i],
                   quickButtonEnabled[i], false);
    }

    if (currentSnapshot.gameStarted) {
        drawButton(font, layout.scrollLeftRect, "<", commandScrollColumn > 0, false);
        drawButton(font, layout.scrollRightRect, ">", commandScrollColumn < commandScrollMaxColumn,
                   false);

        std::ostringstream scrollInfo;
        const int firstVisible = commandScrollColumn * 2 + 1;
        const int lastVisible = std::min(
            static_cast<int>(kStartedQuickActions.size()), firstVisible + 5);
        scrollInfo << firstVisible << "-" << lastVisible << " / "
                   << kStartedQuickActions.size();
        drawTextCentered(font, scrollInfo.str(),
                         Rectangle{layout.scrollLeftRect.x + layout.scrollLeftRect.width +
                                       10.0F,
                                   layout.scrollLeftRect.y,
                                   layout.commandRect.width - 96.0F, 20.0F},
                         14.0F, 1.0F, kMuted);
    }

    drawButton(font, layout.manualButtonRect,
               manualEnabled ? "PERINTAH MANUAL" : "PERINTAH MANUAL NONAKTIF",
               manualEnabled, false);
}

void GuiWindow::drawLogPanel(const Layout& layout) const {
    const Font& font = georgiaFont;
    DrawTextEx(font, "CATATAN TRANSAKSI",
               Vector2{layout.rightPanelRect.x + 22.0F,
                       layout.rosterRect.y + layout.rosterRect.height + 10.0F},
               20.0F, 1.0F, kAccentDark);

    std::string outputCopy;
    {
        std::lock_guard<std::mutex> lock(outputMutex);
        outputCopy = outputText;
    }

    const auto lines = wrapText(font, trimWhitespace(outputCopy), 15.0F, 1.0F,
                                layout.logRect.width, 120);
    const float lineHeight = 19.0F;
    const int visibleLines = std::max(1, static_cast<int>(layout.logRect.height / lineHeight));
    const int startLine =
        std::max(0, static_cast<int>(lines.size()) - visibleLines);

    BeginScissorMode(static_cast<int>(layout.logRect.x),
                     static_cast<int>(layout.logRect.y),
                     static_cast<int>(layout.logRect.width),
                     static_cast<int>(layout.logRect.height));
    float y = layout.logRect.y;
    for (int i = startLine; i < static_cast<int>(lines.size()); ++i) {
        DrawTextEx(font, lines[static_cast<std::size_t>(i)].c_str(),
                   Vector2{layout.logRect.x, y}, 15.0F, 1.0F, kInk);
        y += lineHeight;
    }
    EndScissorMode();
}

void GuiWindow::drawModal() const {
    ModalState current;
    {
        std::lock_guard<std::mutex> lock(modalMutex);
        current = modal;
    }

    if (!current.active) {
        return;
    }

    const Font& font = georgiaFont;
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 110});

    const Rectangle dialogRect{GetScreenWidth() / 2.0F - 250.0F,
                               GetScreenHeight() / 2.0F - 130.0F, 500.0F, 260.0F};
    const Rectangle promptRect{dialogRect.x + 18.0F, dialogRect.y + 52.0F,
                               dialogRect.width - 36.0F, 82.0F};
    const Rectangle inputRect{dialogRect.x + 18.0F, dialogRect.y + 144.0F,
                              dialogRect.width - 36.0F, 34.0F};
    const Rectangle okRect{dialogRect.x + dialogRect.width - 210.0F,
                           dialogRect.y + dialogRect.height - 58.0F, 90.0F, 34.0F};
    const Rectangle cancelRect{dialogRect.x + dialogRect.width - 110.0F,
                               dialogRect.y + dialogRect.height - 58.0F, 90.0F, 34.0F};

    DrawRectangleRec(dialogRect, kWhitePanel);
    DrawRectangleLinesEx(dialogRect, 2.0F, kAccentDark);
    DrawTextEx(font, current.title.c_str(),
               Vector2{dialogRect.x + 18.0F, dialogRect.y + 18.0F}, 24.0F, 1.0F,
               kAccentDark);
    drawWrappedText(font, current.prompt, promptRect, 18.0F, 1.0F, kInk, 5);

    if (!current.yesNo || !current.backendOwned) {
        DrawRectangleRec(inputRect, kPanel);
        DrawRectangleLinesEx(inputRect, 1.0F, kPanelBorder);
        const std::string displayText = current.inputText.empty() ? " " : current.inputText;
        DrawTextEx(font, displayText.c_str(),
                   Vector2{inputRect.x + 8.0F, inputRect.y + 8.0F}, 18.0F, 1.0F,
                   kInk);
    }

    if (!current.errorText.empty()) {
        DrawTextEx(font, current.errorText.c_str(),
                   Vector2{dialogRect.x + 18.0F, dialogRect.y + dialogRect.height - 86.0F},
                   14.0F, 1.0F, Color{180, 40, 40, 255});
    }

    drawButton(font, okRect, current.yesNo && current.backendOwned ? "YA" : "OK",
               true, current.yesNo && current.backendOwned);
    drawButton(font, cancelRect,
               current.yesNo && current.backendOwned ? "TIDAK" : "BATAL", true,
               false);
}

