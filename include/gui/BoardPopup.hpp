#ifndef GUI_BOARD_POPUP_HPP
#define GUI_BOARD_POPUP_HPP

#include <raylib.h>
#include <string>

#include "app/GameSession.hpp"

class BoardPopup {
public:
  BoardPopup();
  ~BoardPopup();

  void open();
  void close();
  bool isOpen() const;

  void drawOverlay(const GameSnapshot &snapshot, Font font);

private:
  bool showPopup;

  // Render utilities exactly matching existing Board style but fitted to the
  // modal format
  bool drawCloseButton(Rectangle rect, const char *text, Color bg, Color fg,
                       Color border, Font font);
};

#endif // GUI_BOARD_POPUP_HPP
