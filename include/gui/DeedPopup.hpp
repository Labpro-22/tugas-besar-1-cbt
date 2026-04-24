#ifndef GUI_DEED_POPUP_HPP
#define GUI_DEED_POPUP_HPP

#include "../app/GameSession.hpp"
#include <raylib.h>
#include <string>

class DeedPopup {
public:
  DeedPopup();
  ~DeedPopup();

  void open();
  void close();
  bool isOpen() const;

  void loadFromTile(const TileSnapshot &snapshot);
  void drawOverlay(Font customFont);

private:
  TileSnapshot currentTile;
  bool showPopup;

  // HTML Colors Translation
  static const Color PRIMARY;
  static const Color SECONDARY_FIXED;
  static const Color BACKGROUND;
  static const Color ON_SURFACE;
  static const Color ON_SURFACE_VARIANT;
  static const Color SURFACE_CONTAINER;
  static const Color SECONDARY_FIXED_DIM;
  static const Color SURFACE_TINT;
  static const Color PRIMARY_CONTAINER;
  static const Color BANNER_BG;

  bool drawButton(Rectangle rect, const char *text, Color bg, Color fg,
                  Color border, Font font);
};

#endif
