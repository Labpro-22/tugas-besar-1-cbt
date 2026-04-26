#include "gui/BoardPopup.hpp"
#include "gui/GuiWindowInternal.hpp"
#include <algorithm>
#include <sstream>

BoardPopup::BoardPopup() : showPopup(false) {}
BoardPopup::~BoardPopup() {}

void BoardPopup::open() { showPopup = true; }
void BoardPopup::close() { showPopup = false; }
bool BoardPopup::isOpen() const { return showPopup; }

static void DrawTextAutofitLB(Font font, const std::string &text,
                              Rectangle bounds, float maxFontSize, Color color,
                              int align = 0, bool fakeBold = false) {
  float fs = maxFontSize;
  Vector2 ts = MeasureTextEx(font, text.c_str(), fs, 1);
  while (ts.x > bounds.width && fs > 8.0f) {
    fs -= 1.0f;
    ts = MeasureTextEx(font, text.c_str(), fs, 1);
  }
  Vector2 pos;
  if (align == -1)
    pos = {bounds.x, bounds.y + bounds.height / 2.0f - ts.y / 2.0f};
  else if (align == 1)
    pos = {bounds.x + bounds.width - ts.x,
           bounds.y + bounds.height / 2.0f - ts.y / 2.0f};
  else
    pos = {bounds.x + bounds.width / 2.0f - ts.x / 2.0f,
           bounds.y + bounds.height / 2.0f - ts.y / 2.0f};

  DrawTextEx(font, text.c_str(), pos, fs, 1, color);
  if (fakeBold)
    DrawTextEx(font, text.c_str(), Vector2{pos.x + 1, pos.y}, fs, 1, color);
}

