#include "gui/LedgerPopup.hpp"
#include <map>

// HTML Colors mapped to Raylib
const Color LedgerPopup::PRIMARY = {65, 0, 2, 255};                // #410002
const Color LedgerPopup::ON_PRIMARY = {255, 255, 255, 255};        // #ffffff
const Color LedgerPopup::SURFACE = {255, 248, 245, 255};           // #fff8f5
const Color LedgerPopup::SURFACE_VARIANT = {233, 225, 220, 255};   // #e9e1dc
const Color LedgerPopup::SURFACE_CONTAINER = {245, 236, 231, 255}; // #f5ece7
const Color LedgerPopup::OUTLINE = {138, 113, 111, 255};           // #8a716f
const Color LedgerPopup::OUTLINE_VARIANT = {221, 192, 189, 255};   // #ddc0bd
const Color LedgerPopup::ON_SURFACE = {30, 27, 24, 255};           // #1e1b18
const Color LedgerPopup::BACKGROUND = {255, 248, 245, 255};        // #fff8f5

LedgerPopup::LedgerPopup()
    : totalWealth(0), showPopup(false), scrollOffsetY(0.0f) {}

LedgerPopup::~LedgerPopup() {}

void LedgerPopup::open() {
  showPopup = true;
  scrollOffsetY = 0.0f;
}

void LedgerPopup::close() { showPopup = false; }

bool LedgerPopup::isOpen() const { return showPopup; }

static std::string mapColorKeyToGroupLabel(const std::string &key) {
  if (key == "CK")
    return "COKLAT";
  if (key == "BM")
    return "BIRU MUDA";
  if (key == "PK")
    return "PINK";
  if (key == "OR")
    return "ORANGE";
  if (key == "MR")
    return "MERAH";
  if (key == "KN")
    return "KUNING";
  if (key == "HJ")
    return "HIJAU";
  if (key == "BT")
    return "BIRU TUA";
  if (key == "AB")
    return "UTILITY";
  return "STASIUN"; // DF
}

// Group Sorting Logic
static int getGroupSortWeight(const std::string &key) {
  if (key == "CK")
    return 1;
  if (key == "BM")
    return 2;
  if (key == "PK")
    return 3;
  if (key == "OR")
    return 4;
  if (key == "MR")
    return 5;
  if (key == "KN")
    return 6;
  if (key == "HJ")
    return 7;
  if (key == "BT")
    return 8;
  if (key == "DF")
    return 9; // Stasiun
  if (key == "AB")
    return 10; // Utility
  return 99;
}

void LedgerPopup::loadFromSnapshot(const GameSnapshot &snapshot) {
  groupedProperties.clear();
  totalWealth = 0;

  // Extract active player
  ownerName = "";
  if (!snapshot.players.empty() && snapshot.activePlayerIndex >= 0 &&
      static_cast<size_t>(snapshot.activePlayerIndex) <
          snapshot.players.size()) {
    ownerName = snapshot.players[snapshot.activePlayerIndex].name;
  }

  for (const auto &tile : snapshot.tiles) {
    if (tile.owner == ownerName && !ownerName.empty()) {
      groupedProperties[tile.colorKey].push_back(tile);
      totalWealth += tile.buyPrice;
      if (tile.type == "Street" && !tile.mortgaged) {
        totalWealth += tile.buildingCount * tile.buildingPrice;
      }
    }
  }
}

static void DrawTextAutofitLB(Font font, const std::string &text,
                              Rectangle bounds, float maxFontSize, Color color,
                              int align = 0, bool fakeBold = false) {
  float fs = maxFontSize;
  Vector2 ts = MeasureTextEx(font, text.c_str(), fs, 1);
  while (ts.x > bounds.width && fs > 8.0f) {
    fs -= 1.0f;
    ts = MeasureTextEx(font, text.c_str(), fs, 1);
  }
  Vector2 pos;
  if (align == -1)
    pos = {bounds.x, bounds.y + bounds.height / 2.0f - ts.y / 2.0f};
  else if (align == 1)
    pos = {bounds.x + bounds.width - ts.x,
           bounds.y + bounds.height / 2.0f - ts.y / 2.0f};
  else
    pos = {bounds.x + bounds.width / 2.0f - ts.x / 2.0f,
           bounds.y + bounds.height / 2.0f - ts.y / 2.0f};

  DrawTextEx(font, text.c_str(), pos, fs, 1, color);
  if (fakeBold)
    DrawTextEx(font, text.c_str(), Vector2{pos.x + 1, pos.y}, fs, 1, color);
}

