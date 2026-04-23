#include "gui/GuiWindowInternal.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>

const int GuiWindowInternal::kWindowWidth = 1520;
const int GuiWindowInternal::kWindowHeight = 940;
const Color GuiWindowInternal::kPaper = {249, 243, 232, 255};
const Color GuiWindowInternal::kPanel = {252, 249, 240, 255};
const Color GuiWindowInternal::kBoardField = {225, 234, 220, 255};
const Color GuiWindowInternal::kAccent = {96, 0, 0, 255};
const Color GuiWindowInternal::kAccentDark = {80, 0, 0, 255};
const Color GuiWindowInternal::kInk = {64, 26, 22, 255};
const Color GuiWindowInternal::kMuted = {143, 128, 118, 255};
const Color GuiWindowInternal::kPanelBorder = {198, 186, 173, 255};
const Color GuiWindowInternal::kWhitePanel = {255, 254, 250, 255};
const Color GuiWindowInternal::kInspected = {243, 236, 220, 255};
const Color GuiWindowInternal::kDisabled = {220, 214, 205, 255};
const Color GuiWindowInternal::kGold = {212, 175, 55, 255};
const Color GuiWindowInternal::kCream = {249, 243, 232, 255};

const std::array<StartedQuickActionSpec, 14>
    GuiWindowInternal::kStartedQuickActions = {{
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

int GuiWindowInternal::clampInt(const int value, const int minimum,
                                const int maximum) {
    return std::max(minimum, std::min(value, maximum));
}

float GuiWindowInternal::clampFloat(const float value, const float minimum,
                                    const float maximum) {
    return std::max(minimum, std::min(value, maximum));
}

Rectangle GuiWindowInternal::insetRect(const Rectangle& rect, const float dx,
                                       const float dy) {
    return Rectangle{rect.x + dx, rect.y + dy, rect.width - 2 * dx,
                     rect.height - 2 * dy};
}

bool GuiWindowInternal::pointInsideRect(const Rectangle& rect,
                                        const Vector2 point) {
    return CheckCollisionPointRec(point, rect);
}

std::string GuiWindowInternal::trimWhitespace(std::string text) {
    const std::string whitespace = " \t\r\n";
    const std::size_t start = text.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        return "";
    }
    const std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(start, end - start + 1);
}

Color GuiWindowInternal::colorFromKey(const std::string& key) {
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

Color GuiWindowInternal::playerPieceColor(const int playerIndex) {
    const std::array<Color, 4> playerColors = {
        Color{211, 47, 47, 255},
        Color{30, 96, 190, 255},
        Color{33, 135, 85, 255},
        Color{231, 123, 21, 255},
    };

    if (playerIndex < 0) {
        return playerColors[0];
    }

    return playerColors[static_cast<std::size_t>(playerIndex) %
                        playerColors.size()];
}

Color GuiWindowInternal::contrastingTextColor(const Color bg) {
    const int luminance = 299 * bg.r + 587 * bg.g + 114 * bg.b;
    return luminance > 145000 ? kInk : kWhitePanel;
}

std::string GuiWindowInternal::playerStateLabel(const PlayerSnapshot& player) {
    if (player.bankrupt) return "BANKRUPT";
    if (player.jailed) return "JAILED";
    if (player.active) return "ACTIVE";
    return "WAITING";
}

std::string GuiWindowInternal::ownerLabel(const TileSnapshot& tile) {
    if (tile.owner.empty()) {
        return "BANK";
    }
    if (tile.owner.size() <= 10) {
        return tile.owner;
    }
    return tile.owner.substr(0, 10);
}

std::string GuiWindowInternal::tileFooter(const TileSnapshot& tile) {
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

std::vector<std::string> GuiWindowInternal::splitLines(
    const std::string& text) {
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

std::string GuiWindowInternal::formatTransactionLogEntry(
    const LogSnapshot& entry) {
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

std::string GuiWindowInternal::buildTransactionLogText(
    const GameSnapshot& currentSnapshot) {
    if (currentSnapshot.logs.empty()) {
        return "Belum ada transaksi/log yang tercatat.";
    }

    std::ostringstream oss;
    for (const LogSnapshot& entry : currentSnapshot.logs) {
        oss << formatTransactionLogEntry(entry) << '\n';
    }
    return oss.str();
}

std::string GuiWindowInternal::truncateText(const Font& font,
                                            const std::string& text,
                                            const float fontSize,
                                            const float spacing,
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

std::vector<std::string> GuiWindowInternal::wrapText(const Font& font,
                                                     const std::string& text,
                                                     const float fontSize,
                                                     const float spacing,
                                                     const float maxWidth,
                                                     const int maxLines) {
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

void GuiWindowInternal::drawWrappedText(const Font& font,
                                        const std::string& text,
                                        const Rectangle& rect,
                                        const float fontSize,
                                        const float spacing, const Color color,
                                        const int maxLines) {
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

void GuiWindowInternal::drawTextCentered(const Font& font,
                                         const std::string& text,
                                         const Rectangle& rect,
                                         const float fontSize,
                                         const float spacing,
                                         const Color color) {
    const Vector2 size = MeasureTextEx(font, text.c_str(), fontSize, spacing);
    const Vector2 position = {rect.x + (rect.width - size.x) / 2.0F,
                              rect.y + (rect.height - size.y) / 2.0F};
    DrawTextEx(font, text.c_str(), position, fontSize, spacing, color);
}

void GuiWindowInternal::drawPanelFrame(const Rectangle& rect) {
    DrawRectangleRec(rect, kPanel);
    DrawRectangleLinesEx(rect, 1.0F, kPanelBorder);
    DrawRectangleRec(Rectangle{rect.x, rect.y, rect.width, 5.0F}, kAccent);
}

void GuiWindowInternal::drawButton(const Font& font, const Rectangle& rect,
                                   const std::string& label,
                                   const bool enabled, const bool active) {
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

bool GuiWindowInternal::isButtonPressed(const Rectangle& rect,
                                        const bool enabled) {
    return enabled && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) &&
           pointInsideRect(rect, GetMousePosition());
}

Rectangle GuiWindowInternal::computeScrollbarThumb(const Rectangle& trackRect,
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

Font GuiWindowInternal::loadSystemFontFromMemory(const std::string& path,
                                                 const int baseFontSize,
                                                 int* codepoints,
                                                 const int codepointCount) {
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

SideDistribution GuiWindowInternal::computeSideDistribution(
    const int totalTiles) {
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

TileSide GuiWindowInternal::resolveTileSide(const int position,
                                            const int totalTiles) {
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

Rectangle GuiWindowInternal::calculateTileRect(const Rectangle& rect,
                                               const int position,
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
            innerRight - ((indexFromRight + 1) * innerWidth) /
                             distribution.bottomInner;
        const float right =
            innerRight - (indexFromRight * innerWidth) /
                             distribution.bottomInner;
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
                          ((indexFromBottom + 1) * innerHeight) /
                              distribution.leftInner;
        const float bottom = innerBottom -
                             (indexFromBottom * innerHeight) /
                                 distribution.leftInner;
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
            innerLeft + ((indexFromLeft + 1) * innerWidth) /
                            distribution.topInner;
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

Rectangle GuiWindowInternal::calculateRosterCardRect(
    const Rectangle& rect, const GameSnapshot& currentSnapshot, const int index) {
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

void GuiWindowInternal::drawPip(const Vector2 center, const float radius) {
    DrawCircleV(center, radius, Color{30, 28, 26, 255});
}

void GuiWindowInternal::drawDieFace(const Rectangle& rect, const int value,
                                    const Font& font) {
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

void GuiWindowInternal::drawRectangleRoundedLinesCompat(
    Rectangle rec, float roundness, int segments, float lineThick, Color color) {
#if defined(RAYLIB_VERSION_MAJOR) && defined(RAYLIB_VERSION_MINOR) && \
    ((RAYLIB_VERSION_MAJOR > 5) ||                                  \
     (RAYLIB_VERSION_MAJOR == 5 && RAYLIB_VERSION_MINOR >= 5))
    DrawRectangleRoundedLinesEx(rec, roundness, segments, lineThick, color);
#elif defined(RAYLIB_VERSION_MAJOR) && RAYLIB_VERSION_MAJOR >= 5
    DrawRectangleRoundedLines(rec, roundness, segments, lineThick, color);
#else
    (void)lineThick;
    DrawRectangleRoundedLines(rec, roundness, segments, color);
#endif
}
