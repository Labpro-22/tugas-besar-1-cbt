#include "gui/LogPopup.hpp"
#include <algorithm>

// ── LogEntry ────────────────────────────────────────────────────────────────

LogPopup::LogEntry::LogEntry(int turn, const std::string &user, const std::string &action, const std::string &detail)
: turn(turn), user(user), action(action), detail(detail) {}

// ── Static colour constants ─────────────────────────────────────────────────

const Color LogPopup::MAROON_COLOR = {99, 13, 13, 255};
const Color LogPopup::BEIGE_COLOR = {245, 245, 220, 255};
const Color LogPopup::DARK_COLOR = {26, 24, 22, 255};
const Color LogPopup::SAND_COLOR = {235, 220, 178, 255};

// ── Constructor / Destructor ────────────────────────────────────────────────

LogPopup::LogPopup() : showPopup(false), scrollOffset(0.0f), horizontalScrollOffset(0.0f),
scrollbarDragging(false), scrollbarDraggingH(false), scrollbarGrabOffset(0.0f), scrollbarGrabOffsetH(0.0f) {}

LogPopup::~LogPopup() {}

// ── State management ────────────────────────────────────────────────────────

void LogPopup::open() { showPopup = true; }

void LogPopup::close() { showPopup = false; }

bool LogPopup::isOpen() const { return showPopup; }

// ── Adding log entries ──────────────────────────────────────────────────────

void LogPopup::addTurnLog(int turn, const std::string &user,
                          const std::string &action,
                          const std::string &detail) {
  logs.push_back(LogEntry(turn, user.empty() ? "-" : user,
                          action.empty() ? "-" : action,
                          detail.empty() ? "-" : detail));
}

void LogPopup::addSystemLog(const std::string &detail) {
  logs.push_back(LogEntry(-1, "SYSTEM", "INFO", detail.empty() ? "-" : detail));
}

void LogPopup::loadFromSnapshot(const GameSnapshot &snapshot, int limit) {
  logs.clear();

  auto startIt = snapshot.logs.begin();
  if (limit > 0 && static_cast<int>(snapshot.logs.size()) > limit) {
    startIt = snapshot.logs.end() - limit;
  }

  for (auto it = startIt; it != snapshot.logs.end(); ++it) {
    const LogSnapshot &entry = *it;
    logs.push_back(LogEntry(entry.turn,
                            entry.username.empty() ? "-" : entry.username,
                            entry.actionType.empty() ? "-" : entry.actionType,
                            entry.detail.empty() ? "-" : entry.detail));
  }
}

// ── Private helper ──────────────────────────────────────────────────────────

