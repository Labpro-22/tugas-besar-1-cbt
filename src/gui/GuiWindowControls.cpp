#include "gui/GuiWindowInternal.hpp"

using namespace gui_internal;

namespace {

void drawRectangleRoundedLinesCompat(Rectangle rec, float roundness, int segments,
                                     float lineThick, Color color) {
#if defined(RAYLIB_VERSION_MAJOR) && \
    defined(RAYLIB_VERSION_MINOR) && \
    RAYLIB_VERSION_MAJOR == 5 && RAYLIB_VERSION_MINOR == 5
    DrawRectangleRoundedLinesEx(rec, roundness, segments, lineThick, color);
#else
    DrawRectangleRoundedLines(rec, roundness, segments, lineThick, color);
#endif
}

}  // namespace

Rectangle GuiWindow::gameOverPopupCardRect() const {
    const float cardWidth = 660.0F;
    const float cardHeight = 498.0F;
    return Rectangle{(GetScreenWidth() - cardWidth) / 2.0F,
                     (GetScreenHeight() - cardHeight) / 2.0F,
                     cardWidth, cardHeight};
}

Rectangle GuiWindow::gameOverPopupExitButtonRect() const {
    const Rectangle cardRect = gameOverPopupCardRect();
    return Rectangle{cardRect.x + 28.0F, cardRect.y + cardRect.height - 114.0F,
                     292.0F, 46.0F};
}

Rectangle GuiWindow::gameOverPopupNewGameButtonRect() const {
    const Rectangle cardRect = gameOverPopupCardRect();
    return Rectangle{cardRect.x + cardRect.width - 320.0F,
                     cardRect.y + cardRect.height - 114.0F, 292.0F, 46.0F};
}

Rectangle GuiWindow::gameOverPopupCloseButtonRect() const {
    const Rectangle cardRect = gameOverPopupCardRect();
    return Rectangle{cardRect.x + 28.0F, cardRect.y + cardRect.height - 58.0F,
                     cardRect.width - 56.0F, 42.0F};
}

