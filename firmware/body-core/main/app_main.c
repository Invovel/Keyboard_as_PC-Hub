// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "body_input.h"
#include "body_hid.h"
#include "body_audio.h"
#include "body_screen.h"
#include "body_ui.h"
#include "touchpad_gesture.h"
#define TAG "body-core"
static bool enabled=true, sound=true, mounted;
static int speed=2;
static bool display_ready, touch_ready, inhibit_touch;
static bool gestures_ready;
static body_page_t page=PAGE_HOME;
static body_touch_t point;
static char feedback[96]="Ready";
static nvs_handle_t settings;
static bool settings_ready;

static void move(int8_t x,int8_t y)
{
    int dx=x*speed/2,dy=y*speed/2;
    if(speed==1){if(x && !dx)dx=x>0?1:-1;if(y && !dy)dy=y>0?1:-1;}
    if(dx>127)dx=127;
    if(dx< -127)dx=-127;
    if(dy>127)dy=127;
    if(dy< -127)dy=-127;
    if(enabled && mounted)body_hid_mouse_move(dx,dy);
}
static void mouse_button(bool down)
{
    if(enabled && mounted)body_hid_mouse_button(HID_MOUSE_FROM_TOUCH,down);
}
static const touchpad_output_t touch_output={move,mouse_button};
static void release(void)
{
    touchpad_gesture_reset(&touch_output);
    body_hid_release_all();
    inhibit_touch=true;
    if(display_ready)body_ui_touch((body_touch_t){0},false);
}
static void tone(body_tone_t t){if(sound)body_audio_play(t);}
static void note(const char *text)
{
    snprintf(feedback,sizeof(feedback),"%s",text);ESP_LOGI(TAG,"%s",text);
}
static void toggle(void)
{
    enabled=!enabled;release();note(enabled?"Output enabled":"Output disabled");
    tone(enabled?TONE_CONFIRM:TONE_ERROR);
}
static void settings_load(void)
{
    esp_err_t err=nvs_flash_init();
    if(err==ESP_OK)err=nvs_open("body",NVS_READWRITE,&settings);
    settings_ready=err==ESP_OK;
    if(!settings_ready){ESP_LOGW(TAG,"Settings unavailable: %s; defaults in RAM",esp_err_to_name(err));return;}
    uint8_t value;
    if(nvs_get_u8(settings,"speed",&value)==ESP_OK && value>=1 && value<=3)speed=value;
    if(nvs_get_u8(settings,"sound",&value)==ESP_OK && value<=1)sound=value;
    ESP_LOGI(TAG,"Settings loaded speed=%d sound=%d",speed,sound);
}
static void settings_save(void)
{
    esp_err_t err=settings_ready?nvs_set_u8(settings,"speed",speed):ESP_ERR_INVALID_STATE;
    if(err==ESP_OK)err=nvs_set_u8(settings,"sound",sound);
    if(err==ESP_OK)err=nvs_commit(settings);
    char message[80];snprintf(message,sizeof(message),"Speed %d, sound %s: %s",speed,sound?"on":"off",
        err==ESP_OK?"saved":"NOT saved");note(message);
}
static void consumer(uint16_t usage,const char *name)
{
    char message[80];snprintf(message,sizeof(message),"%s: %s",name,
        !enabled?"disabled":!mounted?"USB offline":"queued");note(message);
    if(enabled && mounted)body_hid_consumer(usage);
    tone(TONE_CONFIRM);
}
static void action(int id)
{
    if(id>=UI_HOME && id<=UI_SETTINGS){
        release();page=(body_page_t)(id-UI_HOME);
        if(display_ready)body_ui_page(page);
        note(page==PAGE_PAD?"Touchpad ready":"Page changed");tone(TONE_CONFIRM);return;
    }
    static const uint16_t media[]={0xB6,0xCD,0xB5,0xE2,0xEA,0xE9};
    static const char *names[]={"Previous","Play/pause","Next","Mute","Volume -","Volume +"};
    if(id>=UI_PREV && id<=UI_VOL_UP){consumer(media[id-UI_PREV],names[id-UI_PREV]);return;}
    if(id==UI_SENS){speed=speed%3+1;settings_save();tone(TONE_CONFIRM);}
    else if(id==UI_SOUND){sound=!sound;settings_save();tone(TONE_CONFIRM);}
    else if(id==UI_ENABLE)toggle();
    else if(id==UI_TEST_KEY){
        if(enabled && mounted){body_hid_key(0,0x04);note("Keyboard A queued");}
        else note("Keyboard unavailable");
        tone(TONE_CONFIRM);
    }
}
void app_main(void)
{
    gestures_ready=touchpad_gesture_selftest();
    settings_load();body_audio_init();body_hid_init();body_input_init();
    tone(TONE_START);
    display_ready=body_screen_init()==ESP_OK;
    touch_ready=body_touch_init()==ESP_OK;
    if(display_ready)body_ui_init();
    ESP_LOGI(TAG,"Integration ready display=%d touch=%d",display_ready,touch_ready);
    gpio_config_t cfg={.pin_bit_mask=1ULL,.mode=GPIO_MODE_INPUT,.pull_up_en=GPIO_PULLUP_ENABLE};
    ESP_ERROR_CHECK(gpio_config(&cfg));
    bool boot_raw=!gpio_get_level(0),boot_stable=boot_raw,was_down=false;
    enum { CONTACT_NONE, CONTACT_GUI, CONTACT_PAD } owner=CONTACT_NONE;
    int64_t boot_changed=esp_timer_get_time(),last_stats=boot_changed,last_ui=0,last_touch=0;
    uint32_t dropped=0,touch_errors=0,touch_samples=0;
    while(true){
        int64_t now=esp_timer_get_time();
        bool boot=!gpio_get_level(0);
        if(boot!=boot_raw){boot_raw=boot;boot_changed=now;}
        if(boot!=boot_stable && now-boot_changed>=30000){boot_stable=boot;if(boot)toggle();}
        bool usb=body_hid_mounted();
        if(usb!=mounted){mounted=usb;release();note(usb?"USB connected":"USB disconnected");}
        body_hid_tick();
        int id;
        if(display_ready)while(body_ui_action(&id))action(id);
        body_input_stats_t stats=body_input_stats();
        if(stats.dropped!=dropped){dropped=stats.dropped;enabled=false;release();note("Input overflow: disabled");}
        body_input_event_t event;
        while(body_input_read(&event)){
            if(event.type==BODY_INPUT_KEY){
                ESP_LOGI(TAG,"Key %d %s",event.index,event.down?"DOWN":"UP");
                if(event.down)action(UI_PREV+event.index-1);
            }else if(event.type==BODY_INPUT_PRESS){
                ESP_LOGI(TAG,"Press %s",event.down?"DOWN":"UP");
                if(page==PAGE_PAD){
                    if(enabled && mounted)body_hid_mouse_button(HID_MOUSE_FROM_PRESS,event.down);
                    if(event.down){tone(TONE_CONFIRM);note("Press: left mouse");}
                }else if(event.down)action(UI_PLAY);
            }else{
                ESP_LOGI(TAG,"Encoder full cycle %+d",event.index);
                if(page==PAGE_PAD){
                    if(enabled && mounted)body_hid_mouse_wheel(event.index);
                    note("Encoder: scroll");
                }else action(event.index>0?UI_VOL_UP:UI_VOL_DOWN);
            }
        }
        if(touch_ready && now-last_touch>=10000){
            last_touch=now;body_touch_t next;
            esp_err_t err=body_touch_read(&next);
            if(err!=ESP_OK){
                ++touch_errors;
                if(was_down || !inhibit_touch){release();note("Touch read error: released");}
                owner=CONTACT_NONE;was_down=false;point.down=false;
            }else{
                if(next.down)++touch_samples;
                if(next.down!=was_down)ESP_LOGI(TAG,"Touch %s x=%u y=%u",next.down?"DOWN":"UP",next.x,next.y);
                point=next;
                if(!point.down)inhibit_touch=false;
                if(point.down && !was_down && !inhibit_touch){
                    owner=(page==PAGE_PAD && point.x>=55 && point.x<305 && point.y>=108 && point.y<268)
                        ? CONTACT_PAD:CONTACT_GUI;
                }
                if(display_ready)body_ui_touch(point,owner==CONTACT_GUI && !inhibit_touch);
                if(page==PAGE_PAD && gestures_ready && enabled && mounted && !inhibit_touch)
                    touchpad_gesture_sample(point.down && owner==CONTACT_PAD,point.x,point.y,
                                            (uint32_t)(now/1000),&touch_output);
                was_down=point.down;if(!point.down)owner=CONTACT_NONE;
            }
        }
        if(display_ready && now-last_ui>=100000){
            last_ui=now;body_ui_status(mounted,enabled,speed,sound,feedback,point,stats.a_edges,stats.b_edges,stats.pcnt_count);
        }
        if(now-last_stats>=5000000){
            last_stats=now;ESP_LOGI(TAG,"Health A=%"PRIu32" B=%"PRIu32" invalid=%"PRIu32
                " drop=%"PRIu32" touch=%"PRIu32" errors=%"PRIu32" PCNT=%"PRId32" USB=%d enabled=%d heap=%lu",
                stats.a_edges,stats.b_edges,stats.invalid,stats.dropped,touch_samples,touch_errors,
                stats.pcnt_count,mounted,enabled,(unsigned long)esp_get_free_heap_size());
        }
        vTaskDelay(1);
    }
}
