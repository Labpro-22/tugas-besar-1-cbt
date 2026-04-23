#include "gui/GuiWindowControls.hpp"

#include <algorithm>
#include <sstream>

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
        GuiWindowInternal::drawButton(font, layout.quickButtonRects[i], quickButtonLabels[i],
                   quickButtonEnabled[i], false);
    }

    if (currentSnapshot.gameStarted && !currentSnapshot.gameOver) {
        GuiWindowInternal::drawButton(font, layout.scrollLeftRect, "<", commandScrollColumn > 0, false);
        GuiWindowInternal::drawButton(font, layout.scrollRightRect, ">", commandScrollColumn < commandScrollMaxColumn,
                   false);

        std::ostringstream scrollInfo;
        const int firstVisible = commandScrollColumn * 2 + 1;
        const int lastVisible = std::min(
            static_cast<int>(GuiWindowInternal::kStartedQuickActions.size()), firstVisible + 5);
        scrollInfo << firstVisible << "-" << lastVisible << " / "
                   << GuiWindowInternal::kStartedQuickActions.size();
        GuiWindowInternal::drawTextCentered(font, scrollInfo.str(),
                         Rectangle{layout.scrollLeftRect.x + layout.scrollLeftRect.width +
                                       10.0F,
                                   layout.scrollLeftRect.y,
                                   layout.commandRect.width - 96.0F, 20.0F},
                         14.0F, 1.0F, GuiWindowInternal::kMuted);
    }

    GuiWindowInternal::drawButton(font, layout.manualButtonRect,
               manualEnabled ? "PERINTAH MANUAL" : "PERINTAH MANUAL NONAKTIF",
               manualEnabled, false);
}

