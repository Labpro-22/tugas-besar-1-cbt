#include "gui/GuiWindowLayout.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

GuiWindow::Layout GuiWindow::computeLayout(const int screenWidth,
                                           const int screenHeight,
                                           const GameSnapshot& currentSnapshot) const {
    Layout layout;
    const float outerPadding = 18.0F;
    const float gap = 12.0F;
    const float headerHeight = 78.0F;
    const float commandHeight = 154.0F;
    const float availableWidth = screenWidth - outerPadding * 2.0F - gap * 2.0F;

    float sideWidth = GuiWindowInternal::clampFloat(availableWidth / 5.5F, 200.0F, 290.0F);
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

    const float rosterHeight = GuiWindowInternal::clampFloat(layout.rightPanelRect.height * 0.40F,
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
        const Rectangle boardSurface = GuiWindowInternal::insetRect(layout.boardRect, 10.0F, 10.0F);
        const int borderThickness =
            GuiWindowInternal::clampInt(static_cast<int>(std::min(boardSurface.width, boardSurface.height) /
                                       7.0F),
                     56, 96);
        const Rectangle centerField{
            boardSurface.x + borderThickness,
            boardSurface.y + borderThickness,
            boardSurface.width - borderThickness * 2.0F,
            boardSurface.height - borderThickness * 2.0F,
        };

        const float buttonWidth2 =
            GuiWindowInternal::clampFloat(centerField.width / 3.0F, 190.0F, 310.0F);
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

    // If modal is active, some updateFrame logic should be skipped
    ModalState currentModal;
    {
        std::lock_guard<std::mutex> lock(modalMutex);
        currentModal = modal;
    }

    if (currentModal.active) {
        // Modal input is handled in updateModalInput
        return;
    }

    if (currentSnapshot.gameOver && currentSnapshot.hasWinnerSummary) {
        bool popupDismissed = false;
        {
            std::lock_guard<std::mutex> lock(snapshotMutex);
            popupDismissed = gameOverPopupDismissed;
        }

        if (!popupDismissed) {
            if (GuiWindowInternal::isButtonPressed(gameOverPopupExitButtonRect(), true)) {
                submitInputLine("EXIT");
                std::lock_guard<std::mutex> lock(snapshotMutex);
                gameOverPopupDismissed = true;
                return;
            }

            if (GuiWindowInternal::isButtonPressed(gameOverPopupNewGameButtonRect(), true)) {
                submitInputLine("NEW_GAME");
                std::lock_guard<std::mutex> lock(snapshotMutex);
                gameOverPopupDismissed = true;
                return;
            }
            
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) ||
                GuiWindowInternal::isButtonPressed(gameOverPopupCloseButtonRect(), true)) {
                std::lock_guard<std::mutex> lock(snapshotMutex);
                gameOverPopupDismissed = true;
                return;
            }
            return;
        }
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

        const std::string logText = GuiWindowInternal::buildTransactionLogText(currentSnapshot);
        const auto lines =
            GuiWindowInternal::wrapText(georgiaFont, logText, 15.0F, 1.0F, textRect.width, 100000);
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
            GuiWindowInternal::computeScrollbarThumb(scrollbarTrack, visibleLines,
                                  static_cast<int>(lines.size()), logScrollLine);

        if (GuiWindowInternal::pointInsideRect(layout.logRect, mouse)) {
            const float wheelMove = GetMouseWheelMove();
            if (wheelMove != 0.0F) {
                logScrollLine = GuiWindowInternal::clampInt(
                    logScrollLine - static_cast<int>(wheelMove) * 3, 0, maxStartLine);
                logAutoScroll = logScrollLine >= maxStartLine;
            }
        }

        if (!logScrollbarDragging && maxStartLine > 0 &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            GuiWindowInternal::pointInsideRect(scrollbarThumb, mouse)) {
            logScrollbarDragging = true;
            logScrollbarGrabOffset = mouse.y - scrollbarThumb.y;
        }

        if (logScrollbarDragging) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                const float thumbTravel =
                    std::max(1.0F, scrollbarTrack.height - scrollbarThumb.height);
                const float newThumbY = GuiWindowInternal::clampFloat(
                    mouse.y - logScrollbarGrabOffset, scrollbarTrack.y,
                    scrollbarTrack.y + scrollbarTrack.height - scrollbarThumb.height);
                const float ratio = (newThumbY - scrollbarTrack.y) / thumbTravel;
                logScrollLine = GuiWindowInternal::clampInt(static_cast<int>(std::round(ratio * maxStartLine)),
                                         0, maxStartLine);
                logAutoScroll = logScrollLine >= maxStartLine;
            } else {
                logScrollbarDragging = false;
            }
        } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) &&
                   !GuiWindowInternal::pointInsideRect(scrollbarThumb, mouse)) {
            logScrollbarDragging = false;
        }
    }

    if (GuiWindowInternal::pointInsideRect(layout.rosterRect, mouse) &&
        IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        for (std::size_t i = 0; i < currentSnapshot.players.size(); ++i) {
            const Rectangle cardRect =
                GuiWindowInternal::calculateRosterCardRect(layout.rosterRect, currentSnapshot,
                                        static_cast<int>(i));
            if (GuiWindowInternal::pointInsideRect(cardRect, mouse)) {
                inspectedPlayerIndex = static_cast<int>(i);
                return;
            }
        }
    }

    if (currentSnapshot.gameStarted && !currentSnapshot.gameOver) {
        if (GuiWindowInternal::isButtonPressed(layout.rollButtonRect, true)) {
            submitInputLine("LEMPAR_DADU");
            return;
        }

        if (GuiWindowInternal::isButtonPressed(layout.scrollLeftRect, commandScrollColumn > 0)) {
            commandScrollColumn--;
            return;
        }
        if (GuiWindowInternal::isButtonPressed(layout.scrollRightRect,
                            commandScrollColumn < commandScrollMaxColumn)) {
            commandScrollColumn++;
            return;
        }

        for (std::size_t i = 0; i < layout.quickButtonRects.size(); ++i) {
            if (GuiWindowInternal::isButtonPressed(layout.quickButtonRects[i], quickButtonEnabled[i])) {
                executeStartedCommand(visibleCommandIndices[i]);
                return;
            }
        }

        if (GuiWindowInternal::isButtonPressed(layout.manualButtonRect, manualEnabled)) {
            openLocalDialog(LocalDialogType::ManualCommand, "Perintah Manual",
                            "Ketik command lengkap.\nContoh: ATUR_DADU 2 5");
            return;
        }
        return;
    }

    for (std::size_t i = 0; i < layout.quickButtonRects.size(); ++i) {
        if (!GuiWindowInternal::isButtonPressed(layout.quickButtonRects[i], quickButtonEnabled[i])) {
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
    ModalState currentModal;
    {
        std::lock_guard<std::mutex> lock(modalMutex);
        currentModal = modal;
    }

    if (!currentModal.active) {
        modalDragging = false;
        return;
    }

    if (!modalPositionInitialized) {
        Rectangle dRect = modalDialogRect();
        modalPosition = Vector2{
            (GetScreenWidth() - dRect.width) / 2.0F,
            std::max(80.0F, (GetScreenHeight() - dRect.height) / 2.5F),
        };
        modalPositionInitialized = true;
        modalScrollOffset = 0.0F;
        modalScrollMax = 0.0F;
    }

    Rectangle dialogRect = modalDialogRect();
    const Rectangle dragRect{dialogRect.x, dialogRect.y, dialogRect.width, 42.0F};
    const Vector2 mouse = GetMousePosition();

    // Mouse wheel scrolling
    float wheel = GetMouseWheelMove();
    if (wheel != 0 && modalScrollMax > 0) {
        modalScrollOffset -= wheel * 30.0F;
        if (modalScrollOffset < 0) modalScrollOffset = 0;
        if (modalScrollOffset > modalScrollMax) modalScrollOffset = modalScrollMax;
    }

    if (!modalDragging && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        GuiWindowInternal::pointInsideRect(dragRect, mouse)) {
        modalDragging = true;
        modalDragOffset = Vector2{mouse.x - dialogRect.x, mouse.y - dialogRect.y};
    }

    if (modalDragging) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            modalPosition = Vector2{mouse.x - modalDragOffset.x,
                                    mouse.y - modalDragOffset.y};
            modalPosition.x = GuiWindowInternal::clampFloat(
                modalPosition.x, 10.0F, GetScreenWidth() - dialogRect.width - 10.0F);
            modalPosition.y = GuiWindowInternal::clampFloat(
                modalPosition.y, 10.0F, GetScreenHeight() - dialogRect.height - 10.0F);
        } else {
            modalDragging = false;
        }
        return; // Skip other input while dragging
    }

    // Keyboard Input for Text
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

    // Interaction with specialized buttons
    std::lock_guard<std::mutex> lock(snapshotMutex);
    if (!snapshot.gameStarted && snapshot.startupMode == "PLAYER_COUNT") {
        float centerY = dialogRect.y + 250;
        Rectangle minusRect = {dialogRect.x + dialogRect.width/2 - 120, centerY - 30, 60, 60};
        Rectangle plusRect = {dialogRect.x + dialogRect.width/2 + 60, centerY - 30, 60, 60};
        
        if (GuiWindowInternal::isButtonPressed(minusRect, true)) {
            std::lock_guard<std::mutex> lock2(modalMutex);
            int pCount = std::atoi(modal.inputText.c_str());
            if (pCount > 2) modal.inputText = std::to_string(pCount - 1);
            else modal.inputText = "2";
            return;
        }
        if (GuiWindowInternal::isButtonPressed(plusRect, true)) {
            std::lock_guard<std::mutex> lock2(modalMutex);
            int pCount = std::atoi(modal.inputText.c_str());
            if (pCount < 4) modal.inputText = std::to_string(pCount + 1);
            else modal.inputText = "4";
            return;
        }

        Rectangle okBtn = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 80, 140, 50};
        Rectangle cancelBtn = {dialogRect.x + dialogRect.width/2 - 150, dialogRect.y + dialogRect.height - 80, 140, 50};
        
        if (GuiWindowInternal::isButtonPressed(okBtn, true)) {
            std::lock_guard<std::mutex> lock2(modalMutex);
            modal.response.accepted = true;
            modal.response.value = modal.inputText.empty() ? "4" : modal.inputText;
            modal.backendResolved = true;
            modal.active = false;
            modalCondition.notify_all();
            return;
        }
        if (GuiWindowInternal::isButtonPressed(cancelBtn, true)) {
            exitRequested.store(true);
            return;
        }
    } else if (!snapshot.gameStarted && snapshot.startupMode == "USERNAME") {
        Rectangle okBtn = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 80, 180, 50};
        Rectangle cancelBtn = {dialogRect.x + dialogRect.width/2 - 190, dialogRect.y + dialogRect.height - 80, 180, 50};
        
        if (GuiWindowInternal::isButtonPressed(okBtn, true) || IsKeyPressed(KEY_ENTER)) {
            confirmLocalDialog();
            return;
        }
        if (GuiWindowInternal::isButtonPressed(cancelBtn, true)) {
            cancelLocalDialog();
            return;
        }
    } else if (currentModal.active && currentModal.prompt.find("[PROPERTY_PURCHASE:") != std::string::npos) {
        Rectangle yaBtn = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 80, 140, 50};
        Rectangle tidakBtn = {dialogRect.x + dialogRect.width/2 - 150, dialogRect.y + dialogRect.height - 80, 140, 50};
        
        if (GuiWindowInternal::isButtonPressed(yaBtn, true) || IsKeyPressed(KEY_Y)) {
            std::lock_guard<std::mutex> lock2(modalMutex);
            modal.response.accepted = true;
            modal.response.value = "y";
            modal.backendResolved = true;
            modal.active = false;
            modalCondition.notify_all();
            return;
        }
        if (GuiWindowInternal::isButtonPressed(tidakBtn, true) || IsKeyPressed(KEY_N)) {
            std::lock_guard<std::mutex> lock2(modalMutex);
            modal.response.accepted = true;
            modal.response.value = "n";
            modal.backendResolved = true;
            modal.active = false;
            modalCondition.notify_all();
            return;
        }
    } else if (currentModal.active && currentModal.prompt.find("Tangan penuh!") != std::string::npos) {
        // Discard Modal Clicks
        float sideWidth = 340.0F;
        float rightX = dialogRect.x + sideWidth;
        float rightWidth = dialogRect.width - sideWidth;
        
        std::vector<std::string> cards;
        std::stringstream ss(currentModal.prompt);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.find(". ") != std::string::npos) cards.push_back(line);
        }

        float cardW = 145.0F; float cardH = 220.0F; float spacing = 15.0F;
        float totalW = cards.size() * cardW + (cards.size() - 1) * spacing;
        float startX = rightX + (rightWidth - totalW) / 2.0F;
        float startY = dialogRect.y + 140.0F;

        for (size_t i = 0; i < cards.size(); ++i) {
            Rectangle cr = {startX + i * (cardW + spacing), startY, cardW, cardH};
            if (GuiWindowInternal::isButtonPressed(cr, true)) {
                std::lock_guard<std::mutex> lock2(modalMutex);
                modal.inputText = std::to_string(i + 1);
                return;
            }
        }

        Rectangle btnConfirm = {rightX + (rightWidth - 220)/2.0F, dialogRect.y + dialogRect.height - 100, 220, 55};
        if (GuiWindowInternal::isButtonPressed(btnConfirm, !currentModal.inputText.empty())) {
            confirmLocalDialog();
            return;
        }
    } else if (currentModal.active && currentModal.prompt.find("Pilih kartu yang ingin digunakan") != std::string::npos) {
        // Use Card Modal Clicks
        std::vector<std::string> cards;
        std::stringstream ss(currentModal.prompt);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.find(". ") != std::string::npos) cards.push_back(line);
        }

        float cardY = dialogRect.y + 130.0F + 110.0F; // header is 110
        cardY = dialogRect.y + 130.0F; 
        for (size_t i = 0; i < cards.size(); ++i) {
            Rectangle itemRect = {dialogRect.x + 40, cardY, dialogRect.width - 80, 80};
            if (GuiWindowInternal::isButtonPressed(itemRect, true)) {
                std::lock_guard<std::mutex> lock2(modalMutex);
                modal.inputText = std::to_string(i + 1);
                return;
            }
            cardY += 95.0F;
        }

        Rectangle confirmBtn = {dialogRect.x + 40, dialogRect.y + dialogRect.height - 85, dialogRect.width - 240, 55};
        Rectangle cancelBtn = {dialogRect.x + dialogRect.width - 180, dialogRect.y + dialogRect.height - 85, 140, 55};
        
        if (GuiWindowInternal::isButtonPressed(confirmBtn, !currentModal.inputText.empty())) {
            confirmLocalDialog();
        } else if (GuiWindowInternal::isButtonPressed(cancelBtn, true)) {
            cancelLocalDialog();
        }
        return;
    } else if (currentModal.active && currentModal.prompt.find("Tangan penuh!") != std::string::npos) {
        // Discard Modal Clicks
        float sideWidth = 340.0F;
        float rightX = dialogRect.x + sideWidth;
        float rightWidth = dialogRect.width - sideWidth;
        
        std::vector<std::string> cards;
        std::stringstream ss(currentModal.prompt);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.find(". ") != std::string::npos) cards.push_back(line);
        }

        float cardW = 145.0F; float cardH = 220.0F; float spacing = 15.0F;
        float totalW = cards.size() * cardW + (cards.size() - 1) * spacing;
        float startX = rightX + (rightWidth - totalW) / 2.0F;
        float startY = dialogRect.y + 140.0F;

        for (size_t i = 0; i < cards.size(); ++i) {
            Rectangle cr = {startX + i * (cardW + spacing), startY, cardW, cardH};
            if (GuiWindowInternal::isButtonPressed(cr, true)) {
                std::lock_guard<std::mutex> lock2(modalMutex);
                modal.inputText = std::to_string(i + 1);
                return;
            }
        }

        Rectangle btnConfirm = {rightX + (rightWidth - 220)/2.0F, dialogRect.y + dialogRect.height - 100, 220, 55};
        if (GuiWindowInternal::isButtonPressed(btnConfirm, !currentModal.inputText.empty())) {
            confirmLocalDialog();
            return;
        }
    } else if (currentModal.active && currentModal.prompt.find("Pilih kartu yang ingin digunakan") != std::string::npos) {
        // Use Card Modal Clicks
        std::vector<std::string> cards;
        std::stringstream ss(currentModal.prompt);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.find(". ") != std::string::npos) cards.push_back(line);
        }

        float cardY = dialogRect.y + 130.0F; 
        for (size_t i = 0; i < cards.size(); ++i) {
            Rectangle itemRect = {dialogRect.x + 40, cardY, dialogRect.width - 80, 80};
            if (GuiWindowInternal::isButtonPressed(itemRect, true)) {
                std::lock_guard<std::mutex> lock2(modalMutex);
                modal.inputText = std::to_string(i + 1);
                return;
            }
            cardY += 95.0F;
        }

        Rectangle confirmBtn = {dialogRect.x + 40, dialogRect.y + dialogRect.height - 85, dialogRect.width - 240, 55};
        Rectangle cancelBtn = {dialogRect.x + dialogRect.width - 180, dialogRect.y + dialogRect.height - 85, 140, 55};
        
        if (GuiWindowInternal::isButtonPressed(confirmBtn, !currentModal.inputText.empty())) {
            confirmLocalDialog();
        } else if (GuiWindowInternal::isButtonPressed(cancelBtn, true)) {
            cancelLocalDialog();
        }
        return;
    } else if (currentModal.active && currentModal.prompt.find("Pilih target LassoCard") != std::string::npos) {
        // Lasso Target Clicks
        std::vector<std::string> candidates;
        std::stringstream ss(currentModal.prompt);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.find(". ") != std::string::npos && line.find("(Tile ") != std::string::npos) candidates.push_back(line);
        }

        float gridX = dialogRect.x + 40.0F;
        float gridY = dialogRect.y + 210.0F;
        float itemW = (dialogRect.width - 110.0F) / 2.0F;
        float itemH = 160.0F;

        for (size_t i = 0; i < candidates.size(); ++i) {
            Rectangle itemR = {gridX + (i % 2) * (itemW + 30), gridY + (i / 2) * (itemH + 30), itemW, itemH};
            if (GuiWindowInternal::isButtonPressed(itemR, true)) {
                std::lock_guard<std::mutex> lock2(modalMutex);
                modal.inputText = std::to_string(i + 1);
                return;
            }
        }

        Rectangle targetBtn = {dialogRect.x + dialogRect.width - 250, dialogRect.y + dialogRect.height - 85, 210, 55};
        if (GuiWindowInternal::isButtonPressed(targetBtn, !currentModal.inputText.empty())) {
            confirmLocalDialog();
        }
        return;
    } else if (currentModal.active && currentModal.prompt.find("DemolitionCard") != std::string::npos) {
        // Demolition Interactions
        float sideW = 150.0F;
        float listX = dialogRect.x + sideW + 20;
        float listY = dialogRect.y + 110;
        float listW = dialogRect.width - sideW - 40;
        float listH = dialogRect.height - 230;

        std::vector<std::string> targets;
        std::stringstream ss(currentModal.prompt);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.find(". ") != std::string::npos && line.find(" - ") != std::string::npos) targets.push_back(line);
        }

        float itemH = 125.0F;
        float spacing = 15.0F;

        for (size_t i = 0; i < targets.size(); ++i) {
            float y = listY + i * (itemH + spacing) - modalScrollOffset;
            if (y + itemH < listY || y > listY + listH) continue;
            Rectangle itemR = {listX, y, listW, itemH};
            if (GuiWindowInternal::isButtonPressed(itemR, true)) {
                std::lock_guard<std::mutex> lock2(modalMutex);
                modal.inputText = std::to_string(i + 1);
                return;
            }
        }

        Rectangle confirmBtnDemolish = {listX, dialogRect.y + dialogRect.height - 100, listW, 80};
        if (GuiWindowInternal::isButtonPressed(confirmBtnDemolish, !currentModal.inputText.empty())) {
            confirmLocalDialog();
        }
        return;
    } else if (currentModal.active && currentModal.prompt.find("Pilih tile tujuan teleport") != std::string::npos) {
        // Teleport Interactions
        Rectangle inputRect = {dialogRect.x + (dialogRect.width - 180)/2, dialogRect.y + 190, 180, 80};
        Rectangle btnUp = {inputRect.x + inputRect.width - 30, inputRect.y + 10, 30, 30};
        Rectangle btnDown = {inputRect.x + inputRect.width - 30, inputRect.y + 40, 30, 30};
        
        if (GuiWindowInternal::isButtonPressed(btnUp, true)) {
            std::lock_guard<std::mutex> lock2(modalMutex);
            int val = std::atoi(modal.inputText.c_str());
            modal.inputText = std::to_string((val + 1) % 60);
            return;
        }
        if (GuiWindowInternal::isButtonPressed(btnDown, true)) {
            std::lock_guard<std::mutex> lock2(modalMutex);
            int val = std::atoi(modal.inputText.c_str());
            modal.inputText = std::to_string((val - 1 + 60) % 60);
            return;
        }

        Rectangle okBtn = {dialogRect.x + 40, dialogRect.y + dialogRect.height - 90, (dialogRect.width - 100)/2, 60};
        Rectangle cancelBtn = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 90, (dialogRect.width - 100)/2, 60};
        
        if (GuiWindowInternal::isButtonPressed(okBtn, true) || IsKeyPressed(KEY_ENTER)) confirmLocalDialog();
        else if (GuiWindowInternal::isButtonPressed(cancelBtn, true)) cancelLocalDialog();
        return;

    } else if (currentModal.active && currentModal.prompt.find("Aksi lelang") != std::string::npos) {
        // Auction Interactions
        Rectangle cancelBtn = {dialogRect.x + 40, dialogRect.y + dialogRect.height - 90, (dialogRect.width - 100)/2, 70};
        Rectangle okBtn = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 90, (dialogRect.width - 100)/2, 70};
        
        if (GuiWindowInternal::isButtonPressed(okBtn, true) || IsKeyPressed(KEY_ENTER)) confirmLocalDialog();
        else if (GuiWindowInternal::isButtonPressed(cancelBtn, true)) cancelLocalDialog();
        return;

    } else if (currentModal.active && currentModal.title.find("Festival") != std::string::npos) {
        // Festival Interactions
        Rectangle cancelBtn = {dialogRect.x + dialogRect.width - 310, dialogRect.y + dialogRect.height - 85, 140, 60};
        Rectangle okBtn = {dialogRect.x + dialogRect.width - 160, dialogRect.y + dialogRect.height - 85, 120, 60};
        
        if (GuiWindowInternal::isButtonPressed(okBtn, true) || IsKeyPressed(KEY_ENTER)) confirmLocalDialog();
        else if (GuiWindowInternal::isButtonPressed(cancelBtn, true)) cancelLocalDialog();
        return;

    } else if (currentModal.active && (currentModal.title == "Pilih Opsi" || currentModal.prompt.find("Pilihan (1/") != std::string::npos)) {
        // Choice Interactions
        Rectangle inputRect = {dialogRect.x + 40, dialogRect.y + 210, dialogRect.width - 80, 80};
        Rectangle btnUp = {inputRect.x + inputRect.width - 40, inputRect.y + 10, 40, 35};
        Rectangle btnDown = {inputRect.x + inputRect.width - 40, inputRect.y + 45, 40, 35};
        
        bool isThreeChoices = (currentModal.prompt.find("(1/2/3)") != std::string::npos);
        int maxVal = isThreeChoices ? 3 : 2;

        if (GuiWindowInternal::isButtonPressed(btnUp, true)) {
            std::lock_guard<std::mutex> lock2(modalMutex);
            int val = std::atoi(modal.inputText.c_str());
            if (val <= 0) val = 1;
            val = (val % maxVal) + 1;
            modal.inputText = std::to_string(val);
            return;
        }
        if (GuiWindowInternal::isButtonPressed(btnDown, true)) {
            std::lock_guard<std::mutex> lock2(modalMutex);
            int val = std::atoi(modal.inputText.c_str());
            if (val <= 0) val = 1;
            val = (val - 2 + maxVal) % maxVal + 1;
            modal.inputText = std::to_string(val);
            return;
        }

        Rectangle okBtn = {dialogRect.x + 40, dialogRect.y + dialogRect.height - 90, (dialogRect.width - 100)/2, 65};
        Rectangle cancelBtn = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 90, (dialogRect.width - 100)/2, 65};
        
        if (GuiWindowInternal::isButtonPressed(okBtn, true) || IsKeyPressed(KEY_ENTER)) confirmLocalDialog();
        else if (GuiWindowInternal::isButtonPressed(cancelBtn, true)) cancelLocalDialog();
        return;

    } else {
        const Rectangle okRect{dialogRect.x + dialogRect.width - 210.0F,
                               dialogRect.y + dialogRect.height - 58.0F, 90.0F,
                               34.0F};
        const Rectangle cancelRect{dialogRect.x + dialogRect.width - 110.0F,
                                   dialogRect.y + dialogRect.height - 58.0F, 90.0F,
                                   34.0F};
        const Rectangle errorOkRect{dialogRect.x + dialogRect.width - 120.0F,
                                    dialogRect.y + dialogRect.height - 58.0F,
                                    90.0F, 34.0F};

        if (currentModal.localType == LocalDialogType::ErrorMessage) {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) ||
                GuiWindowInternal::isButtonPressed(errorOkRect, true)) {
                confirmLocalDialog();
            }
        } else if (currentModal.backendOwned && currentModal.yesNo) {
            if (GuiWindowInternal::isButtonPressed(okRect, true)) {
                std::lock_guard<std::mutex> lock2(modalMutex);
                modal.response.accepted = true;
                modal.response.value = "y";
                modal.backendResolved = true;
                modal.active = false;
                modalCondition.notify_all();
            } else if (GuiWindowInternal::isButtonPressed(cancelRect, true)) {
                std::lock_guard<std::mutex> lock2(modalMutex);
                modal.response.accepted = true;
                modal.response.value = "n";
                modal.backendResolved = true;
                modal.active = false;
                modalCondition.notify_all();
            }
        } else {
            if (IsKeyPressed(KEY_ENTER) || GuiWindowInternal::isButtonPressed(okRect, true)) {
                confirmLocalDialog();
            } else if (IsKeyPressed(KEY_ESCAPE) || GuiWindowInternal::isButtonPressed(cancelRect, true)) {
                cancelLocalDialog();
            }
        }
    }
}
