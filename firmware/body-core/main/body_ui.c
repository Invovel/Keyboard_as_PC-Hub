#include "body_ui.h"
#include "esp_lvgl_port.h"
#include "freertos/queue.h"
#include <stdio.h>

static QueueHandle_t actions;
static lv_obj_t *status, *feedback, *details, *mic_details;
static lv_indev_t *pointer;
static body_touch_t gui_touch;
static void read_pointer(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;
    data->point.x=gui_touch.x; data->point.y=gui_touch.y;
    data->state=gui_touch.down ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}
static void clicked(lv_event_t *event)
{
    int action=(int)(intptr_t)lv_event_get_user_data(event);
    xQueueSend(actions,&action,0);
}
static lv_obj_t *label(lv_obj_t *parent,const char *text,int x,int y,int width)
{
    lv_obj_t *l=lv_label_create(parent);
    lv_label_set_text(l,text); lv_obj_set_pos(l,x,y); lv_obj_set_width(l,width);
    lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0);
    return l;
}
static void button(const char *text,int action,int x,int y,int w,int h)
{
    lv_obj_t *b=lv_button_create(lv_screen_active());
    lv_obj_set_pos(b,x,y); lv_obj_set_size(b,w,h);
    lv_obj_add_event_cb(b,clicked,LV_EVENT_CLICKED,(void*)(intptr_t)action);
    lv_obj_t *l=lv_label_create(b);lv_label_set_text(l,text);lv_obj_center(l);
}
void body_ui_page(body_page_t page)
{
    lvgl_port_lock(0);
    gui_touch.down=false; lv_indev_reset(pointer,NULL);
    lv_obj_t *root=lv_screen_active(); lv_obj_clean(root);
    lv_obj_remove_flag(root,LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(root,lv_color_hex(0x101824),0);
    lv_obj_set_style_text_color(root,lv_color_hex(0xe6edf4),0);
    status=label(root,"USB offline",65,82,230);
    feedback=label(root,"Ready",60,278,240);
    details=NULL; mic_details=NULL;
    if(page==PAGE_HOME) {
        label(root,"PC HUB",100,40,160);
        button("Media",UI_MEDIA,65,119,110,52);
        button("Touchpad",UI_PAD,185,119,110,52);
        button("Settings",UI_SETTINGS,65,185,110,52);
        button("Test key A",UI_TEST_KEY,185,185,110,52);
        button("Mic",UI_MIC,135,243,90,30);
    } else {
        button("Home",UI_HOME,135,35,90,38);
        if(page==PAGE_MEDIA) {
            button("Prev",UI_PREV,65,118,72,48);button("Play",UI_PLAY,144,118,72,48);
            button("Next",UI_NEXT,223,118,72,48);
            button("Vol -",UI_VOL_DOWN,65,184,72,48);button("Mute",UI_MUTE,144,184,72,48);
            button("Vol +",UI_VOL_UP,223,184,72,48);
        } else if(page==PAGE_PAD) {
            lv_obj_t *pad=lv_obj_create(root);lv_obj_set_pos(pad,55,108);lv_obj_set_size(pad,250,160);
            lv_obj_remove_flag(pad,LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(pad,lv_color_hex(0x1c3044),0);
            label(pad,"TOUCHPAD\nMove / tap / double tap\nTap then hold to drag",4,40,210);
        } else if(page==PAGE_MIC) {
            label(root,"INMP441 input",90,116,180);
            button("Start / Stop",UI_MIC_CAPTURE,110,150,140,38);
            mic_details=label(root,"Not fitted / build disabled",55,201,250);
        } else {
            button("Sensitivity",UI_SENS,65,116,110,45);
            button("Sound",UI_SOUND,185,116,110,45);
            button("Output on/off",UI_ENABLE,90,174,180,40);
            details=label(root,"",60,225,240);
        }
    }
    lvgl_port_unlock();
}
void body_ui_mic_status(const char *text)
{
    lvgl_port_lock(0);
    if(mic_details)lv_label_set_text(mic_details,text);
    lvgl_port_unlock();
}
void body_ui_init(void)
{
    actions=xQueueCreate(16,sizeof(int)); ESP_ERROR_CHECK(actions ? ESP_OK : ESP_ERR_NO_MEM);
    lvgl_port_lock(0);
    pointer=lv_indev_create();lv_indev_set_type(pointer,LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(pointer,read_pointer);
    lvgl_port_unlock(); body_ui_page(PAGE_HOME);
}
bool body_ui_action(int *action){return xQueueReceive(actions,action,0)==pdTRUE;}
void body_ui_touch(body_touch_t point,bool to_gui)
{
    lvgl_port_lock(0);gui_touch=point;gui_touch.down=point.down && to_gui;lvgl_port_unlock();
}
void body_ui_status(bool usb,bool enabled,int sensitivity,bool sound,const char *last,
                    body_touch_t point,uint32_t a,uint32_t b,int32_t pcnt)
{
    lvgl_port_lock(0);
    lv_label_set_text_fmt(status,"USB %s | %s",usb ? "connected" : "offline",enabled ? "ON" : "OFF");
    lv_label_set_text(feedback,last);
    if(details) lv_label_set_text_fmt(details,"Speed %d | Sound %s\nX:%u Y:%u %s\nA:%lu B:%lu P:%ld",sensitivity,
        sound ? "ON" : "OFF",point.x,point.y,point.down ? "DOWN" : "UP",(unsigned long)a,(unsigned long)b,(long)pcnt);
    lvgl_port_unlock();
}
