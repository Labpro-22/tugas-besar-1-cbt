#include "../../../include/gui/GuiWindowInternal.hpp"

using namespace gui_internal;

GuiWindow::Layout GuiWindow::computeLayout(const int screenWidth,
                                           const int screenHeight,
                                           const GameSnapshot& currentSnapshot) const {
    Layout layout;
    const float outerPadding = 18.0F;
    const float gap = 12.0F;
    const float headerHeight = 78.0F;
    const float commandHeight = 154.0F;
    const float availableWidth = screenWidth - outerPadding * 2.0F - gap * 2.0F;

    float sideWidth = clampFloat(availableWidth / 5.5F, 200.0F, 290.0F);
    if (availableWidth - sideWidth * 2.0F < 520.0F) {
        sideWidth = std::max(185.0F, (availableWidth - 520.0F) / 2.0F);
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
                     56, 96);
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
    {
        std::lock_guard<std::mutex> lock(snapshotMutex);
        if (turnPopupTimer > 0.0F) {
            turnPopupTimer = std::max(0.0F, turnPopupTimer - GetFrameTime());
            if (turnPopupTimer <= 0.0F) {
                turnPopupPlayerIndex = -1;
            }
        }
    }

    openPendingErrorPopup();
    updateModalInput();

    if (currentSnapshot.gameOver && currentSnapshot.hasWinnerSummary) {
        bool popupDismissed = false;
        {
            std::lock_guard<std::mutex> lock(snapshotMutex);
            popupDismissed = gameOverPopupDismissed;
        }

        if (!popupDismissed) {
            if (isButtonPressed(gameOverPopupExitButtonRect(), true)) {
                submitInputLine("EXIT");
                std::lock_guard<std::mutex> lock(snapshotMutex);
                gameOverPopupDismissed = true;
                return;
            }

            if (isButtonPressed(gameOverPopupNewGameButtonRect(), true)) {
                submitInputLine("NEW_GAME");
                std::lock_guard<std::mutex> lock(snapshotMutex);
                gameOverPopupDismissed = true;
                return;
            }
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) ||
            isButtonPressed(gameOverPopupCloseButtonRect(), true)) {
            std::lock_guard<std::mutex> lock(snapshotMutex);
            gameOverPopupDismissed = true;
            return;
        }

        if (!popupDismissed) {
            return;
        }
    }

    ModalState currentModal;
    {
        std::lock_guard<std::mutex> lock(modalMutex);
        currentModal = modal;
    }

    if (currentModal.active) {
        if (currentModal.localType != LocalDialogType::ErrorMessage &&
            (!currentModal.backendOwned || !currentModal.yesNo)) {
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

        const Rectangle dialogRect = modalDialogRect();
        const Rectangle okRect{dialogRect.x + dialogRect.width - 210.0F,
                               dialogRect.y + dialogRect.height - 58.0F, 90.0F,
                               34.0F};
        const Rectangle cancelRect{dialogRect.x + dialogRect.width - 110.0F,
                                   dialogRect.y + dialogRect.height - 58.0F, 90.0F,
                                   34.0F};
        const Rectangle errorOkRect{dialogRect.x + dialogRect.width - 120.0F,
                                    dialogRect.y + dialogRect.height - 58.0F,
                                    90.0F, 34.0F};

        if (currentModal.backendOwned &&
            currentModal.request.kind == InputPromptKind::Choice &&
            !currentSnapshot.gameStarted) {
            for (std::size_t i = 0; i < layout.quickButtonRects.size(); ++i) {
                if (!isButtonPressed(layout.quickButtonRects[i],
                                     quickButtonEnabled[i])) {
                    continue;
                }

                std::string value;
                if (currentSnapshot.startupMode == "PLAYER_COUNT") {
                    if (i == 0) value = "2";
                    if (i == 1) value = "3";
                    if (i == 2) value = "4";
                } else if (currentSnapshot.startupMode == "MAIN_MENU") {
                    if (i == 0) value = "1";
                    if (i == 1) value = "2";
                    if (i == 2) value = "0";
                }

                if (!value.empty()) {
                    std::lock_guard<std::mutex> lock(modalMutex);
                    modal.response.accepted = true;
                    modal.response.value = value;
                    modal.backendResolved = true;
                    modal.active = false;
                    modalDragging = false;
                    modalCondition.notify_all();
                    if (currentSnapshot.startupMode == "MAIN_MENU" &&
                        value == "0") {
                        exitRequested.store(true);
                    }
                    return;
                }
            }
        }

        if (currentModal.localType == LocalDialogType::ErrorMessage) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) ||
                isButtonPressed(errorOkRect, true)) {
                confirmLocalDialog();
            }
        } else if (currentModal.backendOwned && currentModal.yesNo) {
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
    {
        const float logLineHeight = 19.0F;
        const float logScrollbarWidth = 12.0F;
        const Rectangle textRect{
            layout.logRect.x,
            layout.logRect.y,
            std::max(20.0F, layout.logRect.width - logScrollbarWidth - 10.0F),
            layout.logRect.height};

        std::string outputCopy;
        {
            std::lock_guard<std::mutex> lock(outputMutex);
            outputCopy = outputText;
        }

        const auto lines =
            wrapText(georgiaFont, outputCopy, 15.0F, 1.0F, textRect.width, 100000);
        const int visibleLines =
            std::max(1, static_cast<int>(layout.logRect.height / logLineHeight));
        const int maxStartLine =
            std::max(0, static_cast<int>(lines.size()) - visibleLines);
        const Rectangle scrollbarTrack{
            layout.logRect.x + layout.logRect.width - logScrollbarWidth,
            layout.logRect.y,
            logScrollbarWidth,
            layout.logRect.height};
        const Rectangle scrollbarThumb =
            computeScrollbarThumb(scrollbarTrack, visibleLines,
                                  static_cast<int>(lines.size()), logScrollLine);

        if (pointInsideRect(layout.logRect, mouse)) {
            const float wheelMove = GetMouseWheelMove();
            if (wheelMove != 0.0F) {
                logScrollLine = clampInt(
                    logScrollLine - static_cast<int>(wheelMove) * 3, 0, maxStartLine);
                logAutoScroll = logScrollLine >= maxStartLine;
            }
        }

        if (!logScrollbarDragging && maxStartLine > 0 &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            pointInsideRect(scrollbarThumb, mouse)) {
            logScrollbarDragging = true;
            logScrollbarGrabOffset = mouse.y - scrollbarThumb.y;
        }

        if (logScrollbarDragging) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                const float thumbTravel =
                    std::max(1.0F, scrollbarTrack.height - scrollbarThumb.height);
                const float newThumbY = clampFloat(
                    mouse.y - logScrollbarGrabOffset, scrollbarTrack.y,
                    scrollbarTrack.y + scrollbarTrack.height - scrollbarThumb.height);
                const float ratio = (newThumbY - scrollbarTrack.y) / thumbTravel;
                logScrollLine = clampInt(static_cast<int>(std::round(ratio * maxStartLine)),
                                         0, maxStartLine);
                logAutoScroll = logScrollLine >= maxStartLine;
            } else {
                logScrollbarDragging = false;
            }
        } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) &&
                   !pointInsideRect(scrollbarThumb, mouse)) {
            logScrollbarDragging = false;
        }
    }

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

    if (currentSnapshot.gameStarted && !currentSnapshot.gameOver) {
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

        if (currentSnapshot.gameStarted && currentSnapshot.gameOver) {
            executeStartedCommand(visibleCommandIndices[i]);
            return;
        }

        if (currentSnapshot.startupMode == "PLAYER_COUNT") {
            if (i == 0) submitInputLine("2");
            if (i == 1) submitInputLine("3");
            if (i == 2) submitInputLine("4");
            return;
        }

        if (i == 0) submitInputLine("1");
        if (i == 1) submitInputLine("2");
        if (i == 2) {
            submitInputLine("0");
            exitRequested.store(true);
        }
        return;
    }
}

