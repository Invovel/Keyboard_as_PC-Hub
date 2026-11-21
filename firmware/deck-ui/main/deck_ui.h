#pragma once
#include "body_screen.h"
typedef enum {DECK_HOME,DECK_CLOCK,DECK_METRICS,DECK_SETTINGS,DECK_VOICE} deck_page_t;
typedef enum {ACT_NONE,ACT_VOICE,ACT_PLAY,ACT_CLOCK,ACT_METRICS,ACT_PREV,ACT_NEXT,ACT_MUTE,ACT_HOME,ACT_ALT,ACT_CODEX,ACT_SETTINGS,ACT_CHROME,ACT_SAVE,ACT_THEME} deck_action_t;
void deck_ui_init(const uint8_t map[4],int theme,bool frames);
bool deck_ui_frames(void);
void deck_ui_navigate(deck_page_t page);
void deck_ui_touch(body_touch_t point,bool valid);
void deck_ui_cancel_touch(void);
bool deck_ui_event(int *action);
void deck_ui_key(int key);
void deck_ui_draft(uint8_t map[4]);
void deck_ui_set_mapping(const uint8_t map[4]);
void deck_ui_saved(bool ok);
int deck_ui_theme(void);
void deck_ui_tick(void);
void deck_ui_voice(bool available,bool active,int level);
void deck_ui_metric(int index,float value,const char *unit);
void deck_ui_weather(float temperature,const char *city,const char *condition);

void deck_ui_clock_changed(void);

void deck_ui_palette(uint32_t left,uint32_t right,uint32_t base);
