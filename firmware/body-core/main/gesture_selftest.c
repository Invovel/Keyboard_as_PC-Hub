// SPDX-License-Identifier: GPL-3.0-or-later
// Runs the actual ported state machine with synthetic input, never sends USB.
#include "touchpad_gesture.h"
#include "esp_log.h"
static int presses,releases,moves,last_x;
static bool held;
static void move(int8_t x,int8_t y){(void)y;++moves;last_x=x;}
static void button(bool down){held=down;if(down)++presses;else ++releases;}
static const touchpad_output_t output={move,button};
static void start(void){touchpad_gesture_reset(&output);presses=releases=moves=last_x=0;held=false;}
static void sample(bool down,int x,uint32_t ms){touchpad_gesture_sample(down,x,100,ms,&output);}
#define CHECK(condition) do { if(!(condition)){ESP_LOGE("gesture-test","FAIL line %d",__LINE__);touchpad_gesture_reset(&output);return false;} } while(0)
bool touchpad_gesture_selftest(void)
{
    start();sample(true,100,100);sample(false,100,150);
    CHECK(presses==1 && held);sample(false,100,400);CHECK(releases==1 && !held);
    start();sample(true,100,1000);sample(true,100,1400);sample(false,100,1450);
    CHECK(presses==0 && releases==0); // Resting finger is not a mouse press.
    start();sample(true,100,1000);sample(true,130,1020);sample(false,130,1050);
    CHECK(moves==1 && last_x==30 && presses==0);
    start();sample(true,100,1000);sample(false,100,1050);sample(true,100,1100);
    sample(false,100,1150);sample(false,100,1190);
    CHECK(presses==2 && releases==2 && !held);
    start();sample(true,100,1000);sample(false,100,1050);sample(true,100,1100);
    sample(true,150,1300);CHECK(held && presses==1 && releases==0 && moves==1);
    sample(false,150,1500);CHECK(!held && releases==1);
    start();sample(true,100,1000);sample(false,100,1050);sample(true,100,1100);
    touchpad_gesture_reset(&output);sample(false,100,2000);
    CHECK(!held && presses==1 && releases==1);
    start();sample(true,100,1000);sample(false,100,1050);sample(true,100,1100);sample(false,100,1150);
    touchpad_gesture_reset(&output);sample(false,100,2000);CHECK(presses==1 && releases==1);
    start();sample(true,100,UINT32_MAX-100);sample(false,100,UINT32_MAX-50);
    sample(false,100,200);CHECK(presses==1 && releases==1 && !held);
    start();ESP_LOGI("gesture-test","PASS tap, dwell, move, double-tap, drag, reset, pending cancel, clock wrap");
    return true;
}
