#include "gui/GuiWindowBoard.hpp"

#include <algorithm>
#include <sstream>

void GuiWindow::drawBoard(const Layout& layout,
                          const GameSnapshot& currentSnapshot) const {
    const Font& font = georgiaFont;
    DrawRectangleRec(layout.boardRect, GuiWindowInternal::kAccent);
    const Rectangle boardSurface = GuiWindowInternal::insetRect(layout.boardRect, 10.0F, 10.0F);
    DrawRectangleRec(boardSurface, GuiWindowInternal::kPanel);
    DrawRectangleLinesEx(boardSurface, 2.0F, GuiWindowInternal::kAccentDark);

    if (!hasSnapshot || currentSnapshot.tiles.empty()) {
        GuiWindowInternal::drawTextCentered(font, "Menunggu sesi game...", boardSurface, 28.0F, 1.0F,
                         GuiWindowInternal::kAccentDark);
        return;
    }

    const int totalTiles = static_cast<int>(currentSnapshot.tiles.size());
    const int borderThickness =
        GuiWindowInternal::clampInt(static_cast<int>(std::min(boardSurface.width, boardSurface.height) /
                                  7.0F),
                 56, 96);
    const Rectangle centerField{boardSurface.x + borderThickness,
                                boardSurface.y + borderThickness,
                                boardSurface.width - borderThickness * 2.0F,
                                boardSurface.height - borderThickness * 2.0F};

    DrawRectangleRec(centerField, GuiWindowInternal::kBoardField);
    DrawRectangleLinesEx(centerField, 1.0F, Color{184, 173, 160, 255});

    for (const TileSnapshot& tile : currentSnapshot.tiles) {
        const Rectangle tileRect =
            GuiWindowInternal::calculateTileRect(boardSurface, tile.position, totalTiles);
        const TileSide side = GuiWindowInternal::resolveTileSide(tile.position, totalTiles);

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

        GuiWindowInternal::drawTextCentered(font, tile.code,
                         Rectangle{tileRect.x + 4.0F, tileRect.y + 4.0F,
                                   tileRect.width - 8.0F, 18.0F},
                         codeFontSize, 1.0F, GuiWindowInternal::kAccentDark);

        const Rectangle nameRect{tileRect.x + 5.0F, tileRect.y + 24.0F,
                                 tileRect.width - 10.0F, tileRect.height - 44.0F};
        GuiWindowInternal::drawWrappedText(font, tile.name, nameRect, nameFontSize, 1.0F, GuiWindowInternal::kInk, 2);

        const std::string footer = GuiWindowInternal::tileFooter(tile);
        if (!footer.empty() && tileRect.width >= 52.0F) {
            const float footerFontSize = tileRect.width < 78.0F ? 10.0F : 11.0F;
            const std::string footerText =
                GuiWindowInternal::truncateText(font, footer, footerFontSize, 1.0F,
                             tileRect.width - 10.0F);
            GuiWindowInternal::drawTextCentered(font, footerText,
                             Rectangle{tileRect.x + 5.0F,
                                        tileRect.y + tileRect.height - 20.0F,
                                        tileRect.width - 10.0F, 16.0F},
                             footerFontSize, 1.0F, GuiWindowInternal::kMuted);
        }
    }

    for (std::size_t i = 0; i < currentSnapshot.players.size(); ++i) {
        const PlayerSnapshot& player = currentSnapshot.players[i];
        const Rectangle tileRect =
            GuiWindowInternal::calculateTileRect(boardSurface, player.position, totalTiles);
        const float markerSize = 18.0F;
        const float markerGap = 5.0F;
        const int row = static_cast<int>(i) / 2;
        const int column = static_cast<int>(i) % 2;
        const float markerX = tileRect.x + 7.0F + column * (markerSize + markerGap);
        const float markerY = tileRect.y + 7.0F + row * (markerSize + markerGap);
        DrawCircle(markerX + markerSize / 2.0F, markerY + markerSize / 2.0F,
                   markerSize / 2.0F,
                   GuiWindowInternal::playerPieceColor(static_cast<int>(i)));
        GuiWindowInternal::drawTextCentered(font, std::to_string(static_cast<int>(i + 1)),
                         Rectangle{markerX, markerY, markerSize, markerSize}, 13.0F,
                         1.0F, GuiWindowInternal::kWhitePanel);
    }

    DrawTextEx(font, "NIMONSPOLI",
               Vector2{centerField.x + centerField.width / 2.0F - 185.0F,
                       centerField.y + 70.0F},
               52.0F, 2.0F, GuiWindowInternal::kAccentDark);

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
    GuiWindowInternal::drawWrappedText(font, info.str(),
                    Rectangle{centerField.x + centerField.width / 2.0F - 150.0F,
                              centerField.y + 150.0F, 300.0F, 120.0F},
                    22.0F, 1.0F, GuiWindowInternal::kMuted, 4);

    if (currentSnapshot.gameStarted) {
        const float diceSize = GuiWindowInternal::clampFloat(centerField.width / 8.0F, 64.0F, 88.0F);
        const float diceGap = GuiWindowInternal::clampFloat(diceSize / 3.0F, 18.0F, 28.0F);
        const float diceTop =
            std::max(centerField.y + 210.0F,
                     layout.rollButtonRect.y - diceSize - 30.0F);
        const Rectangle dieOneRect{
            centerField.x + (centerField.width - (diceSize * 2.0F + diceGap)) / 2.0F,
            diceTop, diceSize, diceSize};
        const Rectangle dieTwoRect{
            dieOneRect.x + dieOneRect.width + diceGap, diceTop, diceSize, diceSize};

        GuiWindowInternal::drawDieFace(dieOneRect, currentSnapshot.hasDiceResult ? currentSnapshot.die1 : 0, font);
        GuiWindowInternal::drawDieFace(dieTwoRect, currentSnapshot.hasDiceResult ? currentSnapshot.die2 : 0, font);

        GuiWindowInternal::drawButton(font, layout.rollButtonRect, "KOCOK DADU", true, true);
    }
}