void GuiWindow::updateModalInput() {
    std::lock_guard<std::mutex> lock(modalMutex);
    if (!modal.active) {
        modalDragging = false;
        return;
    }

    if (!modalPositionInitialized) {
        modalPosition = Vector2{
            (GetScreenWidth() - 500.0F) / 2.0F,
            std::max(96.0F, GetScreenHeight() * 0.18F),
        };
        modalPositionInitialized = true;
    }

    Rectangle dialogRect = modalDialogRect();
    const Rectangle dragRect{dialogRect.x, dialogRect.y, dialogRect.width, 42.0F};
    const Vector2 mouse = GetMousePosition();

    if (!modalDragging && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        pointInsideRect(dragRect, mouse)) {
        modalDragging = true;
        modalDragOffset = Vector2{mouse.x - dialogRect.x, mouse.y - dialogRect.y};
    }

    if (modalDragging) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            modalPosition = Vector2{mouse.x - modalDragOffset.x,
                                    mouse.y - modalDragOffset.y};
            modalPosition.x = clampFloat(
                modalPosition.x, 10.0F, GetScreenWidth() - dialogRect.width - 10.0F);
            modalPosition.y = clampFloat(
                modalPosition.y, 10.0F, GetScreenHeight() - dialogRect.height - 10.0F);
        } else {
            modalDragging = false;
        }
    }
}

