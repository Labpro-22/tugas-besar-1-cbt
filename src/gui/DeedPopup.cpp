#include "gui/DeedPopup.hpp"
#include <algorithm>

const Color DeedPopup::PRIMARY = {65, 0, 2, 255};
const Color DeedPopup::SECONDARY_FIXED = {236, 226, 203, 255};
const Color DeedPopup::BACKGROUND = {255, 248, 245, 255};
const Color DeedPopup::ON_SURFACE = {30, 27, 24, 255};
const Color DeedPopup::ON_SURFACE_VARIANT = {86, 66, 64, 255};
const Color DeedPopup::SURFACE_CONTAINER = {245, 236, 231, 255};
const Color DeedPopup::SECONDARY_FIXED_DIM = {208, 198, 176, 255};
const Color DeedPopup::SURFACE_TINT = {162, 60, 54, 255};
const Color DeedPopup::PRIMARY_CONTAINER = {99, 13, 13, 255};
const Color DeedPopup::BANNER_BG = {0, 0, 139, 255};

DeedPopup::DeedPopup() : showPopup(false) {}

DeedPopup::~DeedPopup() {}

void DeedPopup::open() { showPopup = true; }

void DeedPopup::close() { showPopup = false; }

bool DeedPopup::isOpen() const { return showPopup; }

void DeedPopup::loadFromTile(const TileSnapshot &snapshot) {
  currentTile = snapshot;
}

