#ifndef GUI_WINDOW_INTERNAL_HPP
#define GUI_WINDOW_INTERNAL_HPP

#include "GuiWindow.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace gui_internal {

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

class SideDistribution {
public:
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
    NewGame,
    LoadGame,
    Exit,
};

class StartedQuickActionSpec {
public:
    const char* label;
    StartedQuickAction action;
};

constexpr std::array<StartedQuickActionSpec, 14> kStartedQuickActions = {{
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
    {"NEW GAME", StartedQuickAction::NewGame},
    {"LOAD GAME", StartedQuickAction::LoadGame},
    {"EXIT", StartedQuickAction::Exit},
}};

inline int clampInt(const int value, const int minimum, const int maximum) {
    return std::max(minimum, std::min(value, maximum));
}

inline float clampFloat(const float value, const float minimum, const float maximum) {
    return std::max(minimum, std::min(value, maximum));
}

inline Rectangle insetRect(const Rectangle& rect, const float dx, const float dy) {
    return Rectangle{rect.x + dx, rect.y + dy, rect.width - 2 * dx,
                     rect.height - 2 * dy};
}

inline bool pointInsideRect(const Rectangle& rect, const Vector2 point) {
    return CheckCollisionPointRec(point, rect);
}

inline std::string trimWhitespace(std::string text) {
    const std::string whitespace = " \t\r\n";
    const std::size_t start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }
    const std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(start, end - start + 1);
}

inline Color colorFromKey(const std::string& key) {
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

inline Color playerPieceColor(const int playerIndex) {
    constexpr std::array<Color, 4> kPlayerColors = {
        Color{211, 47, 47, 255},
        Color{30, 96, 190, 255},
        Color{33, 135, 85, 255},
        Color{231, 123, 21, 255},
    };

    if (playerIndex < 0) {
        return kPlayerColors[0];
    }

    return kPlayerColors[static_cast<std::size_t>(playerIndex) %
                         kPlayerColors.size()];
}

inline Color contrastingTextColor(const Color bg) {
    const int luminance = 299 * bg.r + 587 * bg.g + 114 * bg.b;
    return luminance > 145000 ? kInk : kWhitePanel;
}

inline std::string playerStateLabel(const PlayerSnapshot& player) {
    if (player.bankrupt) return "BANKRUPT";
    if (player.jailed) return "JAILED";
    if (player.active) return "ACTIVE";
    return "WAITING";
}

inline std::string ownerLabel(const TileSnapshot& tile) {
    if (tile.owner.empty()) {
        return "BANK";
    }
    if (tile.owner.size() <= 10) {
        return tile.owner;
    }
    return tile.owner.substr(0, 10);
}

inline std::string tileFooter(const TileSnapshot& tile) {
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

inline std::vector<std::string> splitLines(const std::string& text) {
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

inline std::string formatTransactionLogEntry(const LogSnapshot& entry) {
    std::ostringstream oss;
    oss << "[Turn " << entry.turn << "] ";
    oss << (entry.username.empty() ? "-" : entry.username);

    if (!entry.actionType.empty()) {
        oss << " | " << entry.actionType;
    }
    if (!entry.detail.empty()) {
        oss << " | " << entry.detail;
    }

    return oss.str();
}

inline std::string buildTransactionLogText(const GameSnapshot& currentSnapshot) {
    if (currentSnapshot.logs.empty()) {
        return "Belum ada transaksi/log yang tercatat.";
    }

    std::ostringstream oss;
    for (const LogSnapshot& entry : currentSnapshot.logs) {
        oss << formatTransactionLogEntry(entry) << '\n';
    }
    return oss.str();
}

inline std::string truncateText(const Font& font, const std::string& text,
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

inline std::vector<std::string> wrapText(const Font& font, const std::string& text,
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

inline void drawWrappedText(const Font& font, const std::string& text,
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

inline void drawTextCentered(const Font& font, const std::string& text,
                      const Rectangle& rect, const float fontSize,
                      const float spacing, const Color color) {
    const Vector2 size = MeasureTextEx(font, text.c_str(), fontSize, spacing);
    const Vector2 position = {rect.x + (rect.width - size.x) / 2.0F,
                              rect.y + (rect.height - size.y) / 2.0F};
    DrawTextEx(font, text.c_str(), position, fontSize, spacing, color);
}

inline void drawPanelFrame(const Rectangle& rect) {
    DrawRectangleRec(rect, kPanel);
    DrawRectangleLinesEx(rect, 1.0F, kPanelBorder);
    DrawRectangleRec(Rectangle{rect.x, rect.y, rect.width, 5.0F}, kAccent);
}

inline void drawButton(const Font& font, const Rectangle& rect, const std::string& label,
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

inline bool isButtonPressed(const Rectangle& rect, const bool enabled) {
    return enabled && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) &&
           pointInsideRect(rect, GetMousePosition());
}

inline Rectangle computeScrollbarThumb(const Rectangle& trackRect,
                                const int visibleLines,
                                const int totalLines,
                                const int startLine) {
    if (totalLines <= visibleLines || trackRect.height <= 0.0F) {
        return Rectangle{};
    }

    const int maxStartLine = std::max(0, totalLines - visibleLines);
    const float thumbHeight = std::max(
        28.0F, trackRect.height *
                   (static_cast<float>(visibleLines) /
                    static_cast<float>(std::max(visibleLines, totalLines))));
    const float thumbTravel = std::max(0.0F, trackRect.height - thumbHeight);
    const float ratio = maxStartLine > 0
                            ? static_cast<float>(startLine) /
                                  static_cast<float>(maxStartLine)
                            : 0.0F;
    return Rectangle{trackRect.x + 1.0F,
                     trackRect.y + ratio * thumbTravel,
                     trackRect.width - 2.0F,
                     thumbHeight};
}

inline Font loadSystemFontFromMemory(const std::string& path, const int baseFontSize,
                              int* codepoints, const int codepointCount) {
    std::ifstream fontFile(path, std::ios::binary);
    if (!fontFile) {
        return Font{};
    }

    fontFile.seekg(0, std::ios::end);
    const std::streamsize size = fontFile.tellg();
    fontFile.seekg(0, std::ios::beg);
    if (size <= 0) {
        return Font{};
    }

    std::vector<unsigned char> bytes(static_cast<std::size_t>(size));
    if (!fontFile.read(reinterpret_cast<char*>(bytes.data()), size)) {
        return Font{};
    }

    return LoadFontFromMemory(".ttf", bytes.data(),
                              static_cast<int>(bytes.size()), baseFontSize,
                              codepoints, codepointCount);
}

inline SideDistribution computeSideDistribution(const int totalTiles) {
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

inline TileSide resolveTileSide(const int position, const int totalTiles) {
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

inline Rectangle calculateTileRect(const Rectangle& rect, const int position,
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

inline Rectangle calculateRosterCardRect(const Rectangle& rect,
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

inline void drawPip(const Vector2 center, const float radius) {
    DrawCircleV(center, radius, Color{30, 28, 26, 255});
}

inline void drawDieFace(const Rectangle& rect, const int value, const Font& font) {
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


}  // namespace gui_internal

#endif