bool LogPopup::drawButton(Rectangle r, const char *text, Color bg, Color fg,
                          Color border) {
  Vector2 m = GetMousePosition();
  bool hover = CheckCollisionPointRec(m, r);
  DrawRectangleRec(r, hover ? Fade(bg, 0.85f) : bg);
  DrawRectangleLinesEx(r, 2, border);

  const int fs = 24;
  int tw = MeasureText(text, fs);
  DrawText(text, static_cast<int>(r.x + (r.width - tw) / 2),
           static_cast<int>(r.y + (r.height - fs) / 2), fs, fg);

  return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// ── Main overlay draw ───────────────────────────────────────────────────────

void LogPopup::drawOverlay() {
  if (!showPopup)
    return;

  // Dimmed background
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.60f));

  // Modal frame
  float modalW = 760.0f;
  float modalH = 640.0f;
  Rectangle modal = {(GetScreenWidth() - modalW) * 0.5f,
                     (GetScreenHeight() - modalH) * 0.5f, modalW, modalH};

  DrawRectangleRec(modal, BEIGE_COLOR);
  DrawRectangleLinesEx(modal, 4, MAROON_COLOR);

  // Header band
  Rectangle header = {modal.x, modal.y, modal.width, 112};
  DrawRectangleRec(header, BEIGE_COLOR);
  DrawRectangleLinesEx(header, 2, MAROON_COLOR);
  const char *mainTitle = "CETAK LOG";
  int mainTitleW = MeasureText(mainTitle, 46);
  DrawText(mainTitle, static_cast<int>(header.x + (header.width - mainTitleW) / 2),
           static_cast<int>(header.y) + 20, 46, MAROON_COLOR);

  const char *subTitle = "NIMONSPOLI TRANSACTION LEDGER";
  int subTitleW = MeasureText(subTitle, 16);
  DrawText(subTitle, static_cast<int>(header.x + (header.width - subTitleW) / 2),
           static_cast<int>(header.y) + 76, 16, Fade(MAROON_COLOR, 0.85f));

  // Close button
  Rectangle closeBtn = {modal.x + modal.width - 56, modal.y, 56, 56};
  if (drawButton(closeBtn, "X", MAROON_COLOR, BEIGE_COLOR, MAROON_COLOR)) {
    showPopup = false;
    return;
  }

  // Log area (dark terminal)
  Rectangle logArea = {modal.x + 20, modal.y + 132, modal.width - 40, modal.height - 152};
  DrawRectangleRec(logArea, DARK_COLOR);
  DrawRectangleLinesEx(logArea, 2, MAROON_COLOR);

  // Scroll logic & bars
  Vector2 mouse = GetMousePosition();
  float lineH = 28.0f;
  float contentH = 44.0f + (static_cast<float>(logs.size()) * lineH) + 36.0f;
  
  // Calculate max content width
  float maxContentW = 800.0f; // Minimum virtual width
  for (const auto& e : logs) {
      float w = 435.0f + MeasureText(e.detail.c_str(), 20) + 40.0f;
      if (w > maxContentW) maxContentW = w;
  }

  float maxScrollV = (contentH > logArea.height - 12) ? (contentH - (logArea.height - 12)) : 0.0f;
  float maxScrollH = (maxContentW > logArea.width - 12) ? (maxContentW - (logArea.width - 12)) : 0.0f;

  if (CheckCollisionPointRec(mouse, logArea)) {
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        horizontalScrollOffset -= GetMouseWheelMove() * 40.0f;
    } else {
        scrollOffset -= GetMouseWheelMove() * 32.0f;
    }
  }

  // Vertical scrollbar track
  Rectangle scrollTrackV = {logArea.x + logArea.width - 12, logArea.y, 12, logArea.height - 12};
  DrawRectangleRec(scrollTrackV, Color{40, 35, 30, 255});

  if (maxScrollV > 0) {
    float thumbH = std::max(30.0f, scrollTrackV.height * (scrollTrackV.height / contentH));
    float thumbY = logArea.y + (scrollOffset / maxScrollV) * (scrollTrackV.height - thumbH);
    Rectangle thumbV = {scrollTrackV.x + 2, thumbY, 8, thumbH};
    
    bool thumbHoverV = CheckCollisionPointRec(mouse, thumbV);
    if (thumbHoverV && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        scrollbarDragging = true;
        scrollbarGrabOffset = mouse.y - thumbY;
    }

    if (scrollbarDragging) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            float newY = mouse.y - scrollbarGrabOffset;
            float thumbTravel = scrollTrackV.height - thumbH;
            if (thumbTravel > 0) {
                float ratio = (newY - logArea.y) / thumbTravel;
                scrollOffset = std::clamp(ratio * maxScrollV, 0.0f, maxScrollV);
            }
        } else {
            scrollbarDragging = false;
        }
    }
    DrawRectangleRounded(thumbV, 0.8f, 6, (scrollbarDragging || thumbHoverV) ? MAROON_COLOR : Color{120, 100, 90, 255});
  }

  // Horizontal scrollbar track
  Rectangle scrollTrackH = {logArea.x, logArea.y + logArea.height - 12, logArea.width - 12, 12};
  DrawRectangleRec(scrollTrackH, Color{40, 35, 30, 255});

  if (maxScrollH > 0) {
      float thumbW = std::max(30.0f, scrollTrackH.width * (scrollTrackH.width / maxContentW));
      float thumbX = logArea.x + (horizontalScrollOffset / maxScrollH) * (scrollTrackH.width - thumbW);
      Rectangle thumbH_rect = {thumbX, scrollTrackH.y + 2, thumbW, 8};

      bool thumbHoverH = CheckCollisionPointRec(mouse, thumbH_rect);
      if (thumbHoverH && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          scrollbarDraggingH = true;
          scrollbarGrabOffsetH = mouse.x - thumbX;
      }

      if (scrollbarDraggingH) {
          if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
              float newX = mouse.x - scrollbarGrabOffsetH;
              float thumbTravel = scrollTrackH.width - thumbW;
              if (thumbTravel > 0) {
                  float ratio = (newX - logArea.x) / thumbTravel;
                  horizontalScrollOffset = std::clamp(ratio * maxScrollH, 0.0f, maxScrollH);
              }
          } else {
              scrollbarDraggingH = false;
          }
      }
      DrawRectangleRounded(thumbH_rect, 0.8f, 6, (scrollbarDraggingH || thumbHoverH) ? MAROON_COLOR : Color{120, 100, 90, 255});
  }

  scrollOffset = std::clamp(scrollOffset, 0.0f, maxScrollV);
  horizontalScrollOffset = std::clamp(horizontalScrollOffset, 0.0f, maxScrollH);

  // Clipped content
  BeginScissorMode(static_cast<int>(logArea.x), static_cast<int>(logArea.y),
                   static_cast<int>(logArea.width - 12),
                   static_cast<int>(logArea.height - 12));

  float startX = logArea.x - horizontalScrollOffset;
  float y = logArea.y + 12.0f - scrollOffset;
  DrawText("*** INITIALIZING LEDGER STREAM ***", static_cast<int>(startX) + 12, static_cast<int>(y), 18, Fade(MAROON_COLOR, 0.9f));
  y += 34.0f;

  // Horizontal offsets 
  const int colTurn = 12;
  const int colUser = 100;
  const int colAction = 235;
  const int colArrow = 410;
  const int colDetail = 435;

  // Render newest-first
  for (auto it = logs.rbegin(); it != logs.rend(); ++it) {
    const LogEntry &e = *it;
    
    // Skip if outside view
    if (y + lineH < logArea.y) {
        y += lineH;
        continue;
    }
    if (y > logArea.y + logArea.height) break;

    const char *timeTag = (e.turn >= 0) ? TextFormat("[T:%02d]", e.turn) : "[SYS]";
    DrawText(timeTag, static_cast<int>(startX) + colTurn, static_cast<int>(y), 20, Color{131, 37, 33, 255});
    DrawText(e.user.c_str(), static_cast<int>(startX) + colUser, static_cast<int>(y), 20, RAYWHITE);
    DrawText(":", static_cast<int>(startX) + colAction - 20, static_cast<int>(y), 20, SAND_COLOR);
    DrawText(e.action.c_str(), static_cast<int>(startX) + colAction, static_cast<int>(y), 20, SAND_COLOR);
    DrawText(">", static_cast<int>(startX) + colArrow, static_cast<int>(y), 20, SAND_COLOR);
    DrawText(e.detail.c_str(), static_cast<int>(startX) + colDetail, static_cast<int>(y), 20, SAND_COLOR);
    y += lineH;
  }

  DrawText("_", static_cast<int>(startX) + 12, static_cast<int>(y) + 8, 22, SAND_COLOR);
  EndScissorMode();
}
