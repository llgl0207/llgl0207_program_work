#include "ui_menu.h"
#include "draw_esp.h"

enum UIState {
  UI_MENU_MAIN = 0,
  UI_MENU_MUSIC,
  UI_MENU_TEXT,
  UI_MENU_GAMES,
  UI_MENU_SETTINGS
};

struct MenuContext {
  UIState state;
  int cursor;
};

static MenuContext ctx;

static const char* mainItems[]    = {"Music", "Text", "Games", "Settings"};
static const char* musicItems[]   = {"Play", "List", "Back"};
static const char* textItems[]    = {"Open", "Back"};
static const char* gameItems[]    = {"Snake", "Breakout", "Flappy", "Back"};
static const char* settingItems[] = {"DrawMode", "Speed", "Back"};

static void enter_current();
static void go_back();

static void draw_menu(const char* const* items, int count) {
  const int baseX = 600;
  const int baseY = 3500;
  const int itemSpacing = 360;
  const uint16_t labelScale = 110; // percent
  for (int i=0;i<count;i++) {
    int y = baseY - i*itemSpacing;
    DRAW_RenderStringESP(items[i], baseX + 420, y, labelScale, 260);
    uint16_t px = (uint16_t)baseX;
    uint16_t py = (uint16_t)y;
    int reps = (i == ctx.cursor) ? 28 : 6;
    for (int k=0;k<reps;k++) DRAW_AddPointESP(px, py);
  }
}

static void clamp_cursor(int count) {
  if (ctx.cursor < 0) ctx.cursor = 0;
  if (ctx.cursor > count-1) ctx.cursor = count-1;
}

void ui_menu_init() {
  ctx.state = UI_MENU_MAIN;
  ctx.cursor = 0;
}

void ui_menu_update(bool up, bool down, bool left, bool right) {
  int count = 0;
  switch(ctx.state) {
    case UI_MENU_MAIN: count = (int)(sizeof(mainItems)/sizeof(mainItems[0])); break;
    case UI_MENU_MUSIC: count = (int)(sizeof(musicItems)/sizeof(musicItems[0])); break;
    case UI_MENU_TEXT: count = (int)(sizeof(textItems)/sizeof(textItems[0])); break;
    case UI_MENU_GAMES: count = (int)(sizeof(gameItems)/sizeof(gameItems[0])); break;
    case UI_MENU_SETTINGS: count = (int)(sizeof(settingItems)/sizeof(settingItems[0])); break;
  }

  if (up) ctx.cursor--;
  if (down) ctx.cursor++;
  clamp_cursor(count);

  if (left) {
    go_back();
    return;
  }
  if (right) {
    enter_current();
    return;
  }
}

static void enter_current() {
  switch(ctx.state) {
    case UI_MENU_MAIN:
      switch(ctx.cursor) {
        case 0: ctx.state = UI_MENU_MUSIC; ctx.cursor = 0; break;
        case 1: ctx.state = UI_MENU_TEXT; ctx.cursor = 0; break;
        case 2: ctx.state = UI_MENU_GAMES; ctx.cursor = 0; break;
        case 3: ctx.state = UI_MENU_SETTINGS; ctx.cursor = 0; break;
      }
      break;
    case UI_MENU_MUSIC:
      if (ctx.cursor == 2) go_back();
      break;
    case UI_MENU_TEXT:
      if (ctx.cursor == 1) go_back();
      break;
    case UI_MENU_GAMES:
      if (ctx.cursor == 3) go_back();
      break;
    case UI_MENU_SETTINGS:
      if (ctx.cursor == 2) go_back();
      break;
  }
}

static void go_back() {
  if (ctx.state == UI_MENU_MAIN) return;
  ctx.state = UI_MENU_MAIN;
  ctx.cursor = 0;
}

void ui_menu_render() {
  switch(ctx.state) {
    case UI_MENU_MAIN:
      draw_menu(mainItems, (int)(sizeof(mainItems)/sizeof(mainItems[0])));
      break;
    case UI_MENU_MUSIC:
      draw_menu(musicItems, (int)(sizeof(musicItems)/sizeof(musicItems[0])));
      break;
    case UI_MENU_TEXT:
      draw_menu(textItems, (int)(sizeof(textItems)/sizeof(textItems[0])));
      break;
    case UI_MENU_GAMES:
      draw_menu(gameItems, (int)(sizeof(gameItems)/sizeof(gameItems[0])));
      break;
    case UI_MENU_SETTINGS:
      draw_menu(settingItems, (int)(sizeof(settingItems)/sizeof(settingItems[0])));
      break;
  }
}
