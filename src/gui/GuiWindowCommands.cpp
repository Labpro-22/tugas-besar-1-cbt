#include "GuiWindowInternal.hpp"

using namespace gui_internal;

void GuiWindow::updateQuickButtons(const GameSnapshot& currentSnapshot) {
    visibleCommandIndices.fill(-1);
    quickButtonLabels.fill("-");
    quickButtonEnabled.fill(false);
    manualEnabled = currentSnapshot.gameStarted;

    if (!currentSnapshot.gameStarted) {
        commandScrollColumn = 0;
        commandScrollMaxColumn = 0;
        if (currentSnapshot.startupMode == "PLAYER_COUNT") {
            quickButtonLabels = {"2 PEMAIN", "3 PEMAIN", "4 PEMAIN", "-", "-", "-"};
            quickButtonEnabled = {true, true, true, false, false, false};
        } else if (currentSnapshot.startupMode == "LOAD_FILE") {
            quickButtonLabels = {"LOAD GAME", "-", "-", "-", "-", "-"};
            quickButtonEnabled = {false, false, false, false, false, false};
        } else if (currentSnapshot.startupMode == "USERNAME") {
            quickButtonLabels = {"INPUT NAMA", "-", "-", "-", "-", "-"};
            quickButtonEnabled = {false, false, false, false, false, false};
        } else {
            quickButtonLabels = {"NEW GAME", "LOAD GAME", "EXIT", "-", "-", "-"};
            quickButtonEnabled = {true, true, true, false, false, false};
        }
        return;
    }

    const int totalCommands = static_cast<int>(kStartedQuickActions.size());
    const int totalColumns = (totalCommands + 1) / 2;
    const int visibleColumns = 3;
    commandScrollMaxColumn = std::max(0, totalColumns - visibleColumns);
    commandScrollColumn = clampInt(commandScrollColumn, 0, commandScrollMaxColumn);

    for (int column = 0; column < visibleColumns; ++column) {
        const int topSlot = column;
        const int bottomSlot = column + 3;
        const int topIndex = (commandScrollColumn + column) * 2;
        const int bottomIndex = topIndex + 1;

        if (topIndex < totalCommands) {
            quickButtonLabels[static_cast<std::size_t>(topSlot)] =
                kStartedQuickActions[static_cast<std::size_t>(topIndex)].label;
            quickButtonEnabled[static_cast<std::size_t>(topSlot)] = true;
            visibleCommandIndices[static_cast<std::size_t>(topSlot)] = topIndex;
        }
        if (bottomIndex < totalCommands) {
            quickButtonLabels[static_cast<std::size_t>(bottomSlot)] =
                kStartedQuickActions[static_cast<std::size_t>(bottomIndex)].label;
            quickButtonEnabled[static_cast<std::size_t>(bottomSlot)] = true;
            visibleCommandIndices[static_cast<std::size_t>(bottomSlot)] = bottomIndex;
        }
    }
}

void GuiWindow::executeStartedCommand(const int specIndex) {
    if (specIndex < 0 || specIndex >= static_cast<int>(kStartedQuickActions.size())) {
        return;
    }

    const StartedQuickAction action =
        kStartedQuickActions[static_cast<std::size_t>(specIndex)].action;
    switch (action) {
        case StartedQuickAction::PrintBoard:
            submitInputLine("CETAK_PAPAN");
            return;
        case StartedQuickAction::PrintProperties:
            submitInputLine("CETAK_PROPERTI");
            return;
        case StartedQuickAction::RollDice:
            submitInputLine("LEMPAR_DADU");
            return;
        case StartedQuickAction::PrintLog:
            openLocalDialog(LocalDialogType::LogCount, "Cetak Log",
                            "Masukkan jumlah entri opsional.\nKosongkan untuk cetak semua log.");
            return;
        case StartedQuickAction::SetDice:
            openLocalDialog(LocalDialogType::SetDice, "Atur Dadu",
                            "Masukkan dua nilai dadu 1-6.\nContoh: 2 5");
            return;
        case StartedQuickAction::PrintDeed:
            submitInputLine("CETAK_AKTA");
            return;
        case StartedQuickAction::Mortgage:
            submitInputLine("GADAI");
            return;
        case StartedQuickAction::Redeem:
            submitInputLine("TEBUS");
            return;
        case StartedQuickAction::Build:
            submitInputLine("BANGUN");
            return;
        case StartedQuickAction::UseAbility:
            submitInputLine("GUNAKAN_KEMAMPUAN");
            return;
        case StartedQuickAction::Save:
            openLocalDialog(LocalDialogType::SaveFile, "Simpan Game",
                            "Masukkan nama file save.\nContoh: sesi1.txt");
            return;
    }
}

void GuiWindow::openLocalDialog(const LocalDialogType type,
                                const std::string& title,
                                const std::string& prompt,
                                const std::string& initialText) {
    std::lock_guard<std::mutex> lock(modalMutex);
    if (modal.active) {
        return;
    }

    modal = ModalState{};
    modal.active = true;
    modal.backendOwned = false;
    modal.localType = type;
    modal.title = title;
    modal.prompt = prompt;
    modal.inputText = initialText;
    modalDragging = false;
    modalPositionInitialized = false;
}