void GuiWindow::drawLogPanel(const Layout& layout,
                             const GameSnapshot& currentSnapshot) {
    const Font& font = georgiaFont;
    DrawTextEx(font, "CATATAN TRANSAKSI",
               Vector2{layout.rightPanelRect.x + 22.0F,
                       layout.rosterRect.y + layout.rosterRect.height + 10.0F},
               20.0F, 1.0F, GuiWindowInternal::kAccentDark);

    const float scrollbarWidth = 12.0F;
    const Rectangle textRect{
        layout.logRect.x,
        layout.logRect.y,
        std::max(20.0F, layout.logRect.width - scrollbarWidth - 10.0F),
        layout.logRect.height};
    const std::string logText = GuiWindowInternal::buildTransactionLogText(currentSnapshot);
    const auto lines =
        GuiWindowInternal::wrapText(font, logText, 15.0F, 1.0F, textRect.width, 100000);
    const float lineHeight = 19.0F;
    const int visibleLines = std::max(1, static_cast<int>(layout.logRect.height / lineHeight));
    const int maxStartLine =
        std::max(0, static_cast<int>(lines.size()) - visibleLines);
    if (logAutoScroll) {
        logScrollLine = maxStartLine;
    } else {
        logScrollLine = GuiWindowInternal::clampInt(logScrollLine, 0, maxStartLine);
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
                   Vector2{textRect.x, y}, 15.0F, 1.0F, GuiWindowInternal::kInk);
        y += lineHeight;
    }
    EndScissorMode();

    const Rectangle scrollbarTrack{
        layout.logRect.x + layout.logRect.width - scrollbarWidth,
        layout.logRect.y,
        scrollbarWidth,
        layout.logRect.height};
    DrawRectangleRec(scrollbarTrack, Color{236, 231, 221, 255});
    DrawRectangleLinesEx(scrollbarTrack, 1.0F, GuiWindowInternal::kPanelBorder);

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
        DrawRectangleLinesEx(thumb, 1.0F, GuiWindowInternal::kAccentDark);
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
    const Rectangle dialogRect = modalDialogRect();
    const bool isErrorModal = current.localType == LocalDialogType::ErrorMessage;
    
    // Base Dialog Shadow & Background
    DrawRectangleRec(Rectangle{dialogRect.x + 6, dialogRect.y + 6, dialogRect.width, dialogRect.height}, Color{0, 0, 0, 40});
    DrawRectangleRec(dialogRect, GuiWindowInternal::kCream);
    DrawRectangleLinesEx(dialogRect, 1.0F, GuiWindowInternal::kAccentDark);
    DrawRectangleRec(Rectangle{dialogRect.x, dialogRect.y, dialogRect.width, 6.0F}, GuiWindowInternal::kAccent);

    if (!currentSnapshot.gameStarted && currentSnapshot.startupMode == "PLAYER_COUNT") {
        // --- PLAYER COUNT DESIGN ---
        GuiWindowInternal::drawTextCentered(font, "Pilih Opsi", Rectangle{dialogRect.x, dialogRect.y + 30, dialogRect.width, 60}, 48.0F, 1.5F, GuiWindowInternal::kAccent);
        GuiWindowInternal::drawWrappedText(font, "Tentukan jumlah eksekutif aset yang akan berpartisipasi dalam transaksi properti ini.", 
                        Rectangle{dialogRect.x + 40, dialogRect.y + 100, dialogRect.width - 80, 80}, 18.0F, 1.0F, GuiWindowInternal::kInk, 3);
        
        GuiWindowInternal::drawTextCentered(font, "JUMLAH PEMAIN", Rectangle{dialogRect.x, dialogRect.y + 180, dialogRect.width, 30}, 16.0F, 2.0F, GuiWindowInternal::kAccentDark);
        
        float centerY = dialogRect.y + 250;
        Rectangle minusRect = {dialogRect.x + dialogRect.width/2 - 120, centerY - 30, 60, 60};
        Rectangle valueRect = {dialogRect.x + dialogRect.width/2 - 50, centerY - 40, 100, 80};
        Rectangle plusRect = {dialogRect.x + dialogRect.width/2 + 60, centerY - 30, 60, 60};
        
        DrawRectangleRec(minusRect, WHITE);
        DrawRectangleLinesEx(minusRect, 1.0F, GuiWindowInternal::kPanelBorder);
        GuiWindowInternal::drawTextCentered(font, "-", minusRect, 32.0F, 1.0F, GuiWindowInternal::kAccent);
        
        DrawRectangleRec(valueRect, WHITE);
        DrawRectangleLinesEx(valueRect, 3.0F, GuiWindowInternal::kAccent);
        GuiWindowInternal::drawTextCentered(font, current.inputText.empty() ? "4" : current.inputText, valueRect, 56.0F, 1.0F, GuiWindowInternal::kAccent);
        
        DrawRectangleRec(plusRect, WHITE);
        DrawRectangleLinesEx(plusRect, 1.0F, GuiWindowInternal::kPanelBorder);
        GuiWindowInternal::drawTextCentered(font, "+", plusRect, 32.0F, 1.0F, GuiWindowInternal::kAccent);
        
        float iconRowY = dialogRect.y + 350;
        int pCount = std::atoi(current.inputText.c_str());
        if (pCount <= 0) pCount = 4;
        for (int i = 0; i < 4; ++i) {
            float x = dialogRect.x + dialogRect.width/2 - 67 + i * 45;
            Color iconColor = (i < pCount) ? GuiWindowInternal::kGold : LIGHTGRAY;
            DrawCircle(x, iconRowY, 12, iconColor);
            DrawTextEx(font, std::to_string(i+1).c_str(), {x - 4, iconRowY + 15}, 12, 1, DARKGRAY);
        }

        Rectangle okRect = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 80, 140, 50};
        Rectangle cancelRect = {dialogRect.x + dialogRect.width/2 - 150, dialogRect.y + dialogRect.height - 80, 140, 50};
        DrawRectangleRec(okRect, GuiWindowInternal::kAccent);
        GuiWindowInternal::drawTextCentered(font, "OK", okRect, 20.0F, 1.0F, WHITE);
        DrawRectangleLinesEx(cancelRect, 2.0F, GuiWindowInternal::kAccent);
        GuiWindowInternal::drawTextCentered(font, "BATAL", cancelRect, 20.0F, 1.0F, GuiWindowInternal::kAccent);

    } else if (!currentSnapshot.gameStarted && currentSnapshot.startupMode == "USERNAME") {
        // --- USERNAME DESIGN ---
        float iconY = dialogRect.y + 40;
        DrawRectangleRounded({dialogRect.x + dialogRect.width/2 - 30, iconY, 60, 60}, 0.2F, 8, GuiWindowInternal::kGold);
        DrawCircle(dialogRect.x + dialogRect.width/2, iconY + 22, 10, GuiWindowInternal::kAccent);
        DrawEllipse(dialogRect.x + dialogRect.width/2, iconY + 45, 18, 12, GuiWindowInternal::kAccent);

        GuiWindowInternal::drawTextCentered(font, "Username Pemain", Rectangle{dialogRect.x, dialogRect.y + 110, dialogRect.width, 60}, 42.0F, 1.2F, GuiWindowInternal::kAccent);
        GuiWindowInternal::drawWrappedText(font, "Tentukan identitas pemain aset Anda untuk memulai sesi ini.", 
                        Rectangle{dialogRect.x + 40, dialogRect.y + 170, dialogRect.width - 80, 50}, 18.0F, 1.0F, GuiWindowInternal::kInk, 2);
        
        std::string playerNumStr = std::to_string(currentSnapshot.startupCollectedPlayers + 1);
        std::string playerLabel = "Player " + playerNumStr + " (Nama Pemain)";
        DrawTextEx(font, playerLabel.c_str(), {dialogRect.x + 40, dialogRect.y + 230}, 16, 1, GuiWindowInternal::kMuted);
        DrawTextEx(font, (std::to_string(current.inputText.length()) + " / 8 KARAKTER").c_str(), {dialogRect.x + dialogRect.width - 160, dialogRect.y + 230}, 14, 1, GuiWindowInternal::kGold);

        Rectangle inputRect = {dialogRect.x + 40, dialogRect.y + 255, dialogRect.width - 80, 70};
        DrawRectangleRec(inputRect, {245, 235, 220, 255});
        DrawRectangleLinesEx(inputRect, 1.0F, GuiWindowInternal::kPanelBorder);
        
        std::string placeholder = "PLAYER" + playerNumStr;
        DrawTextEx(font, current.inputText.empty() ? placeholder.c_str() : current.inputText.c_str(), {inputRect.x + 20, inputRect.y + 20}, 32, 1, current.inputText.empty() ? LIGHTGRAY : GuiWindowInternal::kInk);
        
        Rectangle okRect = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 80, 180, 50};
        Rectangle cancelRect = {dialogRect.x + dialogRect.width/2 - 190, dialogRect.y + dialogRect.height - 80, 180, 50};
        DrawRectangleRec(okRect, GuiWindowInternal::kAccent);
        GuiWindowInternal::drawTextCentered(font, "OK \x1a", okRect, 20.0F, 1.0F, WHITE);
        DrawRectangleLinesEx(cancelRect, 2.0F, GuiWindowInternal::kAccent);
        GuiWindowInternal::drawTextCentered(font, "BATAL", cancelRect, 20.0F, 1.0F, GuiWindowInternal::kAccent);

    } else if (current.active && current.prompt.find("[PROPERTY_PURCHASE:") != std::string::npos) {
        // --- PROPERTY PURCHASE DESIGN ---
        GuiWindowInternal::drawTextCentered(font, "Konfirmasi", Rectangle{dialogRect.x + 20, dialogRect.y + 15, 120, 30}, 24.0F, 1.0F, GuiWindowInternal::kAccent);
        
        if (currentSnapshot.activePlayerIndex >= 0 && currentSnapshot.activePlayerIndex < static_cast<int>(currentSnapshot.players.size())) {
            std::string giliranText = "Giliran: " + currentSnapshot.players[currentSnapshot.activePlayerIndex].name;
            float tw = MeasureTextEx(font, giliranText.c_str(), 18, 1).x;
            DrawTextEx(font, giliranText.c_str(), {dialogRect.x + dialogRect.width - tw - 20, dialogRect.y + 20}, 18, 1, GuiWindowInternal::kAccent);
        }

        // Extract property info from snapshot
        std::string propName = "PROPERTI TIDAK DIKENAL";
        std::string buyPriceStr = "M 0";
        std::string rentPriceStr = "M 0";
        std::string mortgageStr = "M 0";
        std::string detailRentLabel = "RENT";
        std::string detailValueStr = "M 0";

        auto formatMoney = [](int amount) {
            std::string s = std::to_string(amount);
            int n = s.length();
            for (int i = n - 3; i > 0; i -= 3) {
                s.insert(i, ".");
            }
            return "M " + s;
        };

        size_t startPos = current.prompt.find("[PROPERTY_PURCHASE:");
        if (startPos != std::string::npos) {
            startPos += 19;
            size_t endPos = current.prompt.find("]", startPos);
            if (endPos != std::string::npos) {
                std::string propCode = current.prompt.substr(startPos, endPos - startPos);
                for (const auto& t : currentSnapshot.tiles) {
                    if (t.code == propCode) {
                        propName = t.name;
                        buyPriceStr = formatMoney(t.buyPrice);
                        rentPriceStr = formatMoney(t.rentPrice);
                        mortgageStr = formatMoney(t.mortgagePrice);
                        if (t.type == "property") {
                            detailRentLabel = "INITIAL RENT";
                            detailValueStr = rentPriceStr;
                        }
                        break;
                    }
                }
            }
        }

        Rectangle cardRect = {dialogRect.x + 60, dialogRect.y + 80, dialogRect.width - 120, 360};
        DrawRectangleRec(cardRect, WHITE);
        DrawRectangleLinesEx(cardRect, 4.0F, GuiWindowInternal::kAccent);
        DrawRectangleRec({cardRect.x, cardRect.y, cardRect.width, 80}, GuiWindowInternal::kAccent);
        
        GuiWindowInternal::drawTextCentered(font, propName, {cardRect.x, cardRect.y, cardRect.width, 80}, 24.0F, 1.0F, WHITE);

        float tableY = cardRect.y + 100;
        auto drawRow = [&](std::string label, std::string value, float y) {
            DrawTextEx(font, label.c_str(), {cardRect.x + 20, y}, 14, 1, GRAY);
            float vw = MeasureTextEx(font, value.c_str(), 16, 1).x;
            DrawTextEx(font, value.c_str(), {cardRect.x + cardRect.width - vw - 20, y}, 16, 1, BLACK);
            DrawLineEx({cardRect.x + 20, y + 25}, {cardRect.x + cardRect.width - 20, y + 25}, 0.5F, LIGHTGRAY);
        };
        
        drawRow(detailRentLabel, detailValueStr, tableY);
        drawRow("MORTGAGE VALUE", mortgageStr, tableY + 40);
        drawRow("TAX RATE (EST)", "10%", tableY + 80);

        Rectangle priceBox = {cardRect.x + 30, cardRect.y + 240, cardRect.width - 60, 80};
        DrawRectangleLinesEx(priceBox, 1.0F, GuiWindowInternal::kGold);
        GuiWindowInternal::drawTextCentered(font, "PURCHASE PRICE", {priceBox.x, priceBox.y + 10, priceBox.width, 20}, 12.0F, 1.0F, GuiWindowInternal::kGold);
        GuiWindowInternal::drawTextCentered(font, buyPriceStr, {priceBox.x, priceBox.y + 30, priceBox.width, 40}, 28.0F, 1.0F, GuiWindowInternal::kAccent);

        GuiWindowInternal::drawTextCentered(font, "Apakah kamu ingin membeli properti ini?", Rectangle{dialogRect.x, dialogRect.y + 460, dialogRect.width, 30}, 20.0F, 1.0F, GuiWindowInternal::kInk);
        GuiWindowInternal::drawTextCentered(font, "(y/n) :", Rectangle{dialogRect.x, dialogRect.y + 490, dialogRect.width, 20}, 16.0F, 1.0F, GRAY);

        Rectangle yaRect = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 80, 140, 50};
        Rectangle tidakRect = {dialogRect.x + dialogRect.width/2 - 150, dialogRect.y + dialogRect.height - 80, 140, 50};
        DrawRectangleRec(yaRect, GuiWindowInternal::kAccent);
        GuiWindowInternal::drawTextCentered(font, "YA", yaRect, 20.0F, 1.0F, WHITE);
        DrawRectangleLinesEx(tidakRect, 2.0F, GuiWindowInternal::kAccent);
        GuiWindowInternal::drawTextCentered(font, "TIDAK", tidakRect, 20.0F, 1.0F, GuiWindowInternal::kAccent);

    } else {
        // --- GENERIC FALLBACK ---
        GuiWindowInternal::drawTextCentered(font, current.title, Rectangle{dialogRect.x, dialogRect.y + 10, dialogRect.width, 42}, 24.0F, 1.0F, GuiWindowInternal::kAccentDark);
        GuiWindowInternal::drawWrappedText(font, current.prompt, Rectangle{dialogRect.x + 18, dialogRect.y + 60, dialogRect.width - 36, 140}, 18.0F, 1.0F, GuiWindowInternal::kInk, 8);

        if (!isErrorModal && (!current.yesNo || !current.backendOwned)) {
            Rectangle inputRect{dialogRect.x + 18.0F, dialogRect.y + 204.0F, dialogRect.width - 36.0F, 34.0F};
            DrawRectangleRec(inputRect, WHITE);
            DrawRectangleLinesEx(inputRect, 1.0F, GuiWindowInternal::kPanelBorder);
            DrawTextEx(font, current.inputText.empty() ? " " : current.inputText.c_str(), {inputRect.x + 8, inputRect.y + 8}, 18, 1, GuiWindowInternal::kInk);
        }

        Rectangle okRect{dialogRect.x + dialogRect.width - 210.0F, dialogRect.y + dialogRect.height - 58.0F, 90.0F, 34.0F};
        Rectangle cancelRect{dialogRect.x + dialogRect.width - 110.0F, dialogRect.y + dialogRect.height - 58.0F, 90.0F, 34.0F};
        GuiWindowInternal::drawButton(font, okRect, current.yesNo && current.backendOwned ? "YA" : "OK", true, true);
        GuiWindowInternal::drawButton(font, cancelRect, current.yesNo && current.backendOwned ? "TIDAK" : "BATAL", true, false);
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

    const Color pieceColor = GuiWindowInternal::playerPieceColor(resolvedPlayerIndex);
    const unsigned char panelAlpha =
        static_cast<unsigned char>(GuiWindowInternal::clampFloat(238.0F * alphaScale, 0.0F, 255.0F));
    const unsigned char textAlpha =
        static_cast<unsigned char>(GuiWindowInternal::clampFloat(255.0F * alphaScale, 0.0F, 255.0F));
    const unsigned char shadowAlpha =
        static_cast<unsigned char>(GuiWindowInternal::clampFloat(85.0F * alphaScale, 0.0F, 255.0F));

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
                           static_cast<unsigned char>(GuiWindowInternal::clampFloat(55.0F * alphaScale, 0.0F,
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

    GuiWindowInternal::drawTextCentered(font, "SESI TRANSMISI",
                     Rectangle{cardRect.x + 20.0F, cardRect.y + 66.0F,
                               cardRect.width - 40.0F, 24.0F},
                     30.0F * 0.62F, 3.0F, gold);
    GuiWindowInternal::drawTextCentered(font, "GILIRAN BERGANTI",
                     Rectangle{cardRect.x + 20.0F, cardRect.y + 96.0F,
                               cardRect.width - 40.0F, 54.0F},
                     56.0F, 1.0F, ink);
    GuiWindowInternal::drawTextCentered(font, "ESTATE HOLDER",
                     Rectangle{cardRect.x + 20.0F, cardRect.y + 148.0F,
                               cardRect.width - 40.0F, 24.0F},
                     18.0F, 1.2F, gold);

    GuiWindowInternal::drawTextCentered(font, std::to_string(turnPopupPlayerIndex + 1),
                     Rectangle{cardRect.x + 20.0F, cardRect.y + 168.0F,
                               cardRect.width - 40.0F, 104.0F},
                     120.0F, 1.0F, gold);

    DrawLineEx(Vector2{cardRect.x + 16.0F, cardRect.y + 212.0F},
               Vector2{cardRect.x + 235.0F, cardRect.y + 212.0F}, 1.0F,
               Color{gold.r, gold.g, gold.b,
                     static_cast<unsigned char>(GuiWindowInternal::clampFloat(110.0F * alphaScale, 0.0F,
                                                           255.0F))});
    DrawLineEx(Vector2{cardRect.x + cardRect.width - 16.0F, cardRect.y + 212.0F},
               Vector2{cardRect.x + cardRect.width - 235.0F, cardRect.y + 212.0F},
               1.0F,
               Color{gold.r, gold.g, gold.b,
                     static_cast<unsigned char>(GuiWindowInternal::clampFloat(110.0F * alphaScale, 0.0F,
                                                           255.0F))});

    GuiWindowInternal::drawTextCentered(font, "\"Saatnya mengatur kembali aset Anda.\"",
                     Rectangle{cardRect.x + 24.0F, cardRect.y + 280.0F,
                               cardRect.width - 48.0F, 30.0F},
                     40.0F * 0.58F, 1.0F,
                     Color{236, 214, 186, static_cast<unsigned char>(textAlpha * 0.92F)});

    const Rectangle actionRect{cardRect.x + cardRect.width / 2.0F - 124.0F,
                               cardRect.y + cardRect.height - 22.0F, 248.0F, 20.0F};
    DrawRectangleRec(actionRect, Color{gold.r, gold.g, gold.b,
                                       static_cast<unsigned char>(GuiWindowInternal::clampFloat(
                                           215.0F * alphaScale, 0.0F, 255.0F))});
    DrawRectangleLinesEx(actionRect, 2.0F, Color{112, 80, 18, textAlpha});
    GuiWindowInternal::drawTextCentered(font, "LANJUTKAN STRATEGI", actionRect, 24.0F * 0.72F, 1.0F,
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

    const Color pieceColor = GuiWindowInternal::playerPieceColor(winnerIndex);
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
    GuiWindowInternal::drawRectangleRoundedLinesCompat(cardRect, 0.02F, 6, 2.0F, border);

    GuiWindowInternal::drawTextCentered(font, "PERMAINAN SELESAI",
                     Rectangle{cardRect.x + 20.0F, cardRect.y + 26.0F,
                               cardRect.width - 40.0F, 28.0F},
                     28.0F, 1.0F, heading);
    GuiWindowInternal::drawTextCentered(font, "THE BOARD IS RESOLVED",
                     Rectangle{cardRect.x + 20.0F, cardRect.y + 56.0F,
                               cardRect.width - 40.0F, 20.0F},
                     19.0F, 1.0F, muted);
    DrawLineEx(Vector2{cardRect.x + 60.0F, cardRect.y + 76.0F},
               Vector2{cardRect.x + cardRect.width - 60.0F, cardRect.y + 76.0F},
               1.0F, border);

    if (!currentSnapshot.gameOverReason.empty()) {
        GuiWindowInternal::drawTextCentered(
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
    GuiWindowInternal::drawRectangleRoundedLinesCompat(badgeRect, 0.16F, 10, 2.0F,
                                    Color{118, 99, 22, 255});

    std::string winnerNumbers;
    for (std::size_t i = 0; i < currentSnapshot.winnerNames.size(); ++i) {
        if (i > 0) winnerNumbers += ", ";
        int idx = -1;
        for (std::size_t j = 0; j < currentSnapshot.players.size(); ++j) {
            if (currentSnapshot.players[j].name == currentSnapshot.winnerNames[i]) {
                idx = static_cast<int>(j);
                break;
            }
        }
        winnerNumbers += std::to_string(idx + 1);
    }
    GuiWindowInternal::drawTextCentered(font, winnerNumbers, badgeRect, 48.0F, 1.0F,
                     Color{58, 48, 18, 255});

    GuiWindowInternal::drawTextCentered(font, "Victory Claimed",
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
    GuiWindowInternal::drawTextCentered(font,
                     GuiWindowInternal::truncateText(font, winnerLine, 17.0F, 1.0F,
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
    GuiWindowInternal::drawTextCentered(font, "CASH",
                     Rectangle{innerRect.x, statY, colW, 18.0F}, 16.0F, 1.0F,
                     statLabel);
    GuiWindowInternal::drawTextCentered(font, "M" + std::to_string(currentSnapshot.winnerCash),
                     Rectangle{innerRect.x, statY + 20.0F, colW, 28.0F}, 30.0F, 1.0F,
                     statValue);

    GuiWindowInternal::drawTextCentered(font, "PROPERTI",
                     Rectangle{innerRect.x + colW, statY, colW, 18.0F}, 16.0F, 1.0F,
                     statLabel);
    GuiWindowInternal::drawTextCentered(font, std::to_string(currentSnapshot.winnerPropertyCount),
                     Rectangle{innerRect.x + colW, statY + 20.0F, colW, 28.0F},
                     30.0F, 1.0F, statValue);

    GuiWindowInternal::drawTextCentered(font, "KARTU",
                     Rectangle{innerRect.x + colW * 2.0F, statY, colW, 18.0F}, 16.0F,
                     1.0F, statLabel);
    GuiWindowInternal::drawTextCentered(font, std::to_string(currentSnapshot.winnerCardCount),
                     Rectangle{innerRect.x + colW * 2.0F, statY + 20.0F, colW, 28.0F},
                     30.0F, 1.0F, statValue);

    const Rectangle exitRect = gameOverPopupExitButtonRect();
    const Rectangle newGameRect = gameOverPopupNewGameButtonRect();
    const Rectangle closeRect = gameOverPopupCloseButtonRect();

    DrawRectangleRec(exitRect, Color{214, 214, 214, 255});
    DrawRectangleLinesEx(exitRect, 1.0F, Color{196, 196, 196, 255});
    GuiWindowInternal::drawTextCentered(font, "KELUAR", exitRect, 22.0F, 1.0F, Color{88, 82, 74, 255});

    DrawRectangleRec(newGameRect, Color{98, 8, 8, 255});
    DrawRectangleLinesEx(newGameRect, 1.0F, Color{120, 35, 35, 255});
    GuiWindowInternal::drawTextCentered(font, "MAIN LAGI", newGameRect, 22.0F, 1.0F,
                     Color{245, 239, 227, 255});

    const Color closeFill = Color{122, 102, 6, 255};
    const Color closeText = GuiWindowInternal::contrastingTextColor(closeFill);
    DrawRectangleRec(closeRect, closeFill);
    DrawRectangleLinesEx(closeRect, 1.0F, Color{147, 127, 23, 255});
    GuiWindowInternal::drawTextCentered(font, "TUTUP", closeRect, 24.0F, 1.0F, closeText);
}