bool BoardPopup::drawCloseButton(Rectangle rect, const char *text, Color bg,
                                 Color fg, Color border, Font font) {
  Vector2 mousePos = GetMousePosition();
  bool hover = CheckCollisionPointRec(mousePos, rect);
  DrawRectangleRec(rect, hover ? fg : bg);
  DrawRectangleLinesEx(rect, 1, border);
  DrawTextAutofitLB(
      font, text,
      Rectangle{rect.x + 4, rect.y + 4, rect.width - 8, rect.height - 8}, 24,
      hover ? bg : fg, 0, true);
  return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void BoardPopup::drawOverlay(const GameSnapshot &currentSnapshot, Font font) {
  if (!showPopup)
    return;

  // Darkened backdrop blur-like
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                Fade(Color{30, 27, 24, 255}, 0.80f));

  float pWidth = GetScreenWidth() * 0.85f;
  float pHeight = GetScreenHeight() * 0.85f;
  // limit maximum sizes based on aspect ratios ideally, but we can stick to 90%
  // of screen center
  if (pWidth > 1200.0f)
    pWidth = 1200.0f;
  if (pHeight > 1000.0f)
    pHeight = 1000.0f;

  Rectangle modal = {(GetScreenWidth() - pWidth) * 0.5f,
                     (GetScreenHeight() - pHeight) * 0.5f, pWidth, pHeight};

  // Popup Surface
  Color surfaceColor = {255, 248, 245, 255}; // SURFACE
  Color primaryColor = {65, 0, 2, 255};

  DrawRectangleRec(modal, surfaceColor);
  DrawRectangleLinesEx(modal, 4.0F, primaryColor);

  // Header
  Rectangle header = {modal.x, modal.y, pWidth, 64.0f};
  DrawRectangleRec(header, surfaceColor);
  DrawRectangleLinesEx(
      Rectangle{header.x, header.y + header.height - 4, header.width, 4}, 4.0f,
      primaryColor);
  DrawTextEx(font, "[-] Peta Papan Nimonspoli",
             Vector2{header.x + 20, header.y + 20}, 24, 1, primaryColor);

  Rectangle closeBtn = {modal.x + modal.width - 56, header.y + 12, 40, 40};
  if (drawCloseButton(closeBtn, "X", surfaceColor, primaryColor, primaryColor,
                      font)) {
    showPopup = false;
    return;
  }

  // Footer for Turn
  Rectangle footer = {modal.x, modal.y + modal.height - 64.0f, pWidth, 64.0f};
  DrawRectangleRec(footer, Color{233, 225, 220, 255}); // SURFACE_VARIANT
  DrawRectangleLinesEx(Rectangle{footer.x, footer.y, footer.width, 4}, 4.0f,
                       primaryColor);

  std::string turnStr = "Giliran: ";
  if (currentSnapshot.gameStarted) {
    turnStr += std::to_string(currentSnapshot.currentTurn) + " / " +
               std::to_string(currentSnapshot.maxTurn);
  } else {
    turnStr += "-";
  }
  DrawTextAutofitLB(
      font, turnStr,
      Rectangle{footer.x + 20, footer.y + 12, footer.width - 40, 40}, 24,
      primaryColor, -1, true);

  // Remaining space for Board
  Rectangle boardRect = {modal.x + 20.0f, modal.y + 64.0f + 20.0f,
                         modal.width - 40.0f, modal.height - 128.0f - 40.0f};

  // Ensure it's roughly square like standard board
  float boardSize = std::min(boardRect.width, boardRect.height);
  boardRect.x += (boardRect.width - boardSize) / 2.0f;
  boardRect.y += (boardRect.height - boardSize) / 2.0f;
  boardRect.width = boardSize;
  boardRect.height = boardSize;

  // Render Inner Board logic from GuiWindowBoard
  DrawRectangleRec(boardRect, GuiWindowInternal::kAccent);
  Rectangle boardSurface =
      GuiWindowInternal::insetRect(boardRect, 10.0F, 10.0F);
  DrawRectangleRec(boardSurface, GuiWindowInternal::kPanel);
  DrawRectangleLinesEx(boardSurface, 2.0F, GuiWindowInternal::kAccentDark);

  if (currentSnapshot.tiles.empty()) {
    GuiWindowInternal::drawTextCentered(
        font, "Papan kosong atau data snap belum termuat.", boardSurface, 28.0F,
        1.0F, GuiWindowInternal::kAccentDark);
    return;
  }

  const int totalTiles = static_cast<int>(currentSnapshot.tiles.size());
  const int borderThickness = GuiWindowInternal::clampInt(
      static_cast<int>(std::min(boardSurface.width, boardSurface.height) /
                       7.0F),
      56, 96);
  const Rectangle centerField{boardSurface.x + borderThickness,
                              boardSurface.y + borderThickness,
                              boardSurface.width - borderThickness * 2.0F,
                              boardSurface.height - borderThickness * 2.0F};

  DrawRectangleRec(centerField, GuiWindowInternal::kBoardField);
  DrawRectangleLinesEx(centerField, 1.0F, Color{184, 173, 160, 255});

  for (const TileSnapshot &tile : currentSnapshot.tiles) {
    const Rectangle tileRect = GuiWindowInternal::calculateTileRect(
        boardSurface, tile.position, totalTiles);
    const TileSide side =
        GuiWindowInternal::resolveTileSide(tile.position, totalTiles);

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
    DrawRectangleRec(stripRect, GuiWindowInternal::colorFromKey(tile.colorKey));

    const bool isCorner = side == TileSide::BottomRightCorner ||
                          side == TileSide::BottomLeftCorner ||
                          side == TileSide::TopLeftCorner ||
                          side == TileSide::TopRightCorner;
    const float codeFontSize = isCorner ? 15.0F : 14.0F;
    const float nameFontSize =
        isCorner ? 13.0F : (tileRect.width < 78.0F ? 11.0F : 12.0F);

    GuiWindowInternal::drawTextCentered(
        font, tile.code,
        Rectangle{tileRect.x + 4.0F, tileRect.y + 4.0F, tileRect.width - 8.0F,
                  18.0F},
        codeFontSize, 1.0F, GuiWindowInternal::kAccentDark);

    const Rectangle nameRect{tileRect.x + 5.0F, tileRect.y + 24.0F,
                             tileRect.width - 10.0F, tileRect.height - 44.0F};
    GuiWindowInternal::drawWrappedText(font, tile.name, nameRect, nameFontSize,
                                       1.0F, GuiWindowInternal::kInk, 2);

    const std::string footer = GuiWindowInternal::tileFooter(tile);
    if (!footer.empty() && tileRect.width >= 52.0F) {
      const float footerFontSize = tileRect.width < 78.0F ? 10.0F : 11.0F;
      const std::string footerText = GuiWindowInternal::truncateText(
          font, footer, footerFontSize, 1.0F, tileRect.width - 10.0F);
      GuiWindowInternal::drawTextCentered(
          font, footerText,
          Rectangle{tileRect.x + 5.0F, tileRect.y + tileRect.height - 20.0F,
                    tileRect.width - 10.0F, 16.0F},
          footerFontSize, 1.0F, GuiWindowInternal::kMuted);
    }
  }

  for (std::size_t i = 0; i < currentSnapshot.players.size(); ++i) {
    const PlayerSnapshot &player = currentSnapshot.players[i];
    const Rectangle tileRect = GuiWindowInternal::calculateTileRect(
        boardSurface, player.position, totalTiles);
    const float markerSize = 18.0F;
    const float markerGap = 5.0F;
    const int row = static_cast<int>(i) / 2;
    const int column = static_cast<int>(i) % 2;
    const float markerX = tileRect.x + 7.0F + column * (markerSize + markerGap);
    const float markerY = tileRect.y + 7.0F + row * (markerSize + markerGap);
    DrawCircle(markerX + markerSize / 2.0F, markerY + markerSize / 2.0F,
               markerSize / 2.0F,
               GuiWindowInternal::playerPieceColor(static_cast<int>(i)));
    GuiWindowInternal::drawTextCentered(
        font, std::to_string(static_cast<int>(i + 1)),
        Rectangle{markerX, markerY, markerSize, markerSize}, 13.0F, 1.0F,
        GuiWindowInternal::kWhitePanel);
  }

  DrawTextEx(font, "NIMONSPOLI",
             Vector2{centerField.x + centerField.width / 2.0F - 60.0F,
                     centerField.y + centerField.height / 2.0F - 60.0F},
             36.0F, 2.0F, GuiWindowInternal::kAccentDark);
}