void GuiWindow::confirmLocalDialog() {
    ModalState current;
    {
        std::lock_guard<std::mutex> lock(modalMutex);
        if (!modal.active) {
            return;
        }
        current = modal;
    }

    if (current.backendOwned) {
        std::lock_guard<std::mutex> lock(modalMutex);
        modal.response.accepted = true;
        modal.response.value = current.inputText;
        modal.backendResolved = true;
        modal.active = false;
        modalDragging = false;
        modalCondition.notify_all();
        return;
    }

    const std::string value = trimWhitespace(current.inputText);

    switch (current.localType) {
        case LocalDialogType::ManualCommand:
            if (value.empty()) {
                std::lock_guard<std::mutex> lock(modalMutex);
                modal.errorText = "Perintah tidak boleh kosong.";
                return;
            }
            submitInputLine(value);
            break;
        case LocalDialogType::LogCount:
            if (value.empty()) {
                submitInputLine("CETAK_LOG");
            } else {
                submitInputLine("CETAK_LOG " + value);
            }
            break;
        case LocalDialogType::SetDice: {
            std::istringstream iss(value);
            int d1 = 0;
            int d2 = 0;
            std::string extra;
            if (!(iss >> d1 >> d2) || (iss >> extra) || d1 < 1 || d1 > 6 ||
                d2 < 1 || d2 > 6) {
                std::lock_guard<std::mutex> lock(modalMutex);
                modal.errorText = "Format harus dua angka 1-6. Contoh: 2 5";
                return;
            }
            submitInputLine("ATUR_DADU " + std::to_string(d1) + " " +
                            std::to_string(d2));
            break;
        }
        case LocalDialogType::SaveFile:
            if (value.empty()) {
                std::lock_guard<std::mutex> lock(modalMutex);
                modal.errorText = "Nama file tidak boleh kosong.";
                return;
            }
            submitInputLine("SIMPAN " + value);
            break;
        case LocalDialogType::None:
            break;
    }

    std::lock_guard<std::mutex> lock(modalMutex);
    modalDragging = false;
    modal = ModalState{};
}

void GuiWindow::cancelLocalDialog() {
    std::lock_guard<std::mutex> lock(modalMutex);
    if (!modal.active) {
        return;
    }

    if (modal.backendOwned) {
        modal.response.accepted = false;
        modal.response.value.clear();
        modal.backendResolved = true;
        modal.active = false;
        modalDragging = false;
        modalCondition.notify_all();
        return;
    }

    modalDragging = false;
    modal = ModalState{};
}

InputPromptResponse GuiWindow::requestBackendPrompt(
    const InputPromptRequest& request) {
    std::unique_lock<std::mutex> lock(modalMutex);
    modalCondition.wait(lock, [this]() { return shuttingDown || !modal.active; });
    if (shuttingDown) {
        return {};
    }

    modal = ModalState{};
    modal.active = true;
    modal.backendOwned = true;
    modal.yesNo = request.kind == InputPromptKind::YesNo;
    modal.request = request;
    modal.title = request.title.empty() ? "Input" : request.title;
    modal.prompt = request.prompt;
    modal.inputText = request.initialValue;
    modal.backendResolved = false;
    modalDragging = false;
    modalPositionInitialized = false;
    lock.unlock();

    std::unique_lock<std::mutex> waitLock(modalMutex);
    modalCondition.wait(waitLock,
                        [this]() { return shuttingDown || modal.backendResolved; });
    InputPromptResponse response = modal.response;
    modalDragging = false;
    modal = ModalState{};
    waitLock.unlock();
    modalCondition.notify_all();
    return response;
}

std::string GuiWindow::currentStatusText(
    const GameSnapshot& currentSnapshot) const {
    if (!currentSnapshot.gameStarted) {
        return currentSnapshot.statusText;
    }

    const int inspectedIndex = effectiveInspectedPlayerIndex(currentSnapshot);
    if (inspectedIndex >= 0 &&
        inspectedIndex < static_cast<int>(currentSnapshot.players.size())) {
        return currentSnapshot
            .players[static_cast<std::size_t>(inspectedIndex)]
            .detailText;
    }

    return currentSnapshot.statusText;
}

int GuiWindow::effectiveInspectedPlayerIndex(
    const GameSnapshot& currentSnapshot) const {
    if (!currentSnapshot.gameStarted || currentSnapshot.players.empty()) {
        return -1;
    }

    if (inspectedPlayerIndex >= 0 &&
        inspectedPlayerIndex < static_cast<int>(currentSnapshot.players.size())) {
        return inspectedPlayerIndex;
    }

    if (currentSnapshot.activePlayerIndex >= 0 &&
        currentSnapshot.activePlayerIndex <
            static_cast<int>(currentSnapshot.players.size())) {
        return currentSnapshot.activePlayerIndex;
    }

    return 0;
}

Rectangle GuiWindow::modalDialogRect() const {
    return Rectangle{modalPosition.x, modalPosition.y, 500.0F, 260.0F};
}