bool DeedPopup::drawButton(Rectangle r, const char *text, Color bg, Color fg,
                           Color border, Font font) {
  Vector2 m = GetMousePosition();
  bool hover = CheckCollisionPointRec(m, r);
  DrawRectangleRec(r, hover ? Fade(bg, 0.85f) : bg);
  DrawRectangleLinesEx(r, 1, border);

  const int fs = 20;
  Vector2 ts = MeasureTextEx(font, text, fs, 1);
  DrawTextEx(font, text,
             Vector2{r.x + (r.width - ts.x) / 2, r.y + (r.height - fs) / 2}, fs,
             1, fg);

  return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

static void DrawTextAutofit(Font font, const std::string &text,
                            Rectangle bounds, float maxFontSize, Color color,
                            int align = 0, bool fakeBold = false) {
  // align: -1 left, 0 center, 1 right
  float fs = maxFontSize;
  Vector2 ts = MeasureTextEx(font, text.c_str(), fs, 1);
  while (ts.x > bounds.width && fs > 8.0f) {
    fs -= 1.0f;
    ts = MeasureTextEx(font, text.c_str(), fs, 1);
  }

  Vector2 pos;
  if (align == -1) {
    pos = {bounds.x, bounds.y + (bounds.height / 2.0f) - (ts.y / 2.0f)};
  } else if (align == 1) {
    pos = {bounds.x + bounds.width - ts.x,
           bounds.y + (bounds.height / 2.0f) - (ts.y / 2.0f)};
  } else {
    pos = {bounds.x + (bounds.width / 2.0f) - (ts.x / 2.0f),
           bounds.y + (bounds.height / 2.0f) - (ts.y / 2.0f)};
  }

  DrawTextEx(font, text.c_str(), pos, fs, 1, color);
  if (fakeBold) {
    DrawTextEx(font, text.c_str(), Vector2{pos.x + 1, pos.y}, fs, 1, color);
  }
}

static Color resolveColorKey(const std::string &key) {
  if (key == "CK")
    return Color{139, 69, 19, 255}; // Coklat
  if (key == "BM")
    return Color{173, 216, 230, 255}; // Biru Muda
  if (key == "PK")
    return Color{255, 105, 180, 255}; // Merah Muda
  if (key == "OR")
    return Color{255, 140, 0, 255}; // Orange
  if (key == "MR")
    return Color{220, 20, 60, 255}; // Merah
  if (key == "KN")
    return Color{255, 215, 0, 255}; // Kuning
  if (key == "HJ")
    return Color{34, 139, 34, 255}; // Hijau
  if (key == "BT")
    return Color{0, 0, 139, 255}; // Biru Tua
  if (key == "AB")
    return Color{105, 105, 105, 255}; // Abu-Abu
  return Color{0, 0, 139, 255}; // Default fallback (BANNER_BG was #00008B)
}

void DeedPopup::drawOverlay(Font font) {
  if (!showPopup)
    return;

  // Darkened backdrop (fixed bg-black/60 inset-0)
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.60f));

  float modalW = 360.0f;
  float modalH = 0.0f;

  // Base height calculation mimicking HTML flow
  float baseH = 48.0f + 16.0f; // Top padding + close button area
  baseH += 76.0f + 16.0f;      // Header Box + gap
  baseH += 54.0f + 16.0f;      // Pricing Section + gap

  float rentH = 32.0f; // Rent Structure Header
  float count = static_cast<float>(currentTile.rentLevels.size());
  rentH += (count > 0 ? count : 6.0f) * 32.0f;
  rentH += 4.0f; // bottom border
  baseH += rentH + 16.0f;

  if (currentTile.type == "Street") {
    baseH += 50.0f + 8.0f; // Development Costs + gap
  } else {
    baseH += 8.0f;
  }

  baseH += 50.0f; // Status Footer

  modalH = baseH;

  Rectangle modal = {(GetScreenWidth() - modalW) * 0.5f,
                     (GetScreenHeight() - modalH) * 0.5f, modalW, modalH};

  // Modal Background & Border
  DrawRectangleRec(modal, BACKGROUND);
  DrawRectangleLinesEx(modal, 4, PRIMARY);

  // Close Button
  Rectangle closeBtn = {modal.x + modalW - 48, modal.y, 48, 48};
  drawButton(closeBtn, "X", PRIMARY_CONTAINER, WHITE, PRIMARY, font);
  DrawRectangleRec(Rectangle{closeBtn.x, closeBtn.y, 4, closeBtn.height},
                   PRIMARY);
  DrawRectangleRec(Rectangle{closeBtn.x, closeBtn.y + closeBtn.height - 4,
                             closeBtn.width, 4},
                   PRIMARY);

  if (CheckCollisionPointRec(GetMousePosition(), closeBtn) &&
      IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    showPopup = false;
    return;
  }

  float currentY = modal.y + 48.0f;
  float px = modal.x + 16.0f;
  float cw = modalW - 32.0f;

  // Header Box
  Rectangle headerRect = {px, currentY, cw, 76.0f};
  DrawRectangleRec(headerRect, resolveColorKey(currentTile.colorKey));
  DrawRectangleLinesEx(headerRect, 4, PRIMARY);

  Rectangle hTagBounds = {headerRect.x + 8, headerRect.y + 8,
                          headerRect.width - 16, 20};
  DrawTextAutofit(font, "AKTA KEPEMILIKAN", hTagBounds, 12, Fade(WHITE, 0.9f),
                  0, true);

  Rectangle nmBounds = {headerRect.x + 8, headerRect.y + 28,
                        headerRect.width - 16, 40};
  DrawTextAutofit(font, currentTile.name, nmBounds, 28, WHITE, 0, true);

  currentY += 76.0f + 16.0f;

  if (currentTile.type != "Street" && currentTile.type != "Railroad" &&
      currentTile.type != "Utility" && currentTile.type != "property") {
    DrawTextAutofit(font, "Petak ini bukan properti.",
                    Rectangle{px, currentY, cw, 40.0f}, 16, ON_SURFACE, 0,
                    false);
    return;
  }

  // Pricing Section
  Rectangle priceRect = {px, currentY, cw, 54.0f};
  DrawRectangleRec(priceRect, SECONDARY_FIXED);
  DrawRectangleLinesEx(priceRect, 1, PRIMARY);

  float halfW = cw / 2.0f;

  Rectangle lb1 = {priceRect.x + 4, priceRect.y + 4, halfW - 8, 20};
  DrawTextAutofit(font, "HARGA BELI", lb1, 12, ON_SURFACE_VARIANT, 0, false);
  Rectangle vb1 = {priceRect.x + 4, priceRect.y + 24, halfW - 8, 26};
  std::string buyStr = "M" + std::to_string(currentTile.buyPrice);
  DrawTextAutofit(font, buyStr, vb1, 20, ON_SURFACE, 0, true);

  DrawRectangle(static_cast<int>(priceRect.x + halfW),
                static_cast<int>(priceRect.y + 12), 1, 30, Fade(PRIMARY, 0.5f));

  Rectangle lb2 = {priceRect.x + halfW + 4, priceRect.y + 4, halfW - 8, 20};
  DrawTextAutofit(font, "NILAI GADAI", lb2, 12, ON_SURFACE_VARIANT, 0, false);
  Rectangle vb2 = {priceRect.x + halfW + 4, priceRect.y + 24, halfW - 8, 26};
  std::string mortStr = "M" + std::to_string(currentTile.mortgagePrice);
  DrawTextAutofit(font, mortStr, vb2, 20, ON_SURFACE, 0, true);

  currentY += 54.0f + 16.0f;

  // Rent Structure
  Rectangle rentRect = {px, currentY, cw, rentH};
  DrawRectangleRec(rentRect, BACKGROUND);
  DrawRectangleLinesEx(rentRect, 4, PRIMARY);

  Rectangle rentHdr = {rentRect.x, rentRect.y, cw, 28.0f};
  DrawRectangleRec(rentHdr, SECONDARY_FIXED_DIM);
  DrawRectangleRec(
      Rectangle{rentHdr.x, rentHdr.y + rentHdr.height - 1, rentHdr.width, 1},
      PRIMARY);

  Rectangle rentHdrTitle = {rentHdr.x + 8, rentHdr.y, rentHdr.width - 16,
                            rentHdr.height};
  DrawTextAutofit(font, "DAFTAR HARGA SEWA", rentHdrTitle, 14, ON_SURFACE, 0,
                  true);

  float rowY = rentHdr.y + 28.0f;

  if (currentTile.type == "Street") {
    const char *labels[] = {"Sewa (Kosong)", "1 Rumah", "2 Rumah",
                            "3 Rumah",       "4 Rumah", "Hotel"};
    size_t rCount = currentTile.rentLevels.size();
    for (size_t i = 0; i < rCount && i < 6; i++) {
      Rectangle row = {rentRect.x, rowY, cw, 32.0f};
      if (i % 2 != 0)
        DrawRectangleRec(row, SURFACE_CONTAINER);
      DrawRectangleRec(Rectangle{row.x, row.y + row.height - 1, row.width, 1},
                       PRIMARY);

      std::string amt = "M" + std::to_string(currentTile.rentLevels[i]);
      Color c = (i < 5) ? ON_SURFACE : SURFACE_TINT;

      Rectangle lblB = {row.x + 8, row.y, row.width / 2.0f - 8, row.height};
      DrawTextAutofit(font, labels[i], lblB, 16, ON_SURFACE, -1, false);

      Rectangle amtB = {row.x + row.width / 2.0f, row.y, row.width / 2.0f - 8,
                        row.height};
      DrawTextAutofit(font, amt, amtB, 16, c, 1, true);
      rowY += 32.0f;
    }
    currentY += rentH + 16.0f;

    // Development Costs
    float gap = 8.0f;
    float bW = (cw - gap) / 2.0f;

    Rectangle upg1 = {px, currentY, bW, 50.0f};
    DrawRectangleRec(upg1, SECONDARY_FIXED);
    DrawRectangleLinesEx(upg1, 1, PRIMARY);

    Rectangle hHdr1 = {upg1.x + 4, upg1.y + 4, upg1.width - 8, 16};
    DrawTextAutofit(font, "HARGA RUMAH", hHdr1, 12, ON_SURFACE_VARIANT, 0,
                    false);
    Rectangle hVal1 = {upg1.x + 4, upg1.y + 20, upg1.width - 8, 26};
    std::string ha = "M" + std::to_string(currentTile.houseCost);
    DrawTextAutofit(font, ha, hVal1, 20, ON_SURFACE, 0, true);

    Rectangle upg2 = {px + bW + gap, currentY, bW, 50.0f};
    DrawRectangleRec(upg2, SECONDARY_FIXED);
    DrawRectangleLinesEx(upg2, 1, PRIMARY);

    Rectangle hHdr2 = {upg2.x + 4, upg2.y + 4, upg2.width - 8, 16};
    DrawTextAutofit(font, "HARGA HOTEL", hHdr2, 12, ON_SURFACE_VARIANT, 0,
                    false);
    Rectangle hVal2 = {upg2.x + 4, upg2.y + 20, upg2.width - 8, 26};
    std::string hla = "M" + std::to_string(currentTile.hotelCost);
    DrawTextAutofit(font, hla, hVal2, 20, ON_SURFACE, 0, true);

    currentY += 50.0f + 8.0f;
  } else {
    // Utilities or Railroad
    size_t rCount = currentTile.rentLevels.size();
    for (size_t i = 0; i < rCount; i++) {
      Rectangle row = {rentRect.x, rowY, cw, 32.0f};
      if (i % 2 != 0)
        DrawRectangleRec(row, SURFACE_CONTAINER);
      DrawRectangleRec(Rectangle{row.x, row.y + row.height - 1, row.width, 1},
                       PRIMARY);

      std::string lbl = std::to_string(i + 1) + " dimiliki";
      if (currentTile.type == "Utility")
        lbl = "Gandaan dadu: x" + std::to_string(currentTile.rentLevels[i]);

      Rectangle lblB = {row.x + 8, row.y, row.width / 2.0f - 8, row.height};
      DrawTextAutofit(font, lbl, lblB, 16, ON_SURFACE, -1, false);

      if (currentTile.type == "Railroad") {
        std::string amt = "M" + std::to_string(currentTile.rentLevels[i]);
        Rectangle amtB = {row.x + row.width / 2.0f, row.y, row.width / 2.0f - 8,
                          row.height};
        DrawTextAutofit(font, amt, amtB, 16, ON_SURFACE, 1, true);
      }
      rowY += 32.0f;
    }
    currentY += rentH + 8.0f;
  }

  // Status Footer
  Rectangle footerBorder = {modal.x, currentY, modalW, 4.0f};
  DrawRectangleRec(footerBorder, PRIMARY);

  currentY += 16.0f;

  std::string sTxt = "STATUS: BANK";
  if (!currentTile.owner.empty()) {
    sTxt = "STATUS: OWNED (" + currentTile.owner + ")";
  }

  Vector2 sts = MeasureTextEx(font, sTxt.c_str(), 14, 1);
  float tagW = sts.x + 32;
  if (tagW > modalW - 24)
    tagW = modalW - 24;

  Rectangle sTag = {modal.x + modalW / 2.0f - tagW / 2.0f, currentY, tagW,
                    30.0f};
  DrawRectangleRec(sTag, PRIMARY_CONTAINER);
  DrawRectangleLinesEx(sTag, 1, PRIMARY);

  Rectangle sTagTxt = {sTag.x + 4, sTag.y + 2, sTag.width - 8, sTag.height - 4};
  DrawTextAutofit(font, sTxt, sTagTxt, 14, WHITE, 0, true);
}
