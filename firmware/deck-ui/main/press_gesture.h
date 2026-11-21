// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef enum { PRESS_NONE, PRESS_TAP, PRESS_HOLD } press_result_t;
typedef struct { bool down, fired; uint32_t started; } press_gesture_t;
static inline press_result_t press_gesture_edge(press_gesture_t *s,bool down,uint32_t now)
{
    if(down==s->down)return PRESS_NONE;
    s->down=down;
    if(down){s->started=now;s->fired=false;return PRESS_NONE;}
    if(s->fired)return PRESS_NONE;
    s->fired=true;
    return (uint32_t)(now-s->started)>=700 ? PRESS_HOLD : PRESS_TAP;
}
static inline press_result_t press_gesture_tick(press_gesture_t *s,uint32_t now)
{
    if(!s->down || s->fired || (uint32_t)(now-s->started)<700)return PRESS_NONE;
    s->fired=true;return PRESS_HOLD;
}
static inline bool press_gesture_selftest(void)
{
    press_gesture_t s={0};
    if(press_gesture_edge(&s,false,0)!=PRESS_NONE)return false;
    if(press_gesture_edge(&s,true,10)!=PRESS_NONE)return false;
    if(press_gesture_edge(&s,true,20)!=PRESS_NONE)return false;
    if(press_gesture_tick(&s,709)!=PRESS_NONE)return false;
    if(press_gesture_edge(&s,false,709)!=PRESS_TAP)return false;
    if(press_gesture_edge(&s,false,710)!=PRESS_NONE)return false;
    press_gesture_edge(&s,true,1000);
    if(press_gesture_tick(&s,1700)!=PRESS_HOLD)return false;
    if(press_gesture_tick(&s,1800)!=PRESS_NONE)return false;
    if(press_gesture_edge(&s,false,2000)!=PRESS_NONE)return false;
    press_gesture_edge(&s,true,3000);
    if(press_gesture_edge(&s,false,3700)!=PRESS_HOLD)return false;
    press_gesture_edge(&s,true,UINT32_MAX-300);
    if(press_gesture_tick(&s,399)!=PRESS_HOLD)return false;
    if(press_gesture_edge(&s,false,400)!=PRESS_NONE)return false;
    s=(press_gesture_t){0};
    return press_gesture_edge(&s,false,500)==PRESS_NONE;
}
