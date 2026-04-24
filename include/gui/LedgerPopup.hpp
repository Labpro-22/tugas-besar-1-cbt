#ifndef GUI_LEDGER_POPUP_HPP
#define GUI_LEDGER_POPUP_HPP

#include "../app/GameSession.hpp"
#include <map>
#include <raylib.h>
#include <string>
#include <vector>

class LedgerPopup {
public:
  LedgerPopup();
  ~LedgerPopup();

  void open();
  void close();
  bool isOpen() const;

  void loadFromSnapshot(const GameSnapshot &snapshot);
  void drawOverlay(Font customFont);

private:
  std::string ownerName;
  std::map<std::string, std::vector<TileSnapshot>> groupedProperties;
  int totalWealth;
  bool showPopup;
  float scrollOffsetY;

  // HTML Colors Translation
  static const Color PRIMARY;
  static const Color ON_PRIMARY;
  static const Color SURFACE;
  static const Color SURFACE_VARIANT;
  static const Color SURFACE_CONTAINER;
  static const Color OUTLINE;
  static const Color OUTLINE_VARIANT;
  static const Color ON_SURFACE;
  static const Color BACKGROUND;

  bool drawButton(Rectangle rect, const char *text, Color bg, Color fg,
                  Color border, Font font);
};

#endif
