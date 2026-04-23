#ifndef GUI_WINDOW_INTERNAL_HPP
#define GUI_WINDOW_INTERNAL_HPP

#include "GuiWindow.hpp"

#include <array>
#include <string>
#include <vector>

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

class GuiWindowInternal {
public:
    static const int kWindowWidth;
    static const int kWindowHeight;
    static const Color kPaper;
    static const Color kPanel;
    static const Color kBoardField;
    static const Color kAccent;
    static const Color kAccentDark;
    static const Color kInk;
    static const Color kMuted;
    static const Color kPanelBorder;
    static const Color kWhitePanel;
    static const Color kInspected;
    static const Color kDisabled;
    static const Color kGold;
    static const Color kCream;
    static const std::array<StartedQuickActionSpec, 14> kStartedQuickActions;

    static int clampInt(int value, int minimum, int maximum);
    static float clampFloat(float value, float minimum, float maximum);
    static Rectangle insetRect(const Rectangle& rect, float dx, float dy);
    static bool pointInsideRect(const Rectangle& rect, Vector2 point);
    static std::string trimWhitespace(std::string text);
    static Color colorFromKey(const std::string& key);
    static Color playerPieceColor(int playerIndex);
    static Color contrastingTextColor(Color bg);
    static std::string playerStateLabel(const PlayerSnapshot& player);
    static std::string ownerLabel(const TileSnapshot& tile);
    static std::string tileFooter(const TileSnapshot& tile);
    static std::vector<std::string> splitLines(const std::string& text);
    static std::string formatTransactionLogEntry(const LogSnapshot& entry);
    static std::string buildTransactionLogText(const GameSnapshot& currentSnapshot);
    static std::string truncateText(const Font& font, const std::string& text,
                                    float fontSize, float spacing, float maxWidth);
    static std::vector<std::string> wrapText(const Font& font,
                                             const std::string& text,
                                             float fontSize, float spacing,
                                             float maxWidth, int maxLines = 1000);
    static void drawWrappedText(const Font& font, const std::string& text,
                                const Rectangle& rect, float fontSize,
                                float spacing, Color color,
                                int maxLines = 1000);
    static void drawTextCentered(const Font& font, const std::string& text,
                                 const Rectangle& rect, float fontSize,
                                 float spacing, Color color);
    static void drawPanelFrame(const Rectangle& rect);
    static void drawButton(const Font& font, const Rectangle& rect,
                           const std::string& label, bool enabled,
                           bool active = false);
    static bool isButtonPressed(const Rectangle& rect, bool enabled);
    static Rectangle computeScrollbarThumb(const Rectangle& trackRect,
                                           int visibleLines, int totalLines,
                                           int startLine);
    static Font loadSystemFontFromMemory(const std::string& path,
                                         int baseFontSize, int* codepoints,
                                         int codepointCount);
    static SideDistribution computeSideDistribution(int totalTiles);
    static TileSide resolveTileSide(int position, int totalTiles);
    static Rectangle calculateTileRect(const Rectangle& rect, int position,
                                       int totalTiles);
    static Rectangle calculateRosterCardRect(const Rectangle& rect,
                                             const GameSnapshot& currentSnapshot,
                                             int index);
    static void drawDieFace(const Rectangle& rect, int value, const Font& font);
    static void drawRectangleRoundedLinesCompat(Rectangle rec, float roundness,
                                                int segments, float lineThick,
                                                Color color);

private:
    static void drawPip(Vector2 center, float radius);
};

#endif
