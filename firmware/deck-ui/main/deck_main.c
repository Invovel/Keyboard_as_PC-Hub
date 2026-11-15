// SPDX-License-Identifier: GPL-3.0-or-later
#include "deck_ui.h"
#include "body_input.h"
#include "body_hid.h"
#include "body_audio.h"
#include "body_mic.h"
#include "press_gesture.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
static const char *TAG="deck-main";
static nvs_handle_t settings;
static bool storage,enabled=true,usb;
static uint8_t mapping[4];
static press_gesture_t center;
static uint32_t dropped;
static int bench_step=-1;
static int64_t bench_next;
static unsigned pc_action_seq;
static const deck_page_t bench_pages[]={DECK_CLOCK,DECK_HOME,DECK_METRICS,DECK_HOME,DECK_SETTINGS,DECK_HOME,DECK_VOICE,DECK_HOME};
static void handle(int action){
 if(action==ACT_SAVE){uint8_t next[4];deck_ui_draft(next);esp_err_t err=storage?nvs_set_blob(settings,"mapping",next,4):ESP_ERR_INVALID_STATE;if(err==ESP_OK)err=nvs_commit(settings);if(err==ESP_OK)memcpy(mapping,next,4);deck_ui_saved(err==ESP_OK);ESP_LOGI(TAG,"mapping save %s",esp_err_to_name(err));return;}
 if(action==ACT_THEME){if(storage){nvs_set_u8(settings,"theme",deck_ui_theme());nvs_set_u8(settings,"frames",deck_ui_frames());nvs_commit(settings);}return;}
 if(action==ACT_CLOCK){deck_ui_navigate(DECK_CLOCK);return;}
 if(action==ACT_METRICS){deck_ui_navigate(DECK_METRICS);return;}
 if(action==ACT_SETTINGS){deck_ui_navigate(DECK_SETTINGS);ESP_LOGI(TAG,"button action SETTINGS");return;}
 if(action==ACT_HOME){deck_ui_navigate(DECK_HOME);return;}
 if(action==ACT_VOICE){deck_ui_navigate(DECK_VOICE);ESP_LOGI(TAG,"voice entry only; capture not started");return;}
 if(action==ACT_ALT){if(enabled&&usb)body_hid_key(0x04,0);ESP_LOGI(TAG,"button action ALT %s",enabled&&usb?"queued":"unavailable");return;}
 if(action==ACT_CODEX||action==ACT_CHROME){ESP_LOGI(TAG,"PC_ACTION %s %u",action==ACT_CODEX?"CODEX":"CHROME",++pc_action_seq);return;}
 uint16_t usage=action==ACT_PLAY?0xcd:action==ACT_PREV?0xb6:action==ACT_NEXT?0xb5:action==ACT_MUTE?0xe2:0;
 if(usage){if(enabled&&usb)body_hid_consumer(usage);ESP_LOGI(TAG,"media action=%d %s",action,enabled&&usb?"queued":"unavailable");}
}
static void packet(const char *s){
 char tail;
 if(strcmp(s,"B")==0){bench_step=0;bench_next=esp_timer_get_time();ESP_LOGI(TAG,"display benchmark begin");return;}
 int k1,k2,k3,k4;
 if(sscanf(s,"K %d %d %d %d %c",&k1,&k2,&k3,&k4,&tail)==4&&k1>=0&&k1<=ACT_CHROME&&k2>=0&&k2<=ACT_CHROME&&k3>=0&&k3<=ACT_CHROME&&k4>=0&&k4<=ACT_CHROME){uint8_t next[4]={(uint8_t)k1,(uint8_t)k2,(uint8_t)k3,(uint8_t)k4};memcpy(mapping,next,4);deck_ui_set_mapping(next);esp_err_t err=storage?nvs_set_blob(settings,"mapping",next,4):ESP_ERR_INVALID_STATE;if(err==ESP_OK)err=nvs_commit(settings);ESP_LOGI(TAG,"mapping configured %d/%d/%d/%d %s",k1,k2,k3,k4,esp_err_to_name(err));return;}
 unsigned left,right,base;
 if(sscanf(s,"P %x %x %x %c",&left,&right,&base,&tail)==3&&left<=0xffffff&&right<=0xffffff&&base<=0xffffff){deck_ui_palette(left,right,base);if(storage){uint32_t colors[3]={left,right,base};nvs_set_blob(settings,"palette",colors,sizeof(colors));nvs_set_u8(settings,"theme",3);nvs_commit(settings);}ESP_LOGI(TAG,"custom palette applied");return;}
 float temp,humidity,wind;int code;long long observed;
 if(sscanf(s,"W %f %f %f %d %lld %c",&temp,&humidity,&wind,&code,&observed,&tail)==5&&isfinite(temp)&&isfinite(humidity)&&isfinite(wind)&&temp>-100&&temp<100&&humidity>=0&&humidity<=100&&wind>=0&&wind<200&&code>=0&&code<100){
   time_t now=time(NULL);if(now>1700000000&&llabs((long long)now-observed)<1800){const char *weather=code==0?"晴":code<3?"多云":code==3?"阴":code==45||code==48?"雾":code>=71&&code<=77?"雪":code>=95?"雷雨":"雨";char description[72];snprintf(description,sizeof(description),"%s  湿度%.0f%%  风%.1fm/s",weather,(double)humidity,(double)wind);deck_ui_weather(temp,"西安",description);ESP_LOGI(TAG,"Xian weather accepted temp=%.1f code=%d observed=%lld",(double)temp,code,observed);}return;
 }
 long long epoch;int offset,index;float value;char unit[12],extra;
 if(sscanf(s,"T %lld %d %c",&epoch,&offset,&extra)==2&&epoch>1700000000&&epoch<4102444800LL&&offset>=-840&&offset<=840){
  struct timeval tv={.tv_sec=epoch};settimeofday(&tv,NULL);char zone[32];int inverse=-offset;snprintf(zone,sizeof(zone),"UTC%c%d:%02d",inverse<0?'-':'+',abs(inverse)/60,abs(inverse)%60);setenv("TZ",zone,1);tzset();deck_ui_clock_changed();ESP_LOGI(TAG,"clock synchronized");
 }else if(sscanf(s,"M %d %f %11s %c",&index,&value,unit,&extra)==3&&index>=0&&index<4&&isfinite(value)&&value>=0&&value<1e10){deck_ui_metric(index,value,unit);static unsigned mask;unsigned bit=1u<<index;if(!(mask&bit)){mask|=bit;ESP_LOGI(TAG,"metric index=%d accepted %.2f%s",index,(double)value,unit);}}
}
void app_main(void){
 ESP_ERROR_CHECK(press_gesture_selftest()?ESP_OK:ESP_FAIL);
 esp_err_t err=nvs_flash_init();if(err==ESP_OK)err=nvs_open("deck_v4",NVS_READWRITE,&settings);storage=err==ESP_OK;
 uint8_t theme=0;if(storage){size_t size=4;uint8_t stored[4];if(nvs_get_blob(settings,"mapping",stored,&size)==ESP_OK&&size==4){bool valid=true;for(int i=0;i<4;i++)if(stored[i]>ACT_CHROME)valid=false;if(valid)memcpy(mapping,stored,4);}if(nvs_get_u8(settings,"theme",&theme)!=ESP_OK||theme>3)theme=0;}
 uint8_t frames=0;if(storage)nvs_get_u8(settings,"frames",&frames);
 body_hid_init();body_input_init();body_audio_init();err=body_mic_init();if(err!=ESP_OK&&err!=ESP_ERR_NOT_SUPPORTED)ESP_LOGW(TAG,"mic unavailable: %s",esp_err_to_name(err));
 ESP_ERROR_CHECK(body_screen_init());ESP_ERROR_CHECK(body_touch_init());deck_ui_init(mapping,theme,frames!=0);if(storage&&theme==3){uint32_t colors[3];size_t size=sizeof(colors);if(nvs_get_blob(settings,"palette",colors,&size)==ESP_OK&&size==sizeof(colors))deck_ui_palette(colors[0],colors[1],colors[2]);}
 if(!uart_is_driver_installed(UART_NUM_0))ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0,2048,0,0,NULL,0));
 gpio_config_t bootcfg={.pin_bit_mask=1,.mode=GPIO_MODE_INPUT,.pull_up_en=GPIO_PULLUP_ENABLE};ESP_ERROR_CHECK(gpio_config(&bootcfg));
 ESP_LOGI(TAG,"V4 independent system ready; PSRAM free=%u; default keys unbound",(unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
 int64_t last_touch=0,last_ui=0,last_health=0,boot_change=0;bool boot_raw=false,boot_stable=false,touch_block=false;
 char line[160];unsigned at=0;bool discard=false;
 while(true){
  int64_t now=esp_timer_get_time();bool mounted=body_hid_mounted();if(mounted!=usb){usb=mounted;body_hid_release_all();center=(press_gesture_t){0};deck_ui_cancel_touch();touch_block=true;ESP_LOGI(TAG,"USB %s",usb?"connected":"offline");}body_hid_tick();
  bool boot=!gpio_get_level(0);if(boot!=boot_raw){boot_raw=boot;boot_change=now;}if(boot!=boot_stable&&now-boot_change>=30000){boot_stable=boot;if(boot){enabled=!enabled;body_hid_release_all();center=(press_gesture_t){0};if(!enabled)body_mic_set_capture(false);}}
  body_input_stats_t stats=body_input_stats();if(stats.dropped!=dropped){dropped=stats.dropped;enabled=false;body_hid_release_all();center=(press_gesture_t){0};ESP_LOGE(TAG,"input overflow: outputs disabled");}
  body_input_event_t input;while(body_input_read(&input)){
   if(input.type==BODY_INPUT_KEY){if(input.down){ESP_LOGI(TAG,"key %d",input.index);deck_ui_key(input.index);}}
   else if(input.type==BODY_INPUT_PRESS){press_result_t result=press_gesture_edge(&center,input.down,now/1000);if(result==PRESS_TAP)handle(ACT_PLAY);if(result==PRESS_HOLD)handle(ACT_MUTE);}
   else {if(enabled&&usb)body_hid_consumer(input.index>0?0xe9:0xea);ESP_LOGI(TAG,"volume %+d %s",input.index,enabled&&usb?"queued":"unavailable");}
  }
  if(press_gesture_tick(&center,now/1000)==PRESS_HOLD)handle(ACT_MUTE);
  int action;while(deck_ui_event(&action))handle(action);
  if(now-last_touch>=10000){last_touch=now;body_touch_t point;esp_err_t toucherr=body_touch_read(&point);if(toucherr!=ESP_OK){deck_ui_cancel_touch();touch_block=true;}else{if(!point.down)touch_block=false;deck_ui_touch(point,!touch_block);}}
  if(now-last_ui>=33000){last_ui=now;body_mic_status_t m=body_mic_status();deck_ui_voice(m.configured,m.capturing,m.capturing?(uint32_t)m.peak*100/32768:0);}
  if(now-last_health>=1000000){last_health=now;deck_ui_tick();}
  if(bench_step>=0&&now>=bench_next){if(bench_step<8){deck_ui_navigate(bench_pages[bench_step++]);bench_next=now+1300000;}else{bench_step=-1;ESP_LOGI(TAG,"display benchmark complete");}}
  uint8_t c;while(uart_read_bytes(UART_NUM_0,&c,1,0)==1){if(c=='\n'){if(!discard){line[at]=0;packet(line);}at=0;discard=false;}else if(c!='\r'){if(at<sizeof(line)-1&&!discard)line[at++]=c;else discard=true;}}
  vTaskDelay(pdMS_TO_TICKS(2));
 }
}
