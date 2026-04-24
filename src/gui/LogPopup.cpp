#include "gui/LogPopup.hpp"
#include <algorithm>

// ── LogEntry ────────────────────────────────────────────────────────────────

LogPopup::LogEntry::LogEntry(int turn, const std::string &user,
                             const std::string &action,
                             const std::string &detail)
    : turn(turn), user(user), action(action), detail(detail) {}

// ── Static colour constants ─────────────────────────────────────────────────

const Color LogPopup::MAROON_COLOR = {99, 13, 13, 255};
const Color LogPopup::BEIGE_COLOR = {245, 245, 220, 255};
const Color LogPopup::DARK_COLOR = {26, 24, 22, 255};
const Color LogPopup::SAND_COLOR = {235, 220, 178, 255};

// ── Constructor / Destructor ────────────────────────────────────────────────

LogPopup::LogPopup() : showPopup(false), scrollOffset(0.0f) {}

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
  DrawText("CETAK LOG", static_cast<int>(header.x) + 24,
           static_cast<int>(header.y) + 20, 46, MAROON_COLOR);
  DrawText("NIMONSPOLI TRANSACTION LEDGER", static_cast<int>(header.x) + 24,
           static_cast<int>(header.y) + 76, 16, Fade(MAROON_COLOR, 0.85f));

  // Close button
  Rectangle closeBtn = {modal.x + modal.width - 56, modal.y, 56, 56};
  if (drawButton(closeBtn, "X", MAROON_COLOR, BEIGE_COLOR, MAROON_COLOR)) {
    showPopup = false;
    return;
  }

  // Log area (dark terminal)
  Rectangle logArea = {modal.x + 20, modal.y + 132, modal.width - 40,
                       modal.height - 152};
  DrawRectangleRec(logArea, DARK_COLOR);
  DrawRectangleLinesEx(logArea, 2, MAROON_COLOR);

  // Scroll handling
  Vector2 mouse = GetMousePosition();
  if (CheckCollisionPointRec(mouse, logArea)) {
    scrollOffset -= GetMouseWheelMove() * 24.0f;
  }

  float lineH = 28.0f;
  float contentH = 44.0f + (static_cast<float>(logs.size()) * lineH) + 24.0f;
  float maxScroll =
      (contentH > logArea.height) ? (contentH - logArea.height) : 0.0f;
  scrollOffset = std::clamp(scrollOffset, 0.0f, maxScroll);

  // Clipped content
  BeginScissorMode(static_cast<int>(logArea.x), static_cast<int>(logArea.y),
                   static_cast<int>(logArea.width),
                   static_cast<int>(logArea.height));

  float y = logArea.y + 12.0f - scrollOffset;
  DrawText("*** INITIALIZING LEDGER STREAM ***",
           static_cast<int>(logArea.x) + 12, static_cast<int>(y), 18,
           Fade(MAROON_COLOR, 0.9f));
  y += 34.0f;

  // Render newest-first
  for (auto it = logs.rbegin(); it != logs.rend(); ++it) {
    const LogEntry &e = *it;
    const char *timeTag =
        (e.turn >= 0) ? TextFormat("[T:%02d]", e.turn) : "[SYS]";
    DrawText(timeTag, static_cast<int>(logArea.x) + 12, static_cast<int>(y), 20,
             Color{131, 37, 33, 255});
    DrawText(e.user.c_str(), static_cast<int>(logArea.x) + 110,
             static_cast<int>(y), 20, RAYWHITE);
    DrawText(":", static_cast<int>(logArea.x) + 210, static_cast<int>(y), 20,
             SAND_COLOR);
    DrawText(e.action.c_str(), static_cast<int>(logArea.x) + 230,
             static_cast<int>(y), 20, SAND_COLOR);
    DrawText(">", static_cast<int>(logArea.x) + 330, static_cast<int>(y), 20,
             SAND_COLOR);
    DrawText(e.detail.c_str(), static_cast<int>(logArea.x) + 355,
             static_cast<int>(y), 20, SAND_COLOR);
    y += lineH;
  }

  DrawText("_", static_cast<int>(logArea.x) + 12, static_cast<int>(y) + 8, 22,
           SAND_COLOR);
  EndScissorMode();
}
