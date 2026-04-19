#ifndef GUI_WINDOW_HPP
#define GUI_WINDOW_HPP

#include <array>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <raylib.h>

#include "../app/GameSession.hpp"
#include "StdStreamBridge.hpp"

class GuiWindow {
public:
    GuiWindow();
    ~GuiWindow();

    int run();

private:
    enum class LocalDialogType {
        None,
        ManualCommand,
        LogCount,
        SetDice,
        SaveFile,
    };

    struct ModalState {
        bool active = false;
        bool backendOwned = false;
        bool yesNo = false;
        bool backendResolved = false;
        LocalDialogType localType = LocalDialogType::None;
        InputPromptRequest request;
        InputPromptResponse response;
        std::string title;
        std::string prompt;
        std::string inputText;
        std::string errorText;
    };

    struct Layout {
        Rectangle headerRect{};
        Rectangle leftPanelRect{};
        Rectangle rightPanelRect{};
        Rectangle boardRect{};
        Rectangle commandRect{};
        Rectangle rosterRect{};
        Rectangle logRect{};
        Rectangle statusRect{};
        Rectangle rollButtonRect{};
        Rectangle manualButtonRect{};
        Rectangle scrollLeftRect{};
        Rectangle scrollRightRect{};
        std::array<Rectangle, 6> quickButtonRects{};
    };

    QueueInputBuffer inputBuffer;
    CallbackOutputBuffer outputBuffer;
    std::unique_ptr<StdStreamRedirector> streamRedirector;
    GameSession backendSession;
    std::thread sessionThread;

    mutable std::mutex snapshotMutex;
    GameSnapshot snapshot;
    bool hasSnapshot;

    mutable std::mutex outputMutex;
    std::string outputText;

    mutable std::mutex modalMutex;
    std::condition_variable modalCondition;
    ModalState modal;

    bool shuttingDown;
    int inspectedPlayerIndex;
    int commandScrollColumn;
    int commandScrollMaxColumn;
    int logScrollLine;
    bool logAutoScroll;
    bool logScrollbarDragging;
    float logScrollbarGrabOffset;
    std::array<int, 6> visibleCommandIndices;
    std::array<std::string, 6> quickButtonLabels;
    std::array<bool, 6> quickButtonEnabled;
    bool manualEnabled;
    Font georgiaFont;
    Vector2 modalPosition;
    Vector2 modalDragOffset;
    bool modalDragging;
    bool modalPositionInitialized;

    void startSession();
    void stopSession();
    void submitInputLine(const std::string& line);
    void appendOutput(const std::string& text);
    void applySnapshot(const GameSnapshot& nextSnapshot);
    void updateQuickButtons(const GameSnapshot& currentSnapshot);
    void executeStartedCommand(int specIndex);
    void openLocalDialog(LocalDialogType type, const std::string& title,
                         const std::string& prompt,
                         const std::string& initialText = "");
    void confirmLocalDialog();
    void cancelLocalDialog();
    InputPromptResponse requestBackendPrompt(const InputPromptRequest& request);
    std::string currentStatusText(const GameSnapshot& currentSnapshot) const;
    int effectiveInspectedPlayerIndex(const GameSnapshot& currentSnapshot) const;
    Rectangle modalDialogRect() const;

    Layout computeLayout(int screenWidth, int screenHeight,
                         const GameSnapshot& currentSnapshot) const;
    void updateFrame(const Layout& layout, const GameSnapshot& currentSnapshot);
    void updateModalInput();
    void drawFrame(const Layout& layout, const GameSnapshot& currentSnapshot);
    void drawHeader(const Layout& layout, const GameSnapshot& currentSnapshot);
    void drawPanel(const Rectangle& rect, const std::string& title,
                   const std::string& subtitle) const;
    void drawStatusPanel(const Layout& layout,
                         const GameSnapshot& currentSnapshot) const;
    void drawRoster(const Layout& layout, const GameSnapshot& currentSnapshot) const;
    void drawBoard(const Layout& layout, const GameSnapshot& currentSnapshot) const;
    void drawActionBar(const Layout& layout, const GameSnapshot& currentSnapshot) const;
    void drawLogPanel(const Layout& layout);
    void drawModal() const;
};

#endif
