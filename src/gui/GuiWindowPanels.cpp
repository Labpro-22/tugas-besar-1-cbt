#include "GuiWindowInternal.hpp"

using namespace gui_internal;

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
        const Rectangle turnRect{
            layout.headerRect.x + layout.headerRect.width - 300.0F,
            layout.headerRect.y + 16.0F, 190.0F, 34.0F};
        DrawRectangleRec(turnRect, Color{248, 241, 232, 255});
        DrawRectangleLinesEx(turnRect, 1.0F, Color{198, 184, 170, 255});
        drawTextCentered(font,
                         truncateText(font, turnText, 17.0F, 1.0F,
                                      turnRect.width - 12.0F),
                         turnRect, 17.0F, 1.0F, kAccentDark);
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

