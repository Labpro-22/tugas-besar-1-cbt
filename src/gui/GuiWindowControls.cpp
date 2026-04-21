#include "GuiWindowInternal.hpp"

using namespace gui_internal;

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