bool LedgerPopup::drawButton(Rectangle rect, const char *text, Color bg,
                             Color fg, Color border, Font font) {
  Vector2 mousePos = GetMousePosition();
  bool hover = CheckCollisionPointRec(mousePos, rect);
  DrawRectangleRec(rect, hover ? fg : bg);
  DrawRectangleLinesEx(rect, 1, border);
  DrawTextAutofitLB(
      font, text,
      Rectangle{rect.x + 4, rect.y + 4, rect.width - 8, rect.height - 8}, 24,
      hover ? bg : fg, 0, true);
  return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void LedgerPopup::drawOverlay(Font font) {
  if (!showPopup)
    return;

  // Darkened backdrop blur-like
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                Fade(ON_SURFACE, 0.80f));

  float modalW = 672.0f; // max-w-2xl equivalent tightly
  float maxH = GetScreenHeight() * 0.9f;

  Rectangle modal = {(GetScreenWidth() - modalW) * 0.5f,
                     (GetScreenHeight() - maxH) * 0.5f, modalW, maxH};

  DrawRectangleRec(modal, SURFACE);
  DrawRectangleLinesEx(modal, 4, PRIMARY);

  // Header
  Rectangle header = {modal.x, modal.y, modalW, 64.0f};
  DrawRectangleRec(header, SURFACE);
  DrawRectangleLinesEx(
      Rectangle{header.x, header.y + header.height - 4, header.width, 4}, 4.0f,
      PRIMARY);

  std::string titleStr =
      "PROPERTI MILIK: " + (ownerName.empty() ? "TIDAK ADA" : ownerName);
  DrawTextEx(font, "[-]", Vector2{header.x + 20, header.y + 20}, 24, 1,
             PRIMARY); // Dummy icon
  DrawTextAutofitLB(
      font, titleStr,
      Rectangle{header.x + 50, header.y + 16, header.width - 120, 32}, 24,
      PRIMARY, -1, true);

  Rectangle closeBtn = {modal.x + modalW - 56, header.y + 12, 40, 40};
  if (drawButton(closeBtn, "X", SURFACE_VARIANT, PRIMARY, PRIMARY, font)) {
    showPopup = false;
    return;
  }

  // Measure Scroll Content
  float contentH = 0;
  std::vector<std::string> orderedKeys;
  for (const auto &kv : groupedProperties)
    orderedKeys.push_back(kv.first);

  // Sort keys based on configured weight
  for (size_t i = 0; i < orderedKeys.size(); i++) {
    for (size_t j = i + 1; j < orderedKeys.size(); j++) {
      if (getGroupSortWeight(orderedKeys[i]) >
          getGroupSortWeight(orderedKeys[j])) {
        std::swap(orderedKeys[i], orderedKeys[j]);
      }
    }
  }

  for (const auto &key : orderedKeys) {
    contentH += 40.0f;                                 // group header
    contentH += groupedProperties[key].size() * 48.0f; // row size
    contentH += 24.0f;                                 // gap
  }

  // Scroll Logic
  float viewH = maxH - 64.0f - 64.0f; // Header & Footer subtraction
  Rectangle viewBounds = {modal.x, modal.y + 64, modalW, viewH};

  if (contentH > viewH) {
    scrollOffsetY += GetMouseWheelMove() * 20.0f;
    if (scrollOffsetY > 0.0f)
      scrollOffsetY = 0.0f;
    if (scrollOffsetY < -(contentH - viewH))
      scrollOffsetY = -(contentH - viewH);
  } else {
    scrollOffsetY = 0.0f;
  }

  BeginScissorMode(viewBounds.x, viewBounds.y, viewBounds.width,
                   viewBounds.height);

  float currentY = viewBounds.y + 24.0f + scrollOffsetY;
  float px = modal.x + 24.0f;
  float cw = modalW - 48.0f;

  for (const auto &key : orderedKeys) {
    const auto &tiles = groupedProperties[key];

    float groupH = 40.0f + tiles.size() * 48.0f;
    Rectangle groupRect = {px, currentY, cw, groupH};
    DrawRectangleRec(groupRect, SURFACE_VARIANT);
    DrawRectangleLinesEx(groupRect, 1, PRIMARY);
    // Shadow effect based on html shadow-[2px_2px_0px_0px_#410002]
    DrawRectangle(groupRect.x + groupRect.width, groupRect.y + 2, 2,
                  groupRect.height, PRIMARY);
    DrawRectangle(groupRect.x + 2, groupRect.y + groupRect.height,
                  groupRect.width, 2, PRIMARY);

    // Group Header
    Rectangle grpHdr = {groupRect.x, groupRect.y, groupRect.width, 40.0f};
    DrawRectangleRec(grpHdr, PRIMARY);
    DrawRectangleLinesEx(
        Rectangle{grpHdr.x, grpHdr.y + grpHdr.height - 1, grpHdr.width, 1},
        1.0f, PRIMARY);

    std::string grpLabel = "[" + mapColorKeyToGroupLabel(key) + "]";
    DrawTextAutofitLB(
        font, grpLabel,
        Rectangle{grpHdr.x + 12, grpHdr.y, grpHdr.width - 24, grpHdr.height},
        12, ON_PRIMARY, -1, true);
    std::string countLbl = std::to_string(tiles.size()) + " ASET";
    DrawTextAutofitLB(
        font, countLbl,
        Rectangle{grpHdr.x + 12, grpHdr.y, grpHdr.width - 24, grpHdr.height},
        12, ON_PRIMARY, 1, true);

    float rowY = grpHdr.y + 40.0f;
    for (const auto &tile : tiles) {
      Rectangle rowR = {groupRect.x, rowY, groupRect.width, 48.0f};
      DrawRectangleRec(rowR, SURFACE);
      DrawRectangleLinesEx(
          Rectangle{rowR.x, rowR.y + rowR.height - 1, rowR.width, 1}, 1.0f,
          PRIMARY);

      Color rowCol = tile.mortgaged ? OUTLINE : PRIMARY;

      // Name Cell
      Rectangle nameC = {rowR.x, rowY, rowR.width * 0.45f, rowR.height};
      DrawRectangleLinesEx(
          Rectangle{nameC.x + nameC.width - 1, nameC.y, 1, nameC.height}, 1.0f,
          PRIMARY);
      DrawTextAutofitLB(
          font, tile.name,
          Rectangle{nameC.x + 12, nameC.y, nameC.width - 24, nameC.height}, 16,
          rowCol, -1, true);
      if (tile.mortgaged) {
        // Line through
        Vector2 textSize = MeasureTextEx(font, tile.name.c_str(), 16, 1);
        DrawRectangle(nameC.x + 12, nameC.y + nameC.height / 2, textSize.x, 2,
                      rowCol);
      }

      // Status Cell
      Rectangle statC = {rowR.x + rowR.width * 0.45f, rowY, rowR.width * 0.25f,
                         rowR.height};
      DrawRectangleLinesEx(
          Rectangle{statC.x + statC.width - 1, statC.y, 1, statC.height}, 1.0f,
          PRIMARY);

      std::string stText = "-";
      if (tile.type == "Street") {
        if (tile.buildingCount == 5)
          stText = "Hotel";
        else if (tile.buildingCount > 0)
          stText = std::to_string(tile.buildingCount) + " rumah";
      }
      DrawTextAutofitLB(
          font, stText,
          Rectangle{statC.x + 12, statC.y, statC.width - 24, statC.height}, 16,
          rowCol, -1, false);

      // Value & Tag Cell
      Rectangle valC = {statC.x + statC.width, rowY, rowR.width * 0.30f,
                        rowR.height};
      std::string valT = "M" + std::to_string(tile.buyPrice);
      std::string tagT = tile.mortgaged ? "MORTGAGED" : "OWNED";

      // Layout inner logic simply
      Rectangle tagR = {valC.x + valC.width - 70 - 12,
                        valC.y + valC.height / 2 - 8, 70, 16};
      DrawRectangleRec(tagR, tile.mortgaged ? OUTLINE : PRIMARY);
      DrawTextAutofitLB(font, tagT, tagR, 10, SURFACE, 0, true);

      Rectangle prcR = {valC.x, valC.y, valC.width - 80 - 16, valC.height};
      DrawTextAutofitLB(font, valT, prcR, 16, rowCol, 1, false);

      rowY += 48.0f;
    }

    currentY += groupH + 24.0f;
  }

  EndScissorMode();

  if (contentH > viewH) {
    float scrollBarH = viewH * (viewH / contentH);
    float scrollBarY = viewBounds.y + (-scrollOffsetY / contentH) * viewH;
    DrawRectangle(viewBounds.x + viewBounds.width - 8, viewBounds.y, 8,
                  viewBounds.height, SURFACE);
    DrawRectangle(viewBounds.x + viewBounds.width - 8, scrollBarY, 8,
                  scrollBarH, PRIMARY);
  }

  // Footer
  Rectangle footer = {modal.x, modal.y + maxH - 64.0f, modalW, 64.0f};
  DrawRectangleRec(footer, SURFACE_VARIANT);
  DrawRectangleLinesEx(Rectangle{footer.x, footer.y, footer.width, 4}, 4.0f,
                       PRIMARY);

  DrawTextAutofitLB(font, "Total kekayaan properti:",
                    Rectangle{footer.x + 24, footer.y, 160, footer.height}, 12,
                    PRIMARY, -1, true);

  std::string tv = "M" + std::to_string(totalWealth);
  DrawTextAutofitLB(
      font, tv,
      Rectangle{footer.x + footer.width - 300, footer.y, 276, footer.height},
      28, PRIMARY, 1, true);
}