void GuiWindow::drawActionBar(const Layout& layout,
                              const GameSnapshot& currentSnapshot) const {
    const Font& font = georgiaFont;
    for (std::size_t i = 0; i < layout.quickButtonRects.size(); ++i) {
        drawButton(font, layout.quickButtonRects[i], quickButtonLabels[i],
                   quickButtonEnabled[i], false);
    }

    if (currentSnapshot.gameStarted && !currentSnapshot.gameOver) {
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

void GuiWindow::drawLogPanel(const Layout& layout) {
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

    const float scrollbarWidth = 12.0F;
    const Rectangle textRect{
        layout.logRect.x,
        layout.logRect.y,
        std::max(20.0F, layout.logRect.width - scrollbarWidth - 10.0F),
        layout.logRect.height};
    const auto lines =
        wrapText(font, outputCopy, 15.0F, 1.0F, textRect.width, 100000);
    const float lineHeight = 19.0F;
    const int visibleLines = std::max(1, static_cast<int>(layout.logRect.height / lineHeight));
    const int maxStartLine =
        std::max(0, static_cast<int>(lines.size()) - visibleLines);
    if (logAutoScroll) {
        logScrollLine = maxStartLine;
    } else {
        logScrollLine = clampInt(logScrollLine, 0, maxStartLine);
    }
    const int startLine = logScrollLine;

    BeginScissorMode(static_cast<int>(textRect.x),
                     static_cast<int>(textRect.y),
                     static_cast<int>(textRect.width),
                     static_cast<int>(textRect.height));
    float y = textRect.y;
    for (int i = startLine; i < static_cast<int>(lines.size()); ++i) {
        if (y > textRect.y + textRect.height - lineHeight) {
            break;
        }
        DrawTextEx(font, lines[static_cast<std::size_t>(i)].c_str(),
                   Vector2{textRect.x, y}, 15.0F, 1.0F, kInk);
        y += lineHeight;
    }
    EndScissorMode();

    const Rectangle scrollbarTrack{
        layout.logRect.x + layout.logRect.width - scrollbarWidth,
        layout.logRect.y,
        scrollbarWidth,
        layout.logRect.height};
    DrawRectangleRec(scrollbarTrack, Color{236, 231, 221, 255});
    DrawRectangleLinesEx(scrollbarTrack, 1.0F, kPanelBorder);

    if (maxStartLine > 0) {
        const float thumbHeight = std::max(
            28.0F, scrollbarTrack.height *
                       (static_cast<float>(visibleLines) /
                        static_cast<float>(std::max(visibleLines, static_cast<int>(lines.size())))));
        const float thumbTravel = scrollbarTrack.height - thumbHeight;
        const float ratio =
            static_cast<float>(startLine) / static_cast<float>(maxStartLine);
        const Rectangle thumb{
            scrollbarTrack.x + 1.0F,
            scrollbarTrack.y + ratio * thumbTravel,
            scrollbarTrack.width - 2.0F,
            thumbHeight};
        DrawRectangleRec(thumb, Color{176, 162, 149, 255});
        DrawRectangleLinesEx(thumb, 1.0F, kAccentDark);
    }
}

void GuiWindow::drawModal(const GameSnapshot& currentSnapshot) const {
    ModalState current;
    {
        std::lock_guard<std::mutex> lock(modalMutex);
        current = modal;
    }

    if (!current.active) {
        return;
    }

    const Font& font = georgiaFont;
    const bool isErrorModal = current.localType == LocalDialogType::ErrorMessage;
    std::string activePlayerName;
    if (currentSnapshot.gameStarted && currentSnapshot.activePlayerIndex >= 0 &&
        currentSnapshot.activePlayerIndex <
            static_cast<int>(currentSnapshot.players.size())) {
        activePlayerName =
            currentSnapshot
                .players[static_cast<std::size_t>(currentSnapshot.activePlayerIndex)]
                .name;
    }

    const Rectangle dialogRect = modalDialogRect();
    const Rectangle titleBarRect{dialogRect.x, dialogRect.y, dialogRect.width, 42.0F};
    const Rectangle promptRect{dialogRect.x + 18.0F, dialogRect.y + 56.0F,
                               dialogRect.width - 36.0F,
                               isErrorModal ? 132.0F : 82.0F};
    const Rectangle inputRect{dialogRect.x + 18.0F, dialogRect.y + 146.0F,
                              dialogRect.width - 36.0F, 34.0F};
    const Rectangle okRect{dialogRect.x + dialogRect.width - 210.0F,
                           dialogRect.y + dialogRect.height - 58.0F, 90.0F, 34.0F};
    const Rectangle errorOkRect{dialogRect.x + dialogRect.width - 120.0F,
                                dialogRect.y + dialogRect.height - 58.0F, 90.0F,
                                34.0F};
    const Rectangle cancelRect{dialogRect.x + dialogRect.width - 110.0F,
                               dialogRect.y + dialogRect.height - 58.0F, 90.0F, 34.0F};

    DrawRectangleRec(dialogRect, kWhitePanel);
    DrawRectangleLinesEx(dialogRect, 2.0F, kAccentDark);
    DrawRectangleRec(titleBarRect, Color{245, 232, 225, 255});
    DrawRectangleLinesEx(titleBarRect, 1.0F, kAccentDark);
    DrawTextEx(font, current.title.c_str(),
               Vector2{dialogRect.x + 18.0F, dialogRect.y + 10.0F}, 24.0F, 1.0F,
               kAccentDark);
    if (!activePlayerName.empty()) {
        const float labelSize = 24.0F;
        const std::string label = "Giliran: ";
        const Vector2 labelPos{dialogRect.x + dialogRect.width - 285.0F,
                               dialogRect.y + 6.0F};
        DrawTextEx(font, label.c_str(), labelPos, labelSize, 1.0F, kAccentDark);
        DrawTextEx(font, label.c_str(), Vector2{labelPos.x + 0.7F, labelPos.y},
                   labelSize, 1.0F, kAccentDark);

        const float labelWidth = MeasureTextEx(font, label.c_str(), labelSize, 1.0F).x;
        const std::string nameText =
            truncateText(font, activePlayerName, labelSize, 1.0F, 126.0F);
        const Vector2 namePos{labelPos.x + labelWidth, labelPos.y};
        DrawTextEx(font, nameText.c_str(), namePos, labelSize, 1.0F, kAccentDark);
        DrawTextEx(font, nameText.c_str(), Vector2{namePos.x + 0.7F, namePos.y},
                   labelSize, 1.0F, kAccentDark);
    }
    drawWrappedText(font, current.prompt, promptRect, 18.0F, 1.0F,
                    isErrorModal ? Color{160, 30, 30, 255} : kInk,
                    isErrorModal ? 7 : 5);

    if (!isErrorModal && (!current.yesNo || !current.backendOwned)) {
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

    if (isErrorModal) {
        drawButton(font, errorOkRect, "OK", true, true);
    } else {
        drawButton(font, okRect, current.yesNo && current.backendOwned ? "YA" : "OK",
                   true, current.yesNo && current.backendOwned);
        drawButton(font, cancelRect,
                   current.yesNo && current.backendOwned ? "TIDAK" : "BATAL", true,
                   false);
    }
}

void GuiWindow::drawTurnChangePopup(const GameSnapshot& currentSnapshot) const {
    if (!currentSnapshot.gameStarted) {
        return;
    }

    int popupPlayer = -1;
    std::string popupPlayerName;
    float popupTimer = 0.0F;
    {
        std::lock_guard<std::mutex> lock(snapshotMutex);
        popupPlayer = turnPopupPlayerIndex;
        popupPlayerName = turnPopupPlayerName;
        popupTimer = turnPopupTimer;
    }

    if (popupTimer <= 0.0F || currentSnapshot.players.empty()) {
        return;
    }

    int resolvedPlayerIndex = -1;
    if (!popupPlayerName.empty()) {
        for (std::size_t i = 0; i < currentSnapshot.players.size(); ++i) {
            if (currentSnapshot.players[i].name == popupPlayerName) {
                resolvedPlayerIndex = static_cast<int>(i);
                break;
            }
        }
    }
    if (resolvedPlayerIndex < 0 && popupPlayer >= 0 &&
        popupPlayer < static_cast<int>(currentSnapshot.players.size())) {
        resolvedPlayerIndex = popupPlayer;
    }
    if (resolvedPlayerIndex < 0) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(modalMutex);
        if (modal.active) {
            return;
        }
    }

    const Font& font = georgiaFont;
    const float fadeWindow = 0.35F;
    const float alphaScale = popupTimer < fadeWindow ? popupTimer / fadeWindow : 1.0F;

    const float cardWidth = 610.0F;
    const float cardHeight = 340.0F;
    const Rectangle cardRect{(GetScreenWidth() - cardWidth) / 2.0F,
                             (GetScreenHeight() - cardHeight) / 2.0F,
                             cardWidth, cardHeight};

    const Color pieceColor = playerPieceColor(resolvedPlayerIndex);
    const unsigned char panelAlpha =
        static_cast<unsigned char>(clampFloat(238.0F * alphaScale, 0.0F, 255.0F));
    const unsigned char textAlpha =
        static_cast<unsigned char>(clampFloat(255.0F * alphaScale, 0.0F, 255.0F));
    const unsigned char shadowAlpha =
        static_cast<unsigned char>(clampFloat(85.0F * alphaScale, 0.0F, 255.0F));

    const Color ink = Color{247, 236, 221, textAlpha};
    const Color gold = Color{220, 184, 78, textAlpha};
    const Color goldDim = Color{170, 134, 31, textAlpha};

    Color panelDark = Color{static_cast<unsigned char>(pieceColor.r * 0.55F),
                            static_cast<unsigned char>(pieceColor.g * 0.45F),
                            static_cast<unsigned char>(pieceColor.b * 0.45F),
                            panelAlpha};
    Color panelMid = Color{static_cast<unsigned char>(pieceColor.r * 0.75F),
                           static_cast<unsigned char>(pieceColor.g * 0.62F),
                           static_cast<unsigned char>(pieceColor.b * 0.60F),
                           panelAlpha};

    DrawRectangleRec(Rectangle{0.0F, 0.0F, static_cast<float>(GetScreenWidth()),
                               static_cast<float>(GetScreenHeight())},
                     Color{20, 16, 12,
                           static_cast<unsigned char>(clampFloat(55.0F * alphaScale, 0.0F,
                                                                 255.0F))});

    DrawRectangleRounded(
        Rectangle{cardRect.x + 8.0F, cardRect.y + 12.0F, cardRect.width, cardRect.height},
        0.02F, 6, Color{18, 12, 10, shadowAlpha});
    DrawRectangleRec(cardRect, panelDark);
    DrawRectangleRec(Rectangle{cardRect.x + 8.0F, cardRect.y + 8.0F,
                               cardRect.width - 16.0F, cardRect.height - 16.0F},
                     panelMid);
    DrawRectangleLinesEx(cardRect, 2.0F, goldDim);
    DrawRectangleLinesEx(Rectangle{cardRect.x + 4.0F, cardRect.y + 4.0F,
                                   cardRect.width - 8.0F, cardRect.height - 8.0F},
                         1.0F, gold);

    const float decoLen = 18.0F;
    DrawLineEx(Vector2{cardRect.x + 14.0F, cardRect.y + 14.0F},
               Vector2{cardRect.x + 14.0F + decoLen, cardRect.y + 14.0F}, 3.0F, gold);
    DrawLineEx(Vector2{cardRect.x + 14.0F, cardRect.y + 14.0F},
               Vector2{cardRect.x + 14.0F, cardRect.y + 14.0F + decoLen}, 3.0F, gold);
    DrawLineEx(Vector2{cardRect.x + cardRect.width - 14.0F,
                       cardRect.y + cardRect.height - 14.0F},
               Vector2{cardRect.x + cardRect.width - 14.0F - decoLen,
                       cardRect.y + cardRect.height - 14.0F},
               3.0F, gold);
    DrawLineEx(Vector2{cardRect.x + cardRect.width - 14.0F,
                       cardRect.y + cardRect.height - 14.0F},
               Vector2{cardRect.x + cardRect.width - 14.0F,
                       cardRect.y + cardRect.height - 14.0F - decoLen},
               3.0F, gold);

    drawTextCentered(font, "SESI TRANSMISI",
                     Rectangle{cardRect.x + 20.0F, cardRect.y + 66.0F,
                               cardRect.width - 40.0F, 24.0F},
                     30.0F * 0.62F, 3.0F, gold);
    drawTextCentered(font, "GILIRAN BERGANTI",
                     Rectangle{cardRect.x + 20.0F, cardRect.y + 96.0F,
                               cardRect.width - 40.0F, 54.0F},
                     56.0F, 1.0F, ink);
    drawTextCentered(font, "ESTATE HOLDER",
                     Rectangle{cardRect.x + 20.0F, cardRect.y + 148.0F,
                               cardRect.width - 40.0F, 24.0F},
                     18.0F, 1.2F, gold);

    const int badgeNumber = resolvedPlayerIndex + 1;
    drawTextCentered(font, std::to_string(badgeNumber),
                     Rectangle{cardRect.x + 20.0F, cardRect.y + 168.0F,
                               cardRect.width - 40.0F, 104.0F},
                     150.0F, 1.0F, gold);

    DrawLineEx(Vector2{cardRect.x + 16.0F, cardRect.y + 212.0F},
               Vector2{cardRect.x + 235.0F, cardRect.y + 212.0F}, 1.0F,
               Color{gold.r, gold.g, gold.b,
                     static_cast<unsigned char>(clampFloat(110.0F * alphaScale, 0.0F,
                                                           255.0F))});
    DrawLineEx(Vector2{cardRect.x + cardRect.width - 16.0F, cardRect.y + 212.0F},
               Vector2{cardRect.x + cardRect.width - 235.0F, cardRect.y + 212.0F},
               1.0F,
               Color{gold.r, gold.g, gold.b,
                     static_cast<unsigned char>(clampFloat(110.0F * alphaScale, 0.0F,
                                                           255.0F))});

    drawTextCentered(font, "\"Saatnya mengatur kembali aset Anda.\"",
                     Rectangle{cardRect.x + 24.0F, cardRect.y + 280.0F,
                               cardRect.width - 48.0F, 30.0F},
                     40.0F * 0.58F, 1.0F,
                     Color{236, 214, 186, static_cast<unsigned char>(textAlpha * 0.92F)});

    const Rectangle actionRect{cardRect.x + cardRect.width / 2.0F - 124.0F,
                               cardRect.y + cardRect.height - 22.0F, 248.0F, 20.0F};
    DrawRectangleRec(actionRect, Color{gold.r, gold.g, gold.b,
                                       static_cast<unsigned char>(clampFloat(
                                           215.0F * alphaScale, 0.0F, 255.0F))});
    DrawRectangleLinesEx(actionRect, 2.0F, Color{112, 80, 18, textAlpha});
    drawTextCentered(font, "LANJUTKAN STRATEGI", actionRect, 24.0F * 0.72F, 1.0F,
                     Color{56, 39, 10, textAlpha});
}

void GuiWindow::drawGameOverPopup(const GameSnapshot& currentSnapshot) const {
    if (!currentSnapshot.gameOver || !currentSnapshot.hasWinnerSummary) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(snapshotMutex);
        if (gameOverPopupDismissed) {
            return;
        }
    }

    if (currentSnapshot.winnerNames.empty()) {
        return;
    }

    const Font& font = georgiaFont;

    int winnerIndex = -1;
    if (!currentSnapshot.winnerNames.empty()) {
        for (std::size_t i = 0; i < currentSnapshot.players.size(); ++i) {
            if (currentSnapshot.players[i].name == currentSnapshot.winnerNames.front()) {
                winnerIndex = static_cast<int>(i);
                break;
            }
        }
    }

    const Color pieceColor = playerPieceColor(winnerIndex);
    const Color textColor = contrastingTextColor(pieceColor);
    const Rectangle overlay{0.0F, 0.0F, static_cast<float>(GetScreenWidth()),
                            static_cast<float>(GetScreenHeight())};
    DrawRectangleRec(overlay, Color{20, 18, 16, 120});

    const Rectangle cardRect = gameOverPopupCardRect();
    const Rectangle innerRect{cardRect.x + 18.0F, cardRect.y + 122.0F,
                              cardRect.width - 36.0F, 256.0F};
    const Rectangle statsLine{innerRect.x + 28.0F, innerRect.y + 186.0F,
                              innerRect.width - 56.0F, 1.0F};
    const Rectangle badgeRect{innerRect.x + innerRect.width / 2.0F - 44.0F,
                              innerRect.y + 30.0F, 88.0F, 88.0F};

    const Color cardBase = Color{232, 226, 214, 255};
    const Color surface = Color{235, 229, 218, 255};
    const Color border = Color{214, 206, 193, 255};
    const Color heading = Color{129, 107, 56, 255};
    const Color muted = Color{123, 114, 98, 255};
    const Color statLabel = Color{153, 145, 133, 255};
    const Color statValue = Color{115, 76, 66, 255};

    DrawRectangleRounded(cardRect, 0.02F, 6, cardBase);
    drawRectangleRoundedLinesCompat(cardRect, 0.02F, 6, 2.0F, border);

    drawTextCentered(font, "PERMAINAN SELESAI",
                     Rectangle{cardRect.x + 20.0F, cardRect.y + 26.0F,
                               cardRect.width - 40.0F, 28.0F},
                     28.0F, 1.0F, heading);
    drawTextCentered(font, "THE BOARD IS RESOLVED",
                     Rectangle{cardRect.x + 20.0F, cardRect.y + 56.0F,
                               cardRect.width - 40.0F, 20.0F},
                     19.0F, 1.0F, muted);
    DrawLineEx(Vector2{cardRect.x + 60.0F, cardRect.y + 76.0F},
               Vector2{cardRect.x + cardRect.width - 60.0F, cardRect.y + 76.0F},
               1.0F, border);

    if (!currentSnapshot.gameOverReason.empty()) {
        drawTextCentered(
            font,
            "\"" + currentSnapshot.gameOverReason + "\"",
            Rectangle{cardRect.x + 32.0F, cardRect.y + 84.0F, cardRect.width - 64.0F,
                      24.0F},
            18.0F, 1.0F, muted);
    }

    DrawRectangleRec(innerRect, surface);
    DrawRectangleLinesEx(innerRect, 1.0F, border);

    const Color badgeTop = Color{188, 164, 63, 255};
    const Color badgeBottom = Color{141, 118, 19, 255};
    DrawRectangleRounded(badgeRect, 0.16F, 10, badgeBottom);
    DrawRectangleRounded(
        Rectangle{badgeRect.x + 1.0F, badgeRect.y + 1.0F, badgeRect.width - 2.0F,
                  badgeRect.height / 2.0F},
        0.16F, 10, badgeTop);
    drawRectangleRoundedLinesCompat(badgeRect, 0.16F, 10, 2.0F,
                                    Color{118, 99, 22, 255});

    const int badgeNumber = winnerIndex >= 0 ? winnerIndex + 1 : 1;
    drawTextCentered(font, std::to_string(badgeNumber), badgeRect, 48.0F, 1.0F,
                     Color{58, 48, 18, 255});

    drawTextCentered(font, "Victory Claimed",
                     Rectangle{innerRect.x + 20.0F, innerRect.y + 126.0F,
                               innerRect.width - 40.0F, 22.0F},
                     22.0F, 1.0F, statValue);

    std::string winnerLine = "WINNER: ";
    for (std::size_t i = 0; i < currentSnapshot.winnerNames.size(); ++i) {
        if (i > 0) {
            winnerLine += ", ";
        }
        winnerLine += currentSnapshot.winnerNames[i];
    }
    drawTextCentered(font,
                     truncateText(font, winnerLine, 17.0F, 1.0F,
                                  innerRect.width - 24.0F),
                     Rectangle{innerRect.x + 12.0F, innerRect.y + 152.0F,
                               innerRect.width - 24.0F, 22.0F},
                     17.0F, 1.0F, muted);

    DrawRectangleRec(statsLine, border);
    DrawRectangleRec(Rectangle{innerRect.x + innerRect.width / 3.0F,
                               statsLine.y + 14.0F, 1.0F, 52.0F},
                     border);
    DrawRectangleRec(Rectangle{innerRect.x + (innerRect.width * 2.0F) / 3.0F,
                               statsLine.y + 14.0F, 1.0F, 52.0F},
                     border);

    const float statY = statsLine.y + 14.0F;
    const float colW = innerRect.width / 3.0F;
    drawTextCentered(font, "CASH",
                     Rectangle{innerRect.x, statY, colW, 18.0F}, 16.0F, 1.0F,
                     statLabel);
    drawTextCentered(font, "M" + std::to_string(currentSnapshot.winnerCash),
                     Rectangle{innerRect.x, statY + 20.0F, colW, 28.0F}, 30.0F, 1.0F,
                     statValue);

    drawTextCentered(font, "PROPERTI",
                     Rectangle{innerRect.x + colW, statY, colW, 18.0F}, 16.0F, 1.0F,
                     statLabel);
    drawTextCentered(font, std::to_string(currentSnapshot.winnerPropertyCount),
                     Rectangle{innerRect.x + colW, statY + 20.0F, colW, 28.0F},
                     30.0F, 1.0F, statValue);

    drawTextCentered(font, "KARTU",
                     Rectangle{innerRect.x + colW * 2.0F, statY, colW, 18.0F}, 16.0F,
                     1.0F, statLabel);
    drawTextCentered(font, std::to_string(currentSnapshot.winnerCardCount),
                     Rectangle{innerRect.x + colW * 2.0F, statY + 20.0F, colW, 28.0F},
                     30.0F, 1.0F, statValue);

    const Rectangle exitRect = gameOverPopupExitButtonRect();
    const Rectangle newGameRect = gameOverPopupNewGameButtonRect();
    const Rectangle closeRect = gameOverPopupCloseButtonRect();

    DrawRectangleRec(exitRect, Color{214, 214, 214, 255});
    DrawRectangleLinesEx(exitRect, 1.0F, Color{196, 196, 196, 255});
    drawTextCentered(font, "KELUAR", exitRect, 22.0F, 1.0F, Color{88, 82, 74, 255});

    DrawRectangleRec(newGameRect, Color{98, 8, 8, 255});
    DrawRectangleLinesEx(newGameRect, 1.0F, Color{120, 35, 35, 255});
    drawTextCentered(font, "MAIN LAGI", newGameRect, 22.0F, 1.0F,
                     Color{245, 239, 227, 255});

    const Color closeFill = Color{122, 102, 6, 255};
    const Color closeText = contrastingTextColor(closeFill);
    DrawRectangleRec(closeRect, closeFill);
    DrawRectangleLinesEx(closeRect, 1.0F, Color{147, 127, 23, 255});
    drawTextCentered(font, "TUTUP", closeRect, 24.0F, 1.0F, closeText);

    (void)pieceColor;
    (void)textColor;
}
