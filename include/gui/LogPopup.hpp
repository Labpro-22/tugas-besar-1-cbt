#ifndef GUI_LOG_POPUP_HPP
#define GUI_LOG_POPUP_HPP

#include "../app/GameSession.hpp"
#include <raylib.h>
#include <string>
#include <vector>

class LogPopup {
public:
  class LogEntry {
  public:
    int turn;
    std::string user;
    std::string action;
    std::string detail;

    LogEntry(int turn, const std::string &user, const std::string &action,
             const std::string &detail);
  };

  LogPopup();
  ~LogPopup();

  void open();
  void close();
  bool isOpen() const;

  void addTurnLog(int turn, const std::string &user, const std::string &action,
                  const std::string &detail);
  void addSystemLog(const std::string &detail);
  void loadFromSnapshot(const GameSnapshot &snapshot, int limit = 0);
  void drawOverlay();

private:
  std::vector<LogEntry> logs;
  bool showPopup;
  float scrollOffset;
  float horizontalScrollOffset;
  bool scrollbarDragging;
  bool scrollbarDraggingH;
  float scrollbarGrabOffset;
  float scrollbarGrabOffsetH;

  static const Color MAROON_COLOR;
  static const Color BEIGE_COLOR;
  static const Color DARK_COLOR;
  static const Color SAND_COLOR;

  bool drawButton(Rectangle rect, const char *text, Color bg, Color fg,
                  Color border);
};

#endif
