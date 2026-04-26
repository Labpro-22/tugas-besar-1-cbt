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
        // Player Count
        GuiWindowInternal::drawTextCentered(font, "Pilih Opsi", Rectangle{dialogRect.x, dialogRect.y + 30, dialogRect.width, 60}, 48.0F, 1.5F, GuiWindowInternal::kAccent);
        GuiWindowInternal::drawWrappedText(font, "Tentukan jumlah orang yang akan berpartisipasi dalam permainan ini.", 
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
        // Username
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
        // Purchase Property
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

    } else if (current.active && current.prompt.find("Tangan penuh!") != std::string::npos) {
        // Discard Card
        float sideWidth = 340.0F;
        Rectangle leftSide = {dialogRect.x, dialogRect.y, sideWidth, dialogRect.height};
        DrawRectangleRec(leftSide, Color{70, 0, 0, 255}); // Rich Deep Maroon
        
        GuiWindowInternal::drawTextCentered(font, "ASSET CAPACITY", Rectangle{leftSide.x + 20, leftSide.y + 40, leftSide.width - 40, 30}, 16.0F, 2.0F, Color{220, 180, 180, 255});
        GuiWindowInternal::drawTextCentered(font, "Kartu penuh!", Rectangle{leftSide.x + 20, leftSide.y + 70, leftSide.width - 40, 50}, 32.0F, 1.2F, WHITE);
        
        GuiWindowInternal::drawWrappedText(font, "Sesuai regulasi Nimonspoli, aset operasional Anda terbatas pada 3 slot aktif. Selesaikan transaksi ini dengan melepas satu kartu.", 
                        Rectangle{leftSide.x + 40, leftSide.y + 160, leftSide.width - 80, 160}, 18.0F, 1.2F, {230, 210, 210, 255}, 6);

        Rectangle infoBox = {leftSide.x + 40, leftSide.y + 360, leftSide.width - 80, 100};
        DrawRectangleLinesEx(infoBox, 1.0F, {140, 60, 60, 255});
        DrawCircle(infoBox.x + 25, infoBox.y + 50, 12, GuiWindowInternal::kGold);
        GuiWindowInternal::drawTextCentered(font, "i", {infoBox.x + 13, infoBox.y + 38, 24, 24}, 14.0F, 1.0F, BLACK);
        GuiWindowInternal::drawWrappedText(font, "Kartu yang dibuang akan dikembalikan ke tumpukan Archive.", {infoBox.x + 50, infoBox.y + 25, infoBox.width - 65, 60}, 14.0F, 1.0F, WHITE, 3);

        DrawTextEx(font, "Kapasitas Saat Ini", {leftSide.x + 40, leftSide.y + dialogRect.height - 85}, 14, 1, {180, 140, 140, 255});
        DrawTextEx(font, "4 / 3 KARTU", {leftSide.x + 40, leftSide.y + dialogRect.height - 60}, 28, 1, GuiWindowInternal::kGold);

        // Right side (Cards)
        float rightX = leftSide.x + leftSide.width;
        float rightWidth = dialogRect.width - sideWidth;
        
        GuiWindowInternal::drawTextCentered(font, "Pilih Kartu untuk Dibuang", {rightX, dialogRect.y + 45, rightWidth, 40}, 28.0F, 1.2F, {60, 20, 20, 255});
        DrawRectangle(rightX + (rightWidth - 120)/2.0F, dialogRect.y + 90, 120, 4, GuiWindowInternal::kGold);

        // Parse cards from prompt
        std::vector<std::string> cards;
        std::stringstream ss(current.prompt);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.find(". ") != std::string::npos) {
                cards.push_back(line.substr(line.find(". ") + 2));
            }
        }

        float cardW = 145.0F;
        float cardH = 220.0F;
        float spacing = 15.0F;
        float totalW = cards.size() * cardW + (cards.size() - 1) * spacing;
        float startX = rightX + (rightWidth - totalW) / 2.0F;
        float startY = dialogRect.y + 140.0F;

        for (size_t i = 0; i < cards.size(); ++i) {
            Rectangle cr = {startX + i * (cardW + spacing), startY, cardW, cardH};
            bool selected = (current.inputText == std::to_string(i + 1));
            
            DrawRectangleRec(cr, WHITE);
            DrawRectangleLinesEx(cr, selected ? 3.0F : 1.0F, selected ? GuiWindowInternal::kGold : LIGHTGRAY);
            
            Color topBarColor = (cards[i].find("Move") != std::string::npos) ? Color{80, 180, 120, 255} : 
                               (cards[i].find("Discount") != std::string::npos) ? Color{210, 180, 60, 255} : Color{100, 150, 220, 255};
            DrawRectangle(cr.x, cr.y, cr.width, 8, topBarColor);

            GuiWindowInternal::drawTextCentered(font, cards[i], {cr.x + 10, cr.y + 90, cr.width - 20, 40}, 16.0F, 1.0F, BLACK);
            GuiWindowInternal::drawTextCentered(font, "STRATEGI", {cr.x, cr.y + 180, cr.width, 20}, 11.0F, 2.0F, GRAY);
            
            if (selected) {
                DrawRectangleRec(cr, {topBarColor.r, topBarColor.g, topBarColor.b, 15});
            }
        }

        Rectangle btnConfirm = {rightX + (rightWidth - 220)/2.0F, dialogRect.y + dialogRect.height - 100, 220, 55};
        DrawRectangleRec(btnConfirm, Color{100, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "Konfirmasi Pilihan", btnConfirm, 18.0F, 1.0F, WHITE);

    } else if (current.active && current.prompt.find("Pilih kartu yang ingin digunakan") != std::string::npos) {
        // Use Card
        Rectangle headerRect = {dialogRect.x, dialogRect.y, dialogRect.width, 110};
        DrawRectangleRec(headerRect, Color{110, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "Pilih kartu yang ingin digunakan", {headerRect.x + 40, headerRect.y + 25, dialogRect.width - 80, 40}, 30.0F, 1.2F, WHITE);
        DrawTextEx(font, "ACTION REQUIRED: STRATEGY PHASE", {headerRect.x + (dialogRect.width - 260)/2.0F, headerRect.y + 70}, 14, 2, {200, 150, 150, 255});

        std::vector<std::string> cards;
        std::stringstream ss(current.prompt);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.find(". ") != std::string::npos) {
                cards.push_back(line.substr(line.find(". ") + 2));
            }
        }

        float cardY = headerRect.y + 130.0F;
        for (size_t i = 0; i < cards.size(); ++i) {
            Rectangle itemRect = {dialogRect.x + 40, cardY, dialogRect.width - 80, 80};
            bool selected = (current.inputText == std::to_string(i + 1));
            
            DrawRectangleRec(itemRect, selected ? Color{245, 240, 235, 255} : WHITE);
            DrawRectangleLinesEx(itemRect, 1.0F, selected ? Color{120, 0, 0, 255} : LIGHTGRAY);
            
            Rectangle iconBox = {itemRect.x + 15, itemRect.y + 10, 60, 60};
            DrawRectangleRec(iconBox, {240, 230, 210, 255});
            DrawRectangle(iconBox.x, iconBox.y, iconBox.width, 6, {130, 110, 30, 255});
            
            // Draw simple icon based on card type
            if (cards[i].find("Move") != std::string::npos) {
                DrawCircle(iconBox.x + 30, iconBox.y + 35, 10, {100, 150, 100, 255});
            } else if (cards[i].find("Discount") != std::string::npos) {
                DrawRectangle(iconBox.x + 20, iconBox.y + 25, 20, 20, {210, 180, 60, 255});
            } else {
                DrawCircle(iconBox.x + 30, iconBox.y + 35, 12, {100, 120, 200, 255});
            }

            DrawTextEx(font, cards[i].c_str(), {itemRect.x + 90, itemRect.y + 20}, 20, 1, BLACK);
            DrawTextEx(font, "Aset pendukung operasional strategis.", {itemRect.x + 90, itemRect.y + 45}, 14, 1, GRAY);
            
            DrawCircle(itemRect.x + itemRect.width - 40, itemRect.y + 40, 12, {230, 230, 230, 255});
            if (selected) DrawCircle(itemRect.x + itemRect.width - 40, itemRect.y + 40, 7, Color{120, 0, 0, 255});
            
            cardY += 95.0F;
        }

        Rectangle confirmBtn = {dialogRect.x + 40, dialogRect.y + dialogRect.height - 85, dialogRect.width - 240, 55};
        Rectangle cancelBtn = {dialogRect.x + dialogRect.width - 180, dialogRect.y + dialogRect.height - 85, 140, 55};
        DrawRectangleRec(confirmBtn, Color{110, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "Konfirmasi Pilihan", confirmBtn, 17.0F, 1.0F, WHITE);
        DrawRectangleLinesEx(cancelBtn, 2.0F, Color{110, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "CANCEL", cancelBtn, 17.0F, 1.0F, Color{110, 0, 0, 255});

    } else if (current.active && current.prompt.find("DemolitionCard") != std::string::npos) {
        // Demolition
        Rectangle headerRect = {dialogRect.x, dialogRect.y, dialogRect.width, 80};
        DrawRectangleRec(headerRect, {60, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "DEMOLITION", {headerRect.x + 20, headerRect.y + 25, 300, 30}, 24.0F, 2.0F, {220, 180, 70, 255});
        DrawRectangle(headerRect.x, headerRect.y + 76, headerRect.width, 4, GuiWindowInternal::kGold);

        float sideW = 160.0F;
        Rectangle sideBar = {dialogRect.x, dialogRect.y + 80, sideW, dialogRect.height - 80};
        DrawRectangleRec(sideBar, {30, 10, 10, 255});
        
        GuiWindowInternal::drawTextCentered(font, "WARNING", {sideBar.x, sideBar.y + 40, sideBar.width, 30}, 13.0F, 3.0F, {180, 40, 40, 255});
        GuiWindowInternal::drawWrappedText(font, "Aset lawan yang dipilih akan dihapus secara permanen dari portofolio mereka dan dikembalikan ke otoritas Bank Nimonspoli.", 
                        {sideBar.x + 15, sideBar.y + 85, sideW - 30, 400}, 14.0F, 1.1F, {180, 160, 160, 255}, 10);

        float listX = dialogRect.x + sideW + 20;
        float listY = dialogRect.y + 110;
        float listW = dialogRect.width - sideW - 30;
        float listH = dialogRect.height - 230;

        BeginScissorMode((int)listX - 10, (int)listY - 10, (int)listW + 20, (int)listH + 20);

        std::vector<std::string> targets;
        std::stringstream ss(current.prompt);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.find(". ") != std::string::npos && line.find(" - ") != std::string::npos) targets.push_back(line);
        }

        float itemH = 125.0F;
        float spacing = 15.0F;
        float totalH = targets.size() * (itemH + spacing);
        const_cast<GuiWindow*>(this)->modalScrollMax = std::max(0.0F, totalH - listH);

        for (size_t i = 0; i < targets.size(); ++i) {
            float y = listY + i * (itemH + spacing) - modalScrollOffset;
            if (y + itemH < listY || y > listY + listH) continue;

            Rectangle itemR = {listX, y, listW, itemH};
            bool isSelected = (current.inputText == std::to_string(i + 1));
            
            DrawRectangleRec(itemR, isSelected ? Color{100, 30, 30, 255} : WHITE);
            DrawRectangleLinesEx(itemR, 1.0F, isSelected ? GuiWindowInternal::kGold : LIGHTGRAY);
            
            // Parse name and info
            size_t dotPos = targets[i].find(". ");
            std::string fullLabel = targets[i].substr(dotPos + 2);
            size_t dashPos = fullLabel.find(" - ");
            std::string code = (dashPos != std::string::npos) ? fullLabel.substr(0, dashPos) : fullLabel;
            std::string name = (dashPos != std::string::npos) ? fullLabel.substr(dashPos + 3) : "";
            
            DrawTextEx(font, code.c_str(), {itemR.x + 20, itemR.y + 32}, 24, 1, isSelected ? WHITE : BLACK);
            if (!name.empty()) {
                std::string subText = "ASET - " + name;
                DrawTextEx(font, subText.c_str(), {itemR.x + 20, itemR.y + 68}, 14, 1, isSelected ? Color{220, 200, 200, 255} : Color{140, 80, 80, 255});
            }
            
            if (isSelected) {
                DrawCircle(itemR.x + itemR.width - 40, itemR.y + itemH/2, 12, GuiWindowInternal::kGold);
                DrawCircle(itemR.x + itemR.width - 40, itemR.y + itemH/2, 5, {80, 0, 0, 255});
            }
        }

        EndScissorMode();

        // Scrollbar indicator
        if (modalScrollMax > 0) {
            float sbH = listH * (listH / totalH);
            float sbY = listY + (modalScrollOffset / modalScrollMax) * (listH - sbH);
            DrawRectangle(dialogRect.x + dialogRect.width - 25, (int)sbY, 6, (int)sbH, {100, 40, 40, 255});
        }

        Rectangle confirmBtnDemolish = {listX, dialogRect.y + dialogRect.height - 100, listW, 80};
        DrawRectangleRec(confirmBtnDemolish, current.inputText.empty() ? Color{60, 40, 40, 255} : Color{120, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "KONFIRMASI PENGHANCURAN", confirmBtnDemolish, 18.0F, 1.2F, WHITE);

    } else if (current.active && current.prompt.find("Pilih target LassoCard") != std::string::npos) {
        // Lasso Card
        Rectangle headerRect = {dialogRect.x, dialogRect.y, dialogRect.width, 90};
        DrawRectangleRec(headerRect, Color{90, 0, 0, 255});
        
        Rectangle iconBox = {headerRect.x + 25, headerRect.y + 15, 60, 60};
        DrawRectangleRec(iconBox, GuiWindowInternal::kGold);
        GuiWindowInternal::drawTextCentered(font, "L", iconBox, 32.0F, 1.0F, Color{90, 0, 0, 255});
        
        DrawTextEx(font, "PILIH TARGET LASSOCARD", {headerRect.x + 100, headerRect.y + 20}, 24, 1, WHITE);
        DrawTextEx(font, "Strategic Asset Acquisition", {headerRect.x + 100, headerRect.y + 50}, 16, 1, {200, 130, 130, 255});

        Rectangle infoBar = {dialogRect.x + 35, dialogRect.y + 115, dialogRect.width - 70, 70};
        DrawRectangleRec(infoBar, Color{250, 245, 240, 255});
        DrawRectangle(infoBar.x, infoBar.y, 4, infoBar.height, GuiWindowInternal::kGold);
        GuiWindowInternal::drawWrappedText(font, "\"LassoCard hanya dapat digunakan pada lawan yang berada di depanmu.\"", {infoBar.x + 40, infoBar.y + 15, infoBar.width - 80, 50}, 15.0F, 1.2F, GRAY, 2);

        // Parse candidates
        struct Candidate { std::string name; std::string tile; };
        std::vector<Candidate> candidates;
        std::stringstream ss(current.prompt);
        std::string line;
        while (std::getline(ss, line)) {
            if (line.find(". ") != std::string::npos) {
                size_t dotPos = line.find(". ");
                size_t bracketPos = line.find(" (Tile ");
                if (bracketPos != std::string::npos) {
                    Candidate c;
                    c.name = line.substr(dotPos + 2, bracketPos - (dotPos + 2));
                    c.tile = line.substr(bracketPos + 7, line.find(")", bracketPos) - (bracketPos + 7));
                    candidates.push_back(c);
                }
            }
        }

        float gridX = dialogRect.x + 40.0F;
        float gridY = dialogRect.y + 210.0F;
        float itemW = (dialogRect.width - 110.0F) / 2.0F;
        float itemH = 160.0F;

        for (size_t i = 0; i < candidates.size(); ++i) {
            Rectangle itemR = {gridX + (i % 2) * (itemW + 30), gridY + (i / 2) * (itemH + 30), itemW, itemH};
            bool selected = (current.inputText == std::to_string(i + 1));
            
            DrawRectangleRec(itemR, WHITE);
            DrawRectangleLinesEx(itemR, selected ? 2.0F : 1.0F, selected ? GuiWindowInternal::kGold : LIGHTGRAY);
            DrawRectangle(itemR.x, itemR.y, itemR.width, 6, (i % 2 == 0) ? Color{60, 160, 230, 255} : Color{60, 190, 110, 255});
            
            DrawCircle(itemR.x + 45, itemR.y + 50, 28, {230, 230, 230, 255});
            DrawRectangleRec({itemR.x + itemR.width - 85, itemR.y + 20, 65, 24}, {240, 235, 225, 255});
            GuiWindowInternal::drawTextCentered(font, "TILE " + candidates[i].tile, {itemR.x + itemR.width - 85, itemR.y + 20, 65, 24}, 11.0F, 1.0F, GRAY);
            
            DrawTextEx(font, candidates[i].name.c_str(), {itemR.x + 25, itemR.y + 95}, 20, 1, BLACK);
            DrawTextEx(font, "Executive Portfolio", {itemR.x + 25, itemR.y + 120}, 14, 1, GRAY);
            
            if (selected) {
                DrawTextEx(font, "READY TO TARGET", {itemR.x + 25, itemR.y + 140}, 10, 1.5, GuiWindowInternal::kGold);
            }
        }

        Rectangle targetBtn = {dialogRect.x + dialogRect.width - 250, dialogRect.y + dialogRect.height - 85, 210, 55};
        DrawRectangleRec(targetBtn, Color{90, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "CONFIRM TARGET", targetBtn, 16.0F, 1.0F, WHITE);

    } else if (current.active && current.prompt.find("Pilih tile tujuan teleport") != std::string::npos) {
        // Teleport
        Rectangle headerRect = {dialogRect.x, dialogRect.y, dialogRect.width, 80};
        DrawRectangleRec(headerRect, Color{100, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "Pilih Opsi", {headerRect.x + 20, headerRect.y + 20, 200, 40}, 24.0F, 1.0F, WHITE);
        GuiWindowInternal::drawTextCentered(font, "x", {headerRect.x + headerRect.width - 50, headerRect.y + 20, 30, 40}, 24.0F, 1.0F, GuiWindowInternal::kGold);

        DrawTextEx(font, "INSTRUKSI STRATEGIS", {dialogRect.x + (dialogRect.width - 180)/2, dialogRect.y + 110}, 12, 2, GRAY);
        GuiWindowInternal::drawTextCentered(font, "Pilih tile tujuan teleport (0-59)", {dialogRect.x + 20, dialogRect.y + 130, dialogRect.width - 40, 40}, 20.0F, 1.0F, {100, 50, 50, 255});

        Rectangle inputRect = {dialogRect.x + (dialogRect.width - 180)/2, dialogRect.y + 190, 180, 80};
        DrawRectangleLinesEx(inputRect, 1.0F, {230, 180, 180, 255});
        GuiWindowInternal::drawTextCentered(font, current.inputText.empty() ? "00" : current.inputText, inputRect, 52.0F, 1.0F, {220, 200, 200, 255});
        
        // Stepper icons
        DrawTriangle({inputRect.x + inputRect.width - 25, inputRect.y + 30}, {inputRect.x + inputRect.width - 15, inputRect.y + 30}, {inputRect.x + inputRect.width - 20, inputRect.y + 20}, GRAY);
        DrawTriangle({inputRect.x + inputRect.width - 25, inputRect.y + 50}, {inputRect.x + inputRect.width - 20, inputRect.y + 60}, {inputRect.x + inputRect.width - 15, inputRect.y + 50}, GRAY);

        Rectangle coordBox = {dialogRect.x + 40, dialogRect.y + 310, dialogRect.width - 80, 140};
        DrawRectangleRec(coordBox, {245, 240, 235, 255});
        
        Rectangle mapIcon = {coordBox.x + 25, coordBox.y + 40, 40, 60};
        DrawRectangleRec(mapIcon, {230, 215, 180, 255});
        DrawCircle(mapIcon.x + 20, mapIcon.y + 20, 8, {100, 80, 0, 255});
        
        DrawTextEx(font, "TARGET KOORDINAT", {coordBox.x + 90, coordBox.y + 25}, 10, 1, GRAY);
        GuiWindowInternal::drawWrappedText(font, "Tentukan koordinat aset dengan presisi untuk memastikan dominasi portofolio di area tersebut.", 
                        {coordBox.x + 90, coordBox.y + 45, coordBox.width - 110, 80}, 16.0F, 1.2F, DARKGRAY, 3);

        Rectangle okBtn = {dialogRect.x + 40, dialogRect.y + dialogRect.height - 90, (dialogRect.width - 100)/2, 60};
        Rectangle cancelBtn = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 90, (dialogRect.width - 100)/2, 60};
        DrawRectangleRec(okBtn, {80, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "OK", okBtn, 18.0F, 1.0F, WHITE);
        DrawRectangleLinesEx(cancelBtn, 2.0F, {80, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "BATAL", cancelBtn, 18.0F, 1.0F, {80, 0, 0, 255});

    } else if (current.active && current.prompt.find("Aksi lelang") != std::string::npos) {
        // Auction
        Rectangle gavelRect = {dialogRect.x + (dialogRect.width - 40)/2, dialogRect.y + 40, 40, 40};
        DrawRectangle(gavelRect.x + 10, gavelRect.y + 15, 20, 6, {100, 50, 30, 255});
        DrawRectangle(gavelRect.x + 18, gavelRect.y, 4, 30, {100, 50, 30, 255});
        DrawRectangle(gavelRect.x + 5, gavelRect.y + 35, 30, 4, {100, 50, 30, 255});
        
        GuiWindowInternal::drawTextCentered(font, "AKSI LELANG", {dialogRect.x, dialogRect.y + 90, dialogRect.width, 40}, 28.0F, 2.0F, {80, 20, 20, 255});
        DrawRectangle(dialogRect.x + (dialogRect.width - 40)/2, dialogRect.y + 135, 40, 2, GuiWindowInternal::kGold);

        DrawTextEx(font, "Instruksi Strategis", {dialogRect.x + (dialogRect.width - 140)/2, dialogRect.y + 165}, 14, 1, GRAY);
        
        Rectangle promptBox = {dialogRect.x + 40, dialogRect.y + 195, dialogRect.width - 80, 70};
        DrawRectangleRec(promptBox, {250, 240, 240, 255});
        GuiWindowInternal::drawTextCentered(font, "Aksi lelang (PASS / BID <jumlah>)", promptBox, 18.0F, 1.2F, {100, 40, 40, 255});

        DrawTextEx(font, "Masukkan Perintah Anda", {dialogRect.x + 40, dialogRect.y + 295}, 14, 1, DARKGRAY);
        Rectangle inputRect = {dialogRect.x + 40, dialogRect.y + 320, dialogRect.width - 80, 75};
        DrawRectangleRec(inputRect, WHITE);
        DrawRectangleLinesEx(inputRect, 1.0F, LIGHTGRAY);
        DrawTextEx(font, current.inputText.empty() ? "Contoh: BID 500" : current.inputText.c_str(), {inputRect.x + 20, inputRect.y + 25}, 20, 1, current.inputText.empty() ? LIGHTGRAY : BLACK);
        
        // Input decoration
        DrawLine(inputRect.x + inputRect.width - 40, inputRect.y + 30, inputRect.x + inputRect.width - 20, inputRect.y + 30, GRAY);
        DrawLine(inputRect.x + inputRect.width - 40, inputRect.y + 40, inputRect.x + inputRect.width - 25, inputRect.y + 40, GRAY);
        DrawRectangle(inputRect.x + inputRect.width - 20, inputRect.y + 45, 6, 6, GRAY);

        Rectangle cancelBtn = {dialogRect.x + 40, dialogRect.y + dialogRect.height - 90, (dialogRect.width - 100)/2, 70};
        Rectangle okBtn = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 90, (dialogRect.width - 100)/2, 70};
        
        DrawRectangleRec(okBtn, {100, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "\xf0 \x20 OK", okBtn, 18.0F, 1.0F, WHITE);
        DrawRectangleLinesEx(cancelBtn, 2.0F, {100, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "X \x20 BATAL", cancelBtn, 18.0F, 1.0F, {100, 0, 0, 255});

    } else if (current.active && current.title.find("Festival") != std::string::npos) {
        // Festival
        Rectangle headerRect = {dialogRect.x, dialogRect.y, dialogRect.width, 70};
        DrawRectangleRec(headerRect, {80, 0, 0, 255});
        
        Rectangle ticketIcon = {headerRect.x + 25, headerRect.y + 20, 40, 30};
        DrawRectangleRec(ticketIcon, GuiWindowInternal::kGold);
        DrawCircle(ticketIcon.x, ticketIcon.y + 15, 6, {80, 0, 0, 255});
        DrawCircle(ticketIcon.x + 40, ticketIcon.y + 15, 6, {80, 0, 0, 255});
        
        DrawTextEx(font, "Festival", {headerRect.x + 85, headerRect.y + 22}, 28, 1, WHITE);

        // Gold Gradient Background for content
        float contentH = dialogRect.height - headerRect.height;
        for (int y = 0; y < (int)contentH; ++y) {
            float ratio = (float)y / contentH;
            Color grad = { (unsigned char)(180 + 40*ratio), (unsigned char)(160 + 30*ratio), (unsigned char)(60 + 20*ratio), 255 };
            DrawRectangle(dialogRect.x, headerRect.y + headerRect.height + y, dialogRect.width, 1, grad);
        }

        DrawTextEx(font, "Masukkan kode properti", {dialogRect.x + 40, headerRect.y + 100}, 16, 1, {100, 80, 20, 255});
        Rectangle inputRect = {dialogRect.x + 40, headerRect.y + 125, dialogRect.width - 80, 70};
        DrawRectangleRec(inputRect, {255, 255, 255, 120});
        DrawTextEx(font, current.inputText.empty() ? "Contoh: MGL" : current.inputText.c_str(), {inputRect.x + 25, inputRect.y + 22}, 22, 1, current.inputText.empty() ? DARKGRAY : BLACK);
        DrawCircle(inputRect.x + inputRect.width - 35, inputRect.y + 35, 12, LIGHTGRAY); // Search icon

        Rectangle cancelBtn = {dialogRect.x + dialogRect.width - 310, dialogRect.y + dialogRect.height - 85, 140, 60};
        Rectangle okBtn = {dialogRect.x + dialogRect.width - 160, dialogRect.y + dialogRect.height - 85, 120, 60};
        
        DrawRectangleLinesEx(cancelBtn, 2.0F, {60, 20, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "BATAL", cancelBtn, 16.0F, 1.0F, {60, 20, 0, 255});
        DrawRectangleRec(okBtn, {60, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "OK", okBtn, 16.0F, 1.0F, WHITE);

    } else if (current.active && (current.title == "Pilih Opsi" || current.prompt.find("Pilihan (1/") != std::string::npos)) {
        // Input Choice
        Rectangle decoBar = {dialogRect.x, dialogRect.y, dialogRect.width, 10};
        DrawRectangleRec(decoBar, {120, 0, 0, 255});
        DrawRectangle(dialogRect.x, dialogRect.y + 10, dialogRect.width, 2, {200, 100, 100, 255});

        GuiWindowInternal::drawTextCentered(font, "Pilih Opsi", {dialogRect.x + 40, dialogRect.y + 50, 200, 50}, 34.0F, 1.0F, {120, 0, 0, 255});
        DrawRectangle(dialogRect.x + 45, dialogRect.y + 105, 60, 2, {180, 140, 100, 255});
        
        std::string firstLine = current.prompt;
        size_t nl = firstLine.find('\n');
        if (nl != std::string::npos) firstLine = firstLine.substr(0, nl);
        DrawTextEx(font, firstLine.c_str(), {dialogRect.x + 45, dialogRect.y + 130}, 16, 1, GRAY);

        bool isTax = (current.prompt.find("pajak") != std::string::npos || 
                      current.prompt.find("Pajak") != std::string::npos ||
                      current.prompt.find("Pilihan (1/2)") != std::string::npos);
        
        if (!isTax && !currentSnapshot.logs.empty()) {
            for (int i = static_cast<int>(currentSnapshot.logs.size()) - 1; 
                 i >= std::max(0, static_cast<int>(currentSnapshot.logs.size()) - 2); --i) {
                if (currentSnapshot.logs[i].actionType == "PAJAK") {
                    isTax = true;
                    break;
                }
            }
        }
        
        bool isJail = (current.prompt.find("(1/2/3)") != std::string::npos);
        
        std::string topLabel = "JUMLAH ASET / UNIT";
        std::string subLabel = "";
        if (isTax) {
            topLabel = "OPSI PEMBAYARAN PAJAK";
            subLabel = "Metode Pengurangan Saldo Kas";
        } else if (isJail) {
            topLabel = "OPSI KELUAR PENJARA";
            subLabel = "Ketentuan dan Prosedur Keamanan";
        } else {
            subLabel = "Status Aset & Investasi Saat Ini";
        }

        DrawTextEx(font, topLabel.c_str(), {dialogRect.x + 45, dialogRect.y + 185}, 14, 2, {60, 60, 60, 255});
        Rectangle inputRect = {dialogRect.x + 40, dialogRect.y + 210, dialogRect.width - 80, 80};
        DrawRectangleRec(inputRect, WHITE);
        DrawRectangleLinesEx(inputRect, 1.0F, LIGHTGRAY);
        DrawTextEx(font, current.inputText.empty() ? "1" : current.inputText.c_str(), {inputRect.x + 35, inputRect.y + 22}, 32, 1, BLACK);
        DrawTriangle({inputRect.x + inputRect.width - 35, inputRect.y + 35}, {inputRect.x + inputRect.width - 25, inputRect.y + 35}, {inputRect.x + inputRect.width - 30, inputRect.y + 25}, GRAY);
        DrawTriangle({inputRect.x + inputRect.width - 35, inputRect.y + 55}, {inputRect.x + inputRect.width - 30, inputRect.y + 65}, {inputRect.x + inputRect.width - 25, inputRect.y + 55}, GRAY);

        DrawTextEx(font, subLabel.c_str(), {dialogRect.x + 45, dialogRect.y + 310}, 12, 1, {160, 160, 160, 255});

        float infoBoxHeight = isJail ? 200.0F : 120.0F;
        // Label at 310, infoBox at 335
        Rectangle infoBox = {dialogRect.x + 40, dialogRect.y + 335, dialogRect.width - 80, infoBoxHeight};
        DrawRectangleRec(infoBox, {232, 235, 232, 255});
        DrawRectangle(infoBox.x, infoBox.y, 4, infoBox.height, GuiWindowInternal::kGold);
        DrawCircle(infoBox.x + 35, infoBox.y + 45, 14, {120, 100, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "i", {infoBox.x + 21, infoBox.y + 31, 28, 28}, 16.0F, 1.0F, WHITE);
        
        std::string infoText = current.prompt;
        if (isTax) {
            infoText = "1. Membayar pajak flat M150\n"
                       "2. Membayar pajak sebesar 10%% dari kekayaan total";
        } else if (isJail) {
            infoText = "1. Membayar denda ke Bank sebesar M" + std::to_string(currentSnapshot.jailFine) + ".\n"
                       "2. Menggunakan kartu \"Bebas dari Penjara\".\n"
                       "3. Mencoba melempar dadu (Wajib Double). Gagal 3x = Bayar denda.";
        } else if (!isTax && !currentSnapshot.logs.empty()) {
            for (int i = static_cast<int>(currentSnapshot.logs.size()) - 1; 
                 i >= std::max(0, static_cast<int>(currentSnapshot.logs.size()) - 3); --i) {
                if (currentSnapshot.logs[i].actionType == "PAJAK") {
                    infoText = "1. Membayar pajak flat M150\n2. Membayar pajak sebesar 10% dari kekayaan total";
                    break;
                }
            }
        }
        
        const std::vector<std::string> allLines = GuiWindowInternal::wrapText(font, infoText, 16.0F, 1.1F, infoBox.width - 90, 1000);
        const int visibleLinesCount = static_cast<int>(infoBox.height - 30) / 22; 
        
        {
            std::lock_guard<std::mutex> lock(modalMutex);
            modalScrollMax = static_cast<float>(std::max(0, static_cast<int>(allLines.size()) - visibleLinesCount));
        }

        GuiWindowInternal::drawWrappedText(font, infoText, 
            {infoBox.x + 65, infoBox.y + 20, infoBox.width - 90, infoBox.height - 30}, 16.0F, 1.1F, GuiWindowInternal::kInk, visibleLinesCount, static_cast<int>(modalScrollOffset));

        // Draw scrollbar if needed
        if (allLines.size() > static_cast<std::size_t>(visibleLinesCount)) {
            Rectangle scrollTrack = {infoBox.x + infoBox.width - 15, infoBox.y + 20, 10, infoBox.height - 40};
            DrawRectangleRec(scrollTrack, {200, 200, 200, 100});
            Rectangle thumb = GuiWindowInternal::computeScrollbarThumb(scrollTrack, visibleLinesCount, static_cast<int>(allLines.size()), static_cast<int>(modalScrollOffset));
            DrawRectangleRec(thumb, GuiWindowInternal::kGold);
        }

        Rectangle okBtn = {dialogRect.x + 40, dialogRect.y + dialogRect.height - 90, (dialogRect.width - 100)/2, 65};
        Rectangle cancelBtn = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 90, (dialogRect.width - 100)/2, 65};
        DrawRectangleRec(okBtn, {120, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "OK", okBtn, 18.0F, 1.0F, WHITE);
        DrawRectangleLinesEx(cancelBtn, 2.0F, {120, 0, 0, 255});
        GuiWindowInternal::drawTextCentered(font, "BATAL", cancelBtn, 18.0F, 1.0F, {120, 0, 0, 255});

        // Pagination dots
        DrawCircle(dialogRect.x + dialogRect.width/2 - 10, dialogRect.y + dialogRect.height - 20, 4, DARKGRAY);
        DrawCircle(dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 20, 4, LIGHTGRAY);

    } else if (current.active && current.prompt.find("BANGUN (Pilih Grup Warna)") != std::string::npos) {
        // BANGUN (Pilih Grup Warna) - Card Based Layout
        DrawRectangleRec(dialogRect, {245, 243, 235, 255}); // Paper background
        
        // Header
        Rectangle header = {dialogRect.x, dialogRect.y, dialogRect.width, 70};
        DrawRectangleRec(header, {255, 254, 250, 255});
        DrawRectangle(header.x, header.y + header.height - 1, header.width, 1, {220, 215, 205, 255});
        
        Rectangle iconRect = {header.x + 25, header.y + 20, 30, 30};
        DrawRectangleRec(iconRect, {80, 20, 20, 255});
        DrawRectangle(iconRect.x + 5, iconRect.y + 15, 20, 10, WHITE); // Simple building icon
        DrawRectangle(iconRect.x + 10, iconRect.y + 5, 10, 10, WHITE);
        
        DrawTextEx(font, "BANGUN (Pilih Grup Warna)", {header.x + 75, header.y + 25}, 22, 1, {80, 20, 20, 255});
        DrawTextEx(font, "X", {header.x + header.width - 40, header.y + 25}, 20, 1, DARKGRAY);

        // Subheader (Cash)
        Rectangle subheader = {dialogRect.x, dialogRect.y + 70, dialogRect.width, 50};
        DrawRectangleRec(subheader, {120, 10, 10, 255});
        
        std::string cashStr = "M0";
        size_t cashPos = current.prompt.find("Saat Ini: ");
        if (cashPos != std::string::npos) {
            size_t end = current.prompt.find("\n", cashPos);
            cashStr = current.prompt.substr(cashPos + 10, end - (cashPos + 10));
        }
        DrawTextEx(font, "Saldo Kas Estate", {subheader.x + 25, subheader.y + 15}, 16, 1, {200, 100, 100, 255});
        std::string cashLabel = "Uang Kamu Saat Ini: " + cashStr;
        Vector2 cashSize = MeasureTextEx(font, cashLabel.c_str(), 18, 1);
        DrawTextEx(font, cashLabel.c_str(), {subheader.x + subheader.width - cashSize.x - 25, subheader.y + 15}, 18, 1, {255, 150, 150, 255});

        // Content Area - Scrollable
        Rectangle contentArea = {dialogRect.x, dialogRect.y + 120, dialogRect.width, dialogRect.height - 190};
        
        // Gradient Background
        for(int i=0; i<contentArea.height; i++) {
            float t = (float)i/contentArea.height;
            Color grad = {(unsigned char)(180 + t*50), (unsigned char)(180 + t*40), (unsigned char)(100 + t*40), 255};
            DrawRectangle(contentArea.x, contentArea.y + i, contentArea.width, 1, grad);
        }

        // Parsing groups (Simple regex-like manual parsing)
        std::vector<std::string> lines = GuiWindowInternal::splitLines(current.prompt);
        float cardY = contentArea.y + 30 - modalScrollOffset * 30;
        
        for (size_t i = 0; i < lines.size(); ++i) {
            std::string line = lines[i];
            if (line.find("[") != std::string::npos && line.find("]") != std::string::npos) {
                // Group Header
                size_t start = line.find("[");
                size_t end = line.find("]");
                std::string groupName = line.substr(start, end - start + 1);
                Color groupCol = GuiWindowInternal::colorFromKey(groupName.substr(1, 2));
                
                if (cardY + 100 > contentArea.y && cardY < contentArea.y + contentArea.height) {
                    DrawCircle(contentArea.x + 45, cardY + 15, 18, groupCol);
                    DrawRectangleLinesEx({contentArea.x + 30, cardY, 30, 30}, 1.0F, WHITE);
                    DrawTextEx(font, groupName.c_str(), {contentArea.x + 80, cardY + 5}, 20, 1, {60, 60, 60, 255});
                    DrawRectangle(contentArea.x + 190, cardY + 15, contentArea.width - 230, 1, {255, 255, 255, 100});
                }
                cardY += 45;

                // Property Cards in this group
                float startX = contentArea.x + 40;
                while (i + 1 < lines.size() && lines[i + 1].find("   - ") != std::string::npos) {
                    i++;
                    std::string pLine = lines[i];
                    //   - Jakarta (JKT): 1 rumah (Harga: M200)
                    size_t pNameEnd = pLine.find(" (");
                    std::string pName = pLine.substr(5, pNameEnd - 5);
                    size_t pStatusStart = pLine.find(": ");
                    size_t pStatusEnd = pLine.find(" (Harga");
                    std::string pStatus = pLine.substr(pStatusStart + 2, pStatusEnd - (pStatusStart + 2));
                    size_t pCostStart = pLine.find("Harga: ");
                    std::string pCost = pLine.substr(pCostStart + 7, pLine.find(")", pCostStart) - (pCostStart + 7));

                    Rectangle pCard = {startX, cardY, 280, 130};
                    if (pCard.y + pCard.height > contentArea.y && pCard.y < contentArea.y + contentArea.height) {
                        DrawRectangleRec(pCard, WHITE);
                        DrawRectangle(pCard.x, pCard.y, 4, pCard.height, groupCol);
                        DrawRectangleLinesEx(pCard, 1.0F, {220, 220, 220, 255});
                        
                        DrawTextEx(font, "KAPITAL", {pCard.x + 20, pCard.y + 20}, 12, 1, LIGHTGRAY);
                        DrawTextEx(font, pName.c_str(), {pCard.x + 20, pCard.y + 40}, 18, 1, {60, 60, 60, 255});
                        DrawTextEx(font, pStatus.c_str(), {pCard.x + 20, pCard.y + 85}, 16, 1, GRAY);
                        DrawTextEx(font, pCost.c_str(), {pCard.x + pCard.width - 80, pCard.y + 85}, 18, 1, {120, 20, 20, 255});
                        
                        // House icon
                        DrawRectangle(pCard.x + pCard.width - 45, pCard.y + 20, 20, 15, GuiWindowInternal::kGold);
                        DrawTriangle({pCard.x + pCard.width - 47, pCard.y + 20}, {pCard.x + pCard.width - 23, pCard.y + 20}, {pCard.x + pCard.width - 35, pCard.y + 10}, GuiWindowInternal::kGold);
                    }
                    startX += 300;
                    if (startX + 280 > contentArea.x + contentArea.width) {
                        startX = contentArea.x + 40;
                        cardY += 150;
                    }
                }
                if (startX != contentArea.x + 40) cardY += 150;
                cardY += 20;
            }
        }
        modalScrollMax = (cardY - contentArea.y) / 30.0F;

        // Footer Buttons
        Rectangle footer = {dialogRect.x, dialogRect.y + dialogRect.height - 80, dialogRect.width, 80};
        DrawRectangleRec(footer, {255, 254, 250, 255});
        DrawRectangle(footer.x, footer.y, footer.width, 1, {220, 215, 205, 255});
        
        Rectangle batalBtn = {footer.x + footer.width - 320, footer.y + 15, 140, 50};
        Rectangle okBtn = {footer.x + footer.width - 160, footer.y + 15, 130, 50};
        
        DrawRectangleLinesEx(batalBtn, 1.5F, {120, 20, 20, 255});
        GuiWindowInternal::drawTextCentered(font, "BATAL", batalBtn, 18, 1, {120, 20, 20, 255});
        
        DrawRectangleRec(okBtn, {80, 10, 10, 255});
        GuiWindowInternal::drawTextCentered(font, "OK", okBtn, 18, 1, WHITE);
        DrawCircle(okBtn.x + 25, okBtn.y + 25, 6, GuiWindowInternal::kGold);

    } else if (current.active && current.prompt.find("BANGUN (Pilih Petak)") != std::string::npos) {
        // BANGUN (Pilih Petak)
        DrawRectangleRec(dialogRect, WHITE);
        DrawRectangleLinesEx(dialogRect, 2.0F, {80, 20, 20, 255});
        
        // Header
        Rectangle header = {dialogRect.x, dialogRect.y, dialogRect.width, 60};
        DrawRectangleRec(header, {120, 10, 10, 255});
        DrawTextEx(font, "BANGUN (PILIH PETAK)", {header.x + 25, header.y + 18}, 22, 1, WHITE);
        DrawTextEx(font, "X", {header.x + header.width - 35, header.y + 18}, 18, 1, {255, 200, 200, 255});

        // Group Indicator
        Rectangle groupBar = {dialogRect.x, dialogRect.y + 60, dialogRect.width, 45};
        DrawRectangleRec(groupBar, {0, 50, 100, 255}); // Blue header bar
        
        std::string groupName = "Color Group";
        size_t groupStart = current.prompt.find("[");
        size_t groupEnd = current.prompt.find("]");
        if (groupStart != std::string::npos) groupName = current.prompt.substr(groupStart, groupEnd - groupStart + 1);
        DrawRectangle(groupBar.x + 25, groupBar.y + 18, 12, 12, WHITE);
        DrawTextEx(font, ("Color Group " + groupName).c_str(), {groupBar.x + 50, groupBar.y + 12}, 18, 1, WHITE);

        // Instruction
        DrawTextEx(font, "Select a property in the District to commission new\ndevelopments.", {dialogRect.x + 25, dialogRect.y + 125}, 16, 1, GRAY);

        // List Area
        float itemY = dialogRect.y + 190;
        std::vector<std::string> lines = GuiWindowInternal::splitLines(current.prompt);
        int selection = std::atoi(current.inputText.c_str());
        
        int idx = 1;
        for (const auto& line : lines) {
            if (line.find(". ") != std::string::npos) {
                // 1. Jakarta (JKT): 1 rumah (Biaya: M200)
                size_t nameStart = line.find(". ") + 2;
                size_t nameEnd = line.find(" (");
                std::string pName = line.substr(nameStart, nameEnd - nameStart);
                size_t codeStart = line.find("(") + 1;
                std::string pCode = line.substr(codeStart, 3);
                size_t statusStart = line.find("): ") + 3;
                size_t statusEnd = line.find(" (Biaya");
                std::string pStatus = line.substr(statusStart, statusEnd - statusStart);
                size_t costStart = line.find("Biaya: ") + 7;
                std::string pCost = line.substr(costStart, line.find(")", costStart) - costStart);

                Rectangle itemRect = {dialogRect.x + 25, itemY, dialogRect.width - 50, 90};
                bool isSelected = (selection == idx);
                
                DrawRectangleRec(itemRect, isSelected ? Color{250, 240, 240, 255} : Color{255, 254, 252, 255});
                DrawRectangleLinesEx(itemRect, 2.0F, isSelected ? Color{120, 20, 20, 255} : Color{230, 225, 220, 255});
                
                // Icon
                Rectangle pIcon = {itemRect.x + 15, itemRect.y + 15, 60, 60};
                DrawRectangleRec(pIcon, {0, 40, 80, 255});
                DrawTextEx(font, pCode.c_str(), {pIcon.x + 15, pIcon.y + 22}, 16, 1, WHITE);
                
                DrawTextEx(font, pName.c_str(), {itemRect.x + 90, itemRect.y + 18}, 18, 1, {100, 20, 20, 255});
                DrawTextEx(font, (pStatus + " (dapat dibangun)").c_str(), {itemRect.x + 90, itemRect.y + 45}, 14, 1, GRAY);
                
                // Radio button
                DrawCircle(itemRect.x + itemRect.width - 35, itemRect.y + 45, 12, LIGHTGRAY);
                DrawCircle(itemRect.x + itemRect.width - 35, itemRect.y + 45, 10, WHITE);
                if (isSelected) DrawCircle(itemRect.x + itemRect.width - 35, itemRect.y + 45, 6, {120, 20, 20, 255});

                itemY += 105;
                idx++;
            }
        }

        // Investment Cost
        DrawRectangle(dialogRect.x + 25, itemY + 20, dialogRect.width - 50, 1, {230, 225, 220, 255});
        DrawTextEx(font, "INVESTMENT COST", {dialogRect.x + 25, itemY + 45}, 14, 1, LIGHTGRAY);
        DrawTextEx(font, "M200", {dialogRect.x + dialogRect.width - 150, itemY + 42}, 24, 1, {100, 20, 20, 255});
        DrawTextEx(font, "/ unit", {dialogRect.x + dialogRect.width - 75, itemY + 48}, 14, 1, GRAY);

        // Buttons
        Rectangle batalBtn = {dialogRect.x + dialogRect.width - 300, dialogRect.y + dialogRect.height - 80, 120, 50};
        Rectangle okBtn = {dialogRect.x + dialogRect.width - 160, dialogRect.y + dialogRect.height - 80, 130, 50};
        
        DrawRectangleLinesEx(batalBtn, 1.5F, DARKGRAY);
        GuiWindowInternal::drawTextCentered(font, "BATAL", batalBtn, 18, 1, BLACK);
        DrawRectangleRec(okBtn, {60, 10, 10, 255});
        GuiWindowInternal::drawTextCentered(font, "OK", okBtn, 18, 1, WHITE);

    } else if (current.active && current.prompt.find("UPGRADE HOTEL") != std::string::npos) {
        // Upgrade Hotel
        DrawRectangleRec(dialogRect, {252, 249, 240, 255});
        
        Rectangle topBar = {dialogRect.x, dialogRect.y, dialogRect.width, 50};
        DrawRectangleRec(topBar, {100, 10, 10, 255});
        GuiWindowInternal::drawTextCentered(font, "UPGRADE HOTEL", topBar, 22, 1.5, WHITE);

        // Graphic Area
        Rectangle graphicArea = {dialogRect.x + 50, dialogRect.y + 80, dialogRect.width - 100, 180};
        GuiWindowInternal::drawRectangleRoundedLinesCompat(graphicArea, 0.1, 8, 1.0, {210, 200, 180, 255});
        
        // 4 Houses + Hotel Icon
        for (int i = 0; i < 4; ++i) {
            float hx = graphicArea.x + graphicArea.width/2 - 100 + i*45;
            DrawRectangle(hx, graphicArea.y + 40, 30, 20, {120, 20, 20, 255});
            DrawTriangle({hx-2, graphicArea.y + 40}, {hx+32, graphicArea.y + 40}, {hx+15, graphicArea.y + 25}, {120, 20, 20, 255});
        }
        DrawRectangle(graphicArea.x + graphicArea.width/2 + 85, graphicArea.y + 35, 35, 30, {220, 200, 150, 255}); // Hotel
        
        std::string infoText = "Seluruh color group [BIRU TUA] sudah memiliki 4 rumah. Siap di-upgrade ke hotel!";
        size_t groupStart = current.prompt.find("[");
        size_t groupEnd = current.prompt.find("]");
        if (groupStart != std::string::npos) {
            infoText = "Seluruh color group " + current.prompt.substr(groupStart, groupEnd - groupStart + 1) + " sudah memiliki 4 rumah. Siap di-upgrade ke hotel!";
        }
        GuiWindowInternal::drawWrappedText(font, infoText, {graphicArea.x + 30, graphicArea.y + 100, graphicArea.width - 60, 60}, 16, 1, GRAY, 3);

        // Confirmation Box
        Rectangle confirmBox = {dialogRect.x + 80, dialogRect.y + 285, dialogRect.width - 160, 190};
        DrawRectangleLinesEx(confirmBox, 2.0F, {50, 80, 150, 255});
        DrawRectangle(confirmBox.x + confirmBox.width/2 - 115, confirmBox.y - 12, 230, 24, {0, 50, 100, 255});
        GuiWindowInternal::drawTextCentered(font, "PROPERTY DEVELOPMENT", {confirmBox.x, confirmBox.y - 12, confirmBox.width, 24}, 12, 1, WHITE);
        
        std::string pName = "Property";
        size_t upgradePos = current.prompt.find("Upgrade ");
        size_t toPos = current.prompt.find(" ke Hotel");
        if (upgradePos != std::string::npos) pName = current.prompt.substr(upgradePos + 8, toPos - (upgradePos + 8));

        std::string targetLabel = pName;
        for (const auto& tile : currentSnapshot.tiles) {
            if (tile.name == pName && !tile.code.empty()) {
                targetLabel = tile.code;
                break;
            }
        }
        
        GuiWindowInternal::drawTextCentered(font, ("Upgrade " + targetLabel + " ke Hotel?").c_str(), {confirmBox.x, confirmBox.y + 52, confirmBox.width, 40}, 24, 1.2, {60, 60, 60, 255});
        DrawRectangle(confirmBox.x + 90, confirmBox.y + 110, confirmBox.width - 180, 1, LIGHTGRAY);
        
        std::string costStr = "M200";
        size_t costPos = current.prompt.find("Biaya: ");
        if (costPos != std::string::npos) costStr = current.prompt.substr(costPos + 7);
        
        std::string costLabel = "Biaya: " + costStr;
        GuiWindowInternal::drawTextCentered(font, costLabel.c_str(), {confirmBox.x, confirmBox.y + 130, confirmBox.width, 42}, 28, 1, GuiWindowInternal::kGold);

        // Buttons
        Rectangle yaBtn = {dialogRect.x + 60, dialogRect.y + 520, 250, 80};
        Rectangle tidakBtn = {dialogRect.x + dialogRect.width - 310, dialogRect.y + 520, 250, 80};
        
        DrawRectangleRec(yaBtn, {120, 10, 10, 255});
        DrawCircle(yaBtn.x + 80, yaBtn.y + 40, 8, WHITE);
        DrawTextEx(font, "YA", {yaBtn.x + 110, yaBtn.y + 28}, 24, 1.5, WHITE);
        
        DrawRectangleLinesEx(tidakBtn, 2.0F, {120, 10, 10, 255});
        DrawCircle(tidakBtn.x + 60, tidakBtn.y + 40, 8, {120, 10, 10, 255});
        DrawTextEx(font, "TIDAK", {tidakBtn.x + 90, tidakBtn.y + 28}, 24, 1.5, {120, 10, 10, 255});

        // Bottom Bar
        Rectangle bottomBar = {dialogRect.x, dialogRect.y + dialogRect.height - 40, dialogRect.width, 40};
        DrawRectangleRec(bottomBar, {245, 240, 230, 255});
        DrawRectangle(bottomBar.x, bottomBar.y, bottomBar.width, 1, LIGHTGRAY);

        auto formatMoney = [](int amount) {
            std::string s = std::to_string(amount);
            for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3) {
                s.insert(static_cast<std::size_t>(i), ".");
            }
            return std::string("M") + s;
        };

        int activeCash = 0;
        if (currentSnapshot.activePlayerIndex >= 0 &&
            currentSnapshot.activePlayerIndex <
                static_cast<int>(currentSnapshot.players.size())) {
            activeCash = currentSnapshot.players[static_cast<std::size_t>(currentSnapshot.activePlayerIndex)].cash;
        }

        const std::string balanceText = "CURRENT BALANCE: " + formatMoney(activeCash);
        DrawTextEx(font, balanceText.c_str(), {bottomBar.x + 20, bottomBar.y + 12}, 12, 1, GRAY);

    } else if (current.active && (current.title == "Konfirmasi Gadai" || current.prompt.find("Konfirmasi Gadai") != std::string::npos)) {
        // Konfirmasi Gadai
        DrawRectangleRec(dialogRect, {252, 248, 242, 255}); // bg cream
        DrawRectangleLinesEx(dialogRect, 2.0F, {120, 10, 10, 255});

        // Icon Header
        Rectangle iconBox = {dialogRect.x + dialogRect.width/2 - 40, dialogRect.y + 40, 80, 80};
        DrawRectangleRec(iconBox, {120, 10, 10, 255}); 
        
        // Better Bank Icon
        float ix = iconBox.x + 20, iy = iconBox.y + 25;
        DrawRectangle(ix, iy + 25, 40, 5, WHITE); 
        DrawRectangle(ix + 5, iy + 10, 5, 15, WHITE); 
        DrawRectangle(ix + 17, iy + 10, 5, 15, WHITE); 
        DrawRectangle(ix + 30, iy + 10, 5, 15, WHITE); 
        DrawRectangle(ix, iy + 5, 40, 5, WHITE); 
        DrawTriangle({ix - 5, iy + 5}, {ix + 45, iy + 5}, {ix + 20, iy - 10}, WHITE); // Roof

        GuiWindowInternal::drawTextCentered(font, "KONFIRMASI GADAI", {dialogRect.x, dialogRect.y + 140, dialogRect.width, 40}, 32.0F, 1.8F, {120, 10, 10, 255});
        DrawRectangle(dialogRect.x + dialogRect.width/2 - 50, dialogRect.y + 190, 100, 2, {120, 10, 10, 80});

        GuiWindowInternal::drawTextCentered(font, "Apakah Anda yakin ingin menggadaikan aset ini?", {dialogRect.x, dialogRect.y + 220, dialogRect.width, 30}, 18.0F, 1.0F, {100, 100, 100, 255});
        GuiWindowInternal::drawTextCentered(font, "Anda akan menerima:", {dialogRect.x, dialogRect.y + 255, dialogRect.width, 30}, 18.0F, 1.2F, {120, 10, 10, 255});

        // Amount Box
        std::string amountStr = "M 0";
        std::size_t amountPos = current.prompt.find("menerima:\n");
        if (amountPos != std::string::npos) {
            std::size_t start = amountPos + 10;
            std::size_t end = current.prompt.find("\n", start);
            if (end == std::string::npos) amountStr = current.prompt.substr(start);
            else amountStr = current.prompt.substr(start, end - start);
        }
        
        Rectangle amountBox = {dialogRect.x + 80, dialogRect.y + 300, dialogRect.width - 160, 90};
        DrawRectangleRec(amountBox, {245, 242, 235, 255});
        DrawRectangleLinesEx(amountBox, 1.0F, {120, 10, 10, 40});
        GuiWindowInternal::drawTextCentered(font, amountStr, amountBox, 42.0F, 1.0F, {80, 20, 20, 255});

        DrawRectangle(dialogRect.x + 60, dialogRect.y + 430, dialogRect.width - 120, 1, {210, 205, 195, 255});

        // Property Info
        std::string propName = "Properti";
        std::size_t propPos = current.prompt.find("Target Properti: ");
        if (propPos != std::string::npos) {
            propName = current.prompt.substr(propPos + 17);
        }
        Rectangle propIcon = {dialogRect.x + 80, dialogRect.y + 460, 55, 55};
        DrawRectangleRec(propIcon, {140, 20, 20, 255});
        DrawRectangleLinesEx(propIcon, 2.0F, {100, 0, 0, 255});
        
        DrawTextEx(font, "Target Properti", {dialogRect.x + 150, dialogRect.y + 465}, 16.0F, 1.0F, {120, 120, 120, 255});
        DrawTextEx(font, propName.c_str(), {dialogRect.x + 150, dialogRect.y + 485}, 24.0F, 1.2F, {40, 40, 40, 255});

        // Buttons
        Rectangle noBtn = {dialogRect.x + 50, dialogRect.y + dialogRect.height - 100, (dialogRect.width - 120)/2, 65};
        Rectangle yesBtn = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 100, (dialogRect.width - 120)/2, 65};
        
        DrawRectangleLinesEx(noBtn, 2.0F, {120, 10, 10, 255});
        GuiWindowInternal::drawTextCentered(font, "TIDAK", noBtn, 22.0F, 1.8F, {120, 10, 10, 255});
        
        DrawRectangleRec(yesBtn, {120, 10, 10, 255});
        GuiWindowInternal::drawTextCentered(font, "YA", yesBtn, 22.0F, 1.8F, WHITE);

    } else if (current.active && (current.title == "Konfirmasi Tebus" || current.prompt.find("Konfirmasi Tebus") != std::string::npos)) {
        // Konfirmasi Tebus (Polished)
        for (int i = 0; i < (int)dialogRect.height; i++) {
            float t = (float)i / dialogRect.height;
            Color c = {(unsigned char)(70 + t * 50), (unsigned char)(100 + t * 40), (unsigned char)(90 + t * 40), 255};
            DrawRectangle(dialogRect.x, dialogRect.y + i, dialogRect.width, 1, c);
        }
        DrawRectangleLinesEx(dialogRect, 2.0F, {180, 200, 180, 255});

        Rectangle iconBox = {dialogRect.x + dialogRect.width/2 - 40, dialogRect.y + 40, 80, 80};
        DrawRectangleLinesEx(iconBox, 2.0F, WHITE);
        DrawRectangleRounded({iconBox.x + 10, iconBox.y + 10, 50, 50}, 0.2F, 8, {120, 10, 10, 255});
        DrawRectangle(iconBox.x + 25, iconBox.y + 25, 20, 10, {60, 0, 0, 255}); 

        GuiWindowInternal::drawTextCentered(font, "KONFIRMASI TEBUS", {dialogRect.x, dialogRect.y + 140, dialogRect.width, 40}, 32.0F, 1.8F, WHITE);
        DrawRectangle(dialogRect.x + dialogRect.width/2 - 50, dialogRect.y + 190, 100, 2, {255, 255, 255, 150});
        GuiWindowInternal::drawTextCentered(font, "Apakah Anda yakin ingin menebus kembali aset ini?", {dialogRect.x, dialogRect.y + 220, dialogRect.width, 30}, 18.0F, 1.0F, {220, 220, 220, 255});

        std::string amountStr = "M 0";
        std::size_t amountPos = current.prompt.find("BIAYA TEBUS:\n");
        if (amountPos == std::string::npos) amountPos = current.prompt.find("TEBUS:\n");
        if (amountPos != std::string::npos) {
            std::size_t start = current.prompt.find("M", amountPos);
            std::size_t end = current.prompt.find("\n", start);
            if (end == std::string::npos) amountStr = current.prompt.substr(start);
            else amountStr = current.prompt.substr(start, end - start);
        }
        
        Rectangle amountBox = {dialogRect.x + 80, dialogRect.y + 270, dialogRect.width - 160, 100};
        DrawRectangleRec(amountBox, {255, 255, 255, 230});
        DrawTextEx(font, "BIAYA TEBUS", {amountBox.x + amountBox.width/2 - 50, amountBox.y + 15}, 16.0F, 1.0F, DARKGRAY);
        GuiWindowInternal::drawTextCentered(font, amountStr, {amountBox.x, amountBox.y + 40, amountBox.width, 45}, 36.0F, 1.0F, {120, 10, 10, 255});

        DrawRectangle(dialogRect.x + 60, dialogRect.y + 400, dialogRect.width - 120, 1, {255, 255, 255, 100});

        std::string propName = "Properti";
        std::size_t propPos = current.prompt.find("Target Properti: ");
        if (propPos != std::string::npos) propName = current.prompt.substr(propPos + 17);
        
        Rectangle propIcon = {dialogRect.x + 80, dialogRect.y + 430, 55, 55};
        DrawRectangleRec(propIcon, {140, 20, 20, 255});
        DrawRectangleLinesEx(propIcon, 2.0F, {100, 0, 0, 255});
        DrawTextEx(font, "Target Properti", {dialogRect.x + 150, dialogRect.y + 435}, 16.0F, 1.0F, {200, 200, 200, 255});
        DrawTextEx(font, propName.c_str(), {dialogRect.x + 150, dialogRect.y + 455}, 24.0F, 1.2F, WHITE);

        Rectangle noBtn = {dialogRect.x + 50, dialogRect.y + dialogRect.height - 100, (dialogRect.width - 120)/2, 65};
        Rectangle yesBtn = {dialogRect.x + dialogRect.width/2 + 10, dialogRect.y + dialogRect.height - 100, (dialogRect.width - 120)/2, 65};
        DrawRectangleLinesEx(noBtn, 2.0F, WHITE);
        GuiWindowInternal::drawTextCentered(font, "TIDAK", noBtn, 22.0F, 1.8F, WHITE);
        DrawRectangleRec(yesBtn, {120, 10, 10, 255});
        GuiWindowInternal::drawTextCentered(font, "YA", yesBtn, 22.0F, 1.8F, WHITE);

    } else {
        GuiWindowInternal::drawTextCentered(font, current.title, Rectangle{dialogRect.x, dialogRect.y + 10, dialogRect.width, 42}, 24.0F, 1.0F, GuiWindowInternal::kAccentDark);
        GuiWindowInternal::drawWrappedText(font, current.prompt, Rectangle{dialogRect.x + 18, dialogRect.y + 60, dialogRect.width - 36, 140}, 18.0F, 1.0F, GuiWindowInternal::kInk, 8);

        if (!isErrorModal && (!current.yesNo || !current.backendOwned)) {
            Rectangle inputRect{dialogRect.x + 18.0F, dialogRect.y + 204.0F, dialogRect.width - 36.0F, 34.0F};
            DrawRectangleRec(inputRect, WHITE);
            DrawRectangleLinesEx(inputRect, 1.0F, GuiWindowInternal::kPanelBorder);
            DrawTextEx(font, current.inputText.empty() ? " " : current.inputText.c_str(), {inputRect.x + 8, inputRect.y + 8}, 18, 1, GuiWindowInternal::kInk);
        }

        if (isErrorModal) {
            Rectangle errorOkRect{dialogRect.x + (dialogRect.width - 120.0F) / 2.0F, 
                                  dialogRect.y + dialogRect.height - 58.0F, 120.0F, 34.0F};
            GuiWindowInternal::drawButton(font, errorOkRect, "OK", true, true);
        } else {
            Rectangle okRect{dialogRect.x + dialogRect.width - 210.0F, dialogRect.y + dialogRect.height - 58.0F, 90.0F, 34.0F};
            Rectangle cancelRect{dialogRect.x + dialogRect.width - 110.0F, dialogRect.y + dialogRect.height - 58.0F, 90.0F, 34.0F};
            GuiWindowInternal::drawButton(font, okRect, (current.yesNo && current.backendOwned) ? "YA" : "OK", true, true);
            GuiWindowInternal::drawButton(font, cancelRect, (current.yesNo && current.backendOwned) ? "TIDAK" : "BATAL", true, false);
        }
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

    if (!currentSnapshot.winnerNames.empty()) {
        // in header
    }

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
