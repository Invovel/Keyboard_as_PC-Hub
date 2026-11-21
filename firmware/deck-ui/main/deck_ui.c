// SPDX-License-Identifier: GPL-3.0-or-later
#include "deck_ui.h"
#include "esp_lvgl_port.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "lvgl.h"
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
LV_FONT_DECLARE(deck_font);
static const char *TAG="deck-v4";
static const char *names[]={"未绑定","语音输入","播放暂停","时间天气","电脑状态","上一曲","下一曲","输出静音","返回主界面","ALT","打开 CODEX","打开设置","打开 CHROME"};
static const char *icons[]={"",LV_SYMBOL_AUDIO,LV_SYMBOL_PLAY,LV_SYMBOL_IMAGE,LV_SYMBOL_SETTINGS,LV_SYMBOL_PREV,LV_SYMBOL_NEXT,LV_SYMBOL_MUTE,LV_SYMBOL_HOME,"ALT","CODEX","设置","CHROME"};
static const char *metric_names[]={"CPU","GPU","RAM","NET"};
static struct {float value;char unit[12];int64_t at;} metrics[4];
static uint8_t mapping[4],draft[4];
static int selected,theme_mode,theme_index,metric_detail=-1;
static bool custom_available;
static uint32_t custom_colors[3];
static deck_page_t current=DECK_HOME,pending;
static QueueHandle_t events;
static lv_indev_t *pointer;
static body_touch_t touch;
static lv_obj_t *content,*plane,*clock_label,*date_label,*weather_label,*temperature_label,*city_label,*metric_labels[4],*bars[4],*voice_label,*wave_line,*status_label;
static lv_obj_t *overlay;
static lv_draw_buf_t *overlay_images[4];
static bool overlay_visible=true;
static bool frames_visible;
static lv_point_precise_t wave_points[51];
static lv_draw_buf_t *home_image,*motion_image,*glass_image;
static bool moving,voice_available,voice_active;
static int64_t anim_start,first_frame,last_frame;
static unsigned rendered_frames;
static unsigned overlay_leaks;
static bool contact,consumed,rejected;
static int start_x,start_y,last_x,last_y,samples;
static int64_t touch_start;
static uint32_t palette_left,palette_right,palette_base,palette_ink;
static int64_t weather_at;
static float temperature;
static char city[40]="西安",condition[72];
static void build(deck_page_t page);
static void overlay_check(deck_page_t page);
static void overlay_toggle(void);
static void overlay_refresh(deck_page_t page);
static void enqueue(int action){if(xQueueSend(events,&action,0)!=pdTRUE)ESP_LOGW(TAG,"action queue full");}
static lv_color_t dim(uint32_t hex){return lv_color_make(((hex>>16)&255)*3/4,((hex>>8)&255)*3/4,(hex&255)*3/4);}
static uint16_t pixel(uint32_t c){unsigned r=((c>>16)&255)*3/4,g=((c>>8)&255)*3/4,b=(c&255)*3/4;return ((r>>3)<<11)|((g>>2)<<5)|(b>>3);}
static void theme_colors(void){
 time_t now=time(NULL);struct tm t;int hour=12;if(now>1700000000&&localtime_r(&now,&t))hour=t.tm_hour;
 theme_index=theme_mode==2?1:theme_mode==1?0:(hour<7||hour>=20);
 palette_left=theme_index?0x587884:0xbfd4d9;palette_right=theme_index?0x334e62:0x899fad;
 palette_base=0x1c2833;palette_ink=theme_index?0xd7e5ec:0x254253;
 if(theme_mode==3&&custom_available){palette_left=custom_colors[0];palette_right=custom_colors[1];palette_base=custom_colors[2];unsigned brightness=(((palette_left>>16)&255)*299+((palette_left>>8)&255)*587+(palette_left&255)*114)/1000;palette_ink=brightness>140?0x254253:0xd7e5ec;}
}
static void background_image(void){
 if(!home_image)home_image=lv_draw_buf_create(360,360,LV_COLOR_FORMAT_RGB565,LV_STRIDE_AUTO);
 if(!motion_image)motion_image=lv_draw_buf_create(1880,360,LV_COLOR_FORMAT_RGB565,LV_STRIDE_AUTO);
 if(!home_image||!motion_image){ESP_LOGE(TAG,"ellipse raster allocation failed");return;}
 // Two ellipses in one space: centers (-580,180)/(940,180), radii (760,187).
 // Home is the central 360px crop; a 760px pan centers either ellipse.
 for(int y=0;y<360;y++){
  float dy=(y-180)/187.0f,edge=760*sqrtf(1-dy*dy);
  uint16_t *wide=(uint16_t*)(motion_image->data+y*motion_image->header.stride);
  for(int x=0;x<1880;x++){int world=x-760;wide[x]=pixel(world<=-580+edge?palette_left:world>=940-edge?palette_right:palette_base);}
  memcpy(home_image->data+y*home_image->header.stride,wide+760,360*2);
 }

}
static lv_obj_t *label(const char *s,int x,int y,int width,int size,uint32_t color){
 lv_obj_t *o=lv_label_create(content);lv_label_set_text(o,s);lv_obj_set_pos(o,x,y);lv_obj_set_width(o,width);lv_obj_set_style_text_align(o,LV_TEXT_ALIGN_CENTER,0);lv_obj_set_style_text_color(o,dim(color),0);
 lv_obj_set_style_text_font(o,size==48?&lv_font_montserrat_48:size==32?&lv_font_montserrat_32:size==20?&lv_font_montserrat_20:&deck_font,0);return o;
}
static void local_click(lv_event_t *e){
 int id=(int)(intptr_t)lv_event_get_user_data(e);
 if(moving)return;
 if(id>=100&&id<104){selected=id-100;build(current);}
 else if(id==110){draft[selected]=(draft[selected]+1)%(ACT_CHROME+1);build(current);}
 else if(id==111)enqueue(ACT_SAVE);
 else if(id==112){theme_mode=(theme_mode+1)%(custom_available?4:3);theme_colors();background_image();build(current);enqueue(ACT_THEME);}
 else if(id==113)overlay_toggle();
 else if(id==114){frames_visible=!frames_visible;overlay_refresh(current);enqueue(ACT_THEME);}
 else if(id>=120&&id<124){metric_detail=id-120;build(current);}
 else if(id==124){metric_detail=-1;build(current);}
}
static lv_obj_t *button(const char *s,int id,int x,int y,int width,int height){
 lv_obj_t *o=lv_button_create(content);lv_obj_set_pos(o,x,y);lv_obj_set_size(o,width,height);lv_obj_set_style_radius(o,7,0);lv_obj_set_style_bg_color(o,dim(0x172330),0);lv_obj_set_style_shadow_width(o,0,0);lv_obj_set_style_pad_all(o,0,0);lv_obj_add_event_cb(o,local_click,LV_EVENT_CLICKED,(void*)(intptr_t)id);
 lv_obj_t *l=lv_label_create(o);lv_label_set_text(l,s);lv_obj_set_style_text_font(l,&deck_font,0);lv_obj_set_style_text_color(l,dim(0xc6d7e3),0);lv_obj_center(l);return o;
}
static void image(lv_draw_buf_t *buffer){if(!buffer)return;lv_obj_t *o=lv_image_create(content);lv_image_set_src(o,buffer);lv_obj_set_pos(o,0,0);lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE);}
static void clear_content(void){
 if(content)lv_obj_delete(content);
 content=lv_obj_create(lv_screen_active());lv_obj_remove_style_all(content);lv_obj_set_pos(content,0,0);lv_obj_set_size(content,360,360);lv_obj_remove_flag(content,LV_OBJ_FLAG_SCROLLABLE);
 temperature_label=city_label=clock_label=date_label=weather_label=voice_label=wave_line=status_label=NULL;memset(metric_labels,0,sizeof(metric_labels));memset(bars,0,sizeof(bars));
}
static uint32_t page_color(deck_page_t p){return p==DECK_CLOCK?palette_left:p==DECK_METRICS?palette_right:p==DECK_SETTINGS?0x080c11:palette_base;}
static void overlay_line(lv_obj_t *parent,const lv_point_precise_t *points,int count,lv_color_t color){
 lv_obj_t *o=lv_line_create(parent);lv_line_set_points(o,points,count);lv_obj_set_style_line_color(o,color,0);lv_obj_set_style_line_width(o,2,0);lv_obj_set_style_line_rounded(o,true,0);lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
}
static void overlay_refresh(deck_page_t page){
 static const uint8_t at_position[]={0,3,1,2};
 static const lv_point_precise_t mic_stand[]={{13,21},{13,25},{16,30},{24,32},{32,30},{35,25},{35,21}};
 static const lv_point_precise_t mic_base[]={{24,32},{24,39},{17,39},{31,39}};
 static const lv_point_precise_t code_left[]={{17,12},{7,24},{17,36}},code_right[]={{31,12},{41,24},{31,36}};
 static const lv_point_precise_t globe_x[]={{7,24},{41,24}},globe_y[]={{24,7},{24,41}};
 if(!overlay){overlay=lv_obj_create(lv_layer_top());lv_obj_remove_style_all(overlay);lv_obj_set_size(overlay,360,360);lv_obj_set_pos(overlay,0,0);lv_obj_remove_flag(overlay,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);}
 lv_obj_clean(overlay);
 for(int k=0;k<4;k++)if(overlay_images[k]){lv_draw_buf_destroy(overlay_images[k]);overlay_images[k]=NULL;}
 lv_color_t ink=dim(page==DECK_VOICE||page==DECK_SETTINGS?0xd7e5ec:palette_ink);
 for(int k=0;k<4;k++){
  int action=mapping[at_position[k]];
  // The four 96x79 content canvases exist even when unbound or borderless.
  lv_obj_t *canvas=lv_obj_create(overlay);lv_obj_remove_style_all(canvas);lv_obj_set_pos(canvas,32+(k%2)*200,82+(k/2)*113);lv_obj_set_size(canvas,96,79);lv_obj_remove_flag(canvas,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
  if(frames_visible){lv_obj_set_style_border_width(canvas,1,0);lv_obj_set_style_border_color(canvas,ink,0);}
  if(!action)continue;
  lv_obj_t *slot=lv_obj_create(canvas);lv_obj_remove_style_all(slot);lv_obj_set_pos(slot,24,16);lv_obj_set_size(slot,48,48);lv_obj_remove_flag(slot,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
  if(action==ACT_VOICE){
   lv_obj_t *capsule=lv_obj_create(slot);lv_obj_remove_style_all(capsule);lv_obj_set_pos(capsule,19,7);lv_obj_set_size(capsule,10,21);lv_obj_set_style_radius(capsule,5,0);lv_obj_set_style_border_width(capsule,2,0);lv_obj_set_style_border_color(capsule,ink,0);lv_obj_remove_flag(capsule,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
   overlay_line(slot,mic_stand,7,ink);overlay_line(slot,mic_base,4,ink);
  }else if(action==ACT_CODEX){overlay_line(slot,code_left,3,ink);overlay_line(slot,code_right,3,ink);
  }else if(action==ACT_CHROME){
   lv_obj_t *circle=lv_obj_create(slot);lv_obj_remove_style_all(circle);lv_obj_set_pos(circle,7,7);lv_obj_set_size(circle,34,34);lv_obj_set_style_radius(circle,17,0);lv_obj_set_style_border_width(circle,2,0);lv_obj_set_style_border_color(circle,ink,0);lv_obj_remove_flag(circle,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);overlay_line(slot,globe_x,2,ink);overlay_line(slot,globe_y,2,ink);
  }else{
   lv_obj_t *l=lv_label_create(slot);lv_label_set_text(l,action==ACT_SETTINGS?LV_SYMBOL_SETTINGS:icons[action]);lv_obj_set_style_text_font(l,&lv_font_montserrat_20,0);lv_obj_set_style_text_color(l,ink,0);lv_obj_center(l);lv_obj_remove_flag(l,LV_OBJ_FLAG_CLICKABLE);
  }
  // Rasterize each 48px icon once; transitions only blend four tiny cached images.
  overlay_images[k]=lv_snapshot_take(slot,LV_COLOR_FORMAT_A8);
  if(overlay_images[k]){lv_obj_delete(slot);lv_obj_t *cached=lv_image_create(canvas);lv_image_set_src(cached,overlay_images[k]);lv_obj_set_style_image_recolor(cached,ink,0);lv_obj_set_pos(cached,24,16);lv_obj_remove_flag(cached,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);}
 }
 if(overlay_visible&&page==DECK_HOME&&!moving)lv_obj_remove_flag(overlay,LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(overlay,LV_OBJ_FLAG_HIDDEN);
 overlay_check(page);
}
static void overlay_check(deck_page_t page){
 bool top=overlay&&lv_obj_get_parent(overlay)==lv_layer_top();
 bool passthrough=top&&!lv_obj_has_flag(overlay,LV_OBJ_FLAG_CLICKABLE);
 ESP_LOGI(TAG,"floating layer page=%d top=%d passthrough=%d visible=%d slots=%u frames=%d",page,top,passthrough,!lv_obj_has_flag(overlay,LV_OBJ_FLAG_HIDDEN),(unsigned)lv_obj_get_child_count(overlay),frames_visible);
}
static void overlay_toggle(void){
 overlay_visible=!overlay_visible;
 if(overlay_visible&&current==DECK_HOME&&!moving)lv_obj_remove_flag(overlay,LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(overlay,LV_OBJ_FLAG_HIDDEN);
 ESP_LOGI(TAG,"floating icons %s",overlay_visible?"visible":"hidden");
}
static void home_tap(lv_event_t *e){(void)e;if(!moving&&current==DECK_HOME)overlay_toggle();}
static void build(deck_page_t page){
 current=page;clear_content();lv_obj_set_style_bg_color(lv_screen_active(),dim(page_color(page)),0);
 if(page!=DECK_VOICE&&glass_image){lv_draw_buf_destroy(glass_image);glass_image=NULL;}
 if(page==DECK_HOME){image(home_image);lv_obj_add_flag(content,LV_OBJ_FLAG_CLICKABLE);lv_obj_add_event_cb(content,home_tap,LV_EVENT_SHORT_CLICKED,NULL);}
 else if(page==DECK_CLOCK){clock_label=label("--:--",60,63,240,48,palette_ink);date_label=label("时间待更新",60,132,240,14,palette_ink);temperature_label=label("--°",60,182,240,32,palette_ink);city_label=label(city[0]?city:"未设置地点",60,232,240,14,palette_ink);weather_label=label("天气待更新",60,265,240,14,palette_ink);}
 else if(page==DECK_METRICS){
  if(metric_detail>=0){int i=metric_detail;label(metric_names[i],60,106,240,20,palette_ink);metric_labels[i]=label("--",40,151,280,48,palette_ink);status_label=label("等待电脑数据",60,224,240,14,palette_ink);lv_obj_t *hit=button("",124,35,70,290,210);lv_obj_set_style_bg_opa(hit,LV_OPA_TRANSP,0);}
  else for(int k=0;k<4;k++){int y=68+k*56;label(metric_names[k],53,y+4,45,14,palette_ink);metric_labels[k]=label("--",170,y,137,20,palette_ink);lv_obj_set_style_text_align(metric_labels[k],LV_TEXT_ALIGN_RIGHT,0);bars[k]=lv_bar_create(content);lv_obj_set_pos(bars[k],53,y+39);lv_obj_set_size(bars[k],254,2);lv_bar_set_range(bars[k],0,100);lv_obj_set_style_bg_color(bars[k],dim(palette_ink),LV_PART_INDICATOR);lv_obj_set_style_bg_opa(bars[k],LV_OPA_30,LV_PART_MAIN);lv_obj_t *hit=button("",120+k,50,y-3,260,48);lv_obj_set_style_bg_opa(hit,LV_OPA_TRANSP,0);}
 }
 else if(page==DECK_SETTINGS){
  label("显示与快捷配置",65,63,230,14,0x8ea5b8);
  label("主题",65,106,60,14,0xc6d7e3);button(theme_mode==0?"自动":theme_mode==1?"雾青":theme_mode==2?"夜色":"上传主题",112,164,98,125,34);
  button("图标显示",113,65,143,110,32);button("边框显示",114,185,143,110,32);
  for(int k=0;k<4;k++){lv_obj_t *b=button(mapping[k]?icons[mapping[k]]:"",100+k,65+k*59,190,53,30);if(k==selected){lv_obj_set_style_border_width(b,1,0);lv_obj_set_style_border_color(b,dim(0xa0b8c9),0);}}
  button(names[draft[selected]],110,65,231,164,36);button("保存",111,235,231,58,36);status_label=label("",65,281,230,14,0x8299ac);
 }
 else if(page==DECK_VOICE){
  image(glass_image);lv_obj_t *glass=lv_obj_create(content);lv_obj_remove_style_all(glass);lv_obj_set_size(glass,360,360);lv_obj_set_style_bg_color(glass,dim(0x111820),0);lv_obj_set_style_bg_opa(glass,217,0);lv_obj_remove_flag(glass,LV_OBJ_FLAG_CLICKABLE);
  wave_line=lv_line_create(content);for(int i=0;i<51;i++)wave_points[i]=(lv_point_precise_t){65+i*4.6f,178};lv_line_set_points(wave_line,wave_points,51);lv_obj_set_style_line_color(wave_line,dim(0xc7e5eb),0);lv_obj_set_style_line_width(wave_line,2,0);voice_label=label("等待输入",65,245,230,14,0xc7e5eb);
 }
 overlay_refresh(page);
}
static void blur_pass(lv_draw_buf_t *in,lv_draw_buf_t *out,bool vertical){
 int radius=40,n=81;
 for(int outer=0;outer<360;outer++){
  int sr=0,sg=0,sb=0;
  for(int j=-radius;j<=radius;j++){int at=j<0?0:j;int x=vertical?outer:at,y=vertical?at:outer;uint16_t v=((uint16_t*)(in->data+y*in->header.stride))[x];sr+=(v>>11)&31;sg+=(v>>5)&63;sb+=v&31;}
  for(int i=0;i<360;i++){
   int x=vertical?outer:i,y=vertical?i:outer;((uint16_t*)(out->data+y*out->header.stride))[x]=((sr/n)<<11)|((sg/n)<<5)|(sb/n);
   int leave=i-radius;if(leave<0)leave=0;int enter=i+radius+1;if(enter>359)enter=359;
   int x0=vertical?outer:leave,y0=vertical?leave:outer,x1=vertical?outer:enter,y1=vertical?enter:outer;
   uint16_t a=((uint16_t*)(in->data+y0*in->header.stride))[x0],b=((uint16_t*)(in->data+y1*in->header.stride))[x1];sr+=((b>>11)&31)-((a>>11)&31);sg+=((b>>5)&63)-((a>>5)&63);sb+=(b&31)-(a&31);
  }
 }
}
static void prepare_glass(void){
 if(glass_image){lv_draw_buf_destroy(glass_image);glass_image=NULL;}
 glass_image=lv_snapshot_take(lv_screen_active(),LV_COLOR_FORMAT_RGB565);
 lv_draw_buf_t *tmp=lv_draw_buf_create(360,360,LV_COLOR_FORMAT_RGB565,LV_STRIDE_AUTO);
 if(glass_image&&tmp){blur_pass(glass_image,tmp,false);blur_pass(tmp,glass_image,true);ESP_LOGI(TAG,"frost snapshot blurred");}
 else ESP_LOGW(TAG,"frost allocation failed; visual fallback only");
 if(tmp)lv_draw_buf_destroy(tmp);
}
static void refresh(lv_event_t *e){(void)e;if(moving){int64_t t=esp_timer_get_time();if(!rendered_frames)first_frame=t;last_frame=t;rendered_frames++;if(overlay&&!lv_obj_has_flag(overlay,LV_OBJ_FLAG_HIDDEN))overlay_leaks++;}}
static void anim_exec(void *obj,int32_t value){if(pending==DECK_CLOCK||pending==DECK_METRICS||current==DECK_CLOCK||current==DECK_METRICS)lv_obj_set_x(obj,value);else lv_obj_set_y(obj,value);}
static void anim_ready(lv_anim_t *a){(void)a;int64_t span=last_frame-first_frame;ESP_LOGI(TAG,"transition frames=%u render_fps=%.1f duration_ms=%lld overlay_leaks=%u",rendered_frames,span>0?(rendered_frames-1)*1000000.0/span:0,(esp_timer_get_time()-anim_start)/1000,overlay_leaks);lv_obj_delete(plane);plane=NULL;moving=false;build(pending);}
void deck_ui_navigate(deck_page_t page){
 lvgl_port_lock(0);if(moving||page==current){lvgl_port_unlock();return;}
 if(page==DECK_SETTINGS)memcpy(draft,mapping,4);
 if(page==DECK_VOICE)prepare_glass();
 lv_indev_reset(pointer,NULL);touch.down=false;
 pending=page;rendered_frames=0;overlay_leaks=0;first_frame=last_frame=0;anim_start=esp_timer_get_time();
 overlay_refresh(page);
 lv_obj_add_flag(overlay,LV_OBJ_FLAG_HIDDEN);
 lv_obj_add_flag(content,LV_OBJ_FLAG_HIDDEN);lv_obj_set_style_bg_color(lv_screen_active(),dim(palette_base),0);
 bool lateral=page==DECK_CLOCK||page==DECK_METRICS||current==DECK_CLOCK||current==DECK_METRICS;
 plane=lv_image_create(lv_screen_active());lv_image_set_src(plane,lateral?motion_image:home_image);lv_obj_set_size(plane,lateral?1880:360,360);
 lv_obj_set_style_bg_color(lv_screen_active(),dim(page_color(page==DECK_HOME?current:page)),0);
 int vertical_offset=(page==DECK_HOME?current:page)==DECK_VOICE?360:-360;
 int start=page==DECK_HOME?vertical_offset:0,end=page==DECK_HOME?0:vertical_offset;
 if(lateral){start=current==DECK_CLOCK?0:current==DECK_METRICS?-1520:-760;end=page==DECK_CLOCK?0:page==DECK_METRICS?-1520:-760;}
 lv_anim_t a;lv_anim_init(&a);lv_anim_set_var(&a,plane);lv_anim_set_values(&a,start,end);lv_anim_set_duration(&a,420);lv_anim_set_exec_cb(&a,anim_exec);lv_anim_set_path_cb(&a,lv_anim_path_ease_in_out);lv_anim_set_completed_cb(&a,anim_ready);moving=true;lv_anim_start(&a);lvgl_port_unlock();
}
static void read_touch(lv_indev_t *i,lv_indev_data_t *d){(void)i;d->point.x=touch.x;d->point.y=touch.y;d->state=touch.down&&!moving?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;}
void deck_ui_cancel_touch(void){lvgl_port_lock(0);contact=false;consumed=true;touch.down=false;lv_indev_reset(pointer,NULL);lvgl_port_unlock();}
void deck_ui_touch(body_touch_t p,bool valid){
 if(!valid){deck_ui_cancel_touch();return;}lvgl_port_lock(0);
 if(moving){touch.down=false;contact=p.down;consumed=true;lvgl_port_unlock();return;}
 if(p.down&&!contact){start_x=last_x=p.x;start_y=last_y=p.y;samples=0;consumed=rejected=false;touch_start=esp_timer_get_time();}
 if(p.down&&contact&&!consumed){int dx=(int)p.x-last_x,dy=(int)p.y-last_y;if(dx*dx+dy*dy>8100)rejected=true;}
 if(p.down){last_x=p.x;last_y=p.y;samples++;}
 int dx=last_x-start_x,dy=last_y-start_y,ax=abs(dx),ay=abs(dy);int id=-1;
 // Navigation is named by the starting edge, rather than the finger's direction.
 if(p.down&&!consumed&&!rejected&&samples>=3&&esp_timer_get_time()-touch_start>=80000){if(ay>=130&&ay*10>=ax*17){if(start_y<=54&&dy>0)id=DECK_VOICE;if(start_y>=306&&dy<0)id=DECK_SETTINGS;}if(ax>=130&&ax*10>=ay*17){if(start_x<=54&&dx>0)id=DECK_CLOCK;if(start_x>=306&&dx<0)id=DECK_METRICS;}}
 if(id>=0){consumed=true;bool reverse=(current==DECK_CLOCK&&id==DECK_METRICS)||(current==DECK_METRICS&&id==DECK_CLOCK)||(current==DECK_VOICE&&id==DECK_SETTINGS)||(current==DECK_SETTINGS&&id==DECK_VOICE);if(current==DECK_HOME||reverse)deck_ui_navigate(current==DECK_HOME?id:DECK_HOME);}
 if((ax>14||ay>14)&&contact)lv_indev_reset(pointer,NULL);
 touch=p;touch.down=p.down&&!consumed&&!rejected&&ax<=14&&ay<=14;contact=p.down;lvgl_port_unlock();
}
void deck_ui_init(const uint8_t map[4],int theme,bool frames){memcpy(mapping,map,4);memcpy(draft,map,4);theme_mode=theme;frames_visible=frames;theme_colors();events=xQueueCreate(16,sizeof(int));ESP_ERROR_CHECK(events?ESP_OK:ESP_ERR_NO_MEM);lvgl_port_lock(0);background_image();pointer=lv_indev_create();lv_indev_set_type(pointer,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(pointer,read_touch);lv_display_add_event_cb(lv_display_get_default(),refresh,LV_EVENT_RENDER_READY,NULL);build(DECK_HOME);lvgl_port_unlock();ESP_LOGI(TAG,"independent UI ready; four content slots");}
bool deck_ui_frames(void){return frames_visible;}
bool deck_ui_event(int *action){return xQueueReceive(events,action,0)==pdTRUE;}
void deck_ui_key(int key){if(key>=1&&key<=4&&!moving)enqueue(mapping[key-1]);}
void deck_ui_draft(uint8_t map[4]){memcpy(map,draft,4);}
void deck_ui_set_mapping(const uint8_t map[4]){lvgl_port_lock(0);memcpy(mapping,map,4);memcpy(draft,map,4);overlay_refresh(current);lvgl_port_unlock();}
void deck_ui_saved(bool ok){lvgl_port_lock(0);if(ok){memcpy(mapping,draft,4);overlay_refresh(current);}if(status_label)lv_label_set_text(status_label,ok?"已保存":"保存失败");lvgl_port_unlock();}
int deck_ui_theme(void){return theme_mode;}
void deck_ui_metric(int index,float v,const char *unit){if(index<0||index>3||!isfinite(v)||!unit)return;lvgl_port_lock(0);metrics[index].value=v;snprintf(metrics[index].unit,12,"%s",unit);metrics[index].at=esp_timer_get_time();lvgl_port_unlock();}
void deck_ui_weather(float v,const char *c,const char *s){if(!isfinite(v)||!c||!s)return;lvgl_port_lock(0);temperature=v;snprintf(city,40,"%s",c);snprintf(condition,sizeof(condition),"%s",s);weather_at=esp_timer_get_time();lvgl_port_unlock();}
void deck_ui_palette(uint32_t left,uint32_t right,uint32_t base){lvgl_port_lock(0);custom_colors[0]=left;custom_colors[1]=right;custom_colors[2]=base;custom_available=true;theme_mode=3;theme_colors();background_image();if(!moving)build(current);lvgl_port_unlock();}
void deck_ui_clock_changed(void){lvgl_port_lock(0);int old=theme_index;theme_colors();if(theme_index!=old){background_image();if(!moving)build(current);}lvgl_port_unlock();}
void deck_ui_tick(void){
 lvgl_port_lock(0);if(moving){lvgl_port_unlock();return;}time_t now=time(NULL);struct tm t;
 if(clock_label){if(now>1700000000&&localtime_r(&now,&t)){char s[64];strftime(s,64,"%H:%M",&t);lv_label_set_text(clock_label,s);const char *days[]={"日","一","二","三","四","五","六"};snprintf(s,64,"%d月%d日 星期%s",t.tm_mon+1,t.tm_mday,days[t.tm_wday]);lv_label_set_text(date_label,s);}else{lv_label_set_text(clock_label,"--:--");lv_label_set_text(date_label,"时间待更新");}}
 if(weather_label&&weather_at&&esp_timer_get_time()-weather_at<1800000000LL){char w[32];snprintf(w,sizeof(w),"%.0f°",(double)temperature);lv_label_set_text(temperature_label,w);lv_label_set_text(city_label,city);lv_label_set_text(weather_label,condition);}
 for(int i=0;i<4;i++)if(metric_labels[i]){bool fresh=metrics[i].at&&esp_timer_get_time()-metrics[i].at<(i==1?15000000:3000000);if(fresh){char m[32];snprintf(m,sizeof(m),"%.1f%s",(double)metrics[i].value,metrics[i].unit);lv_label_set_text(metric_labels[i],m);if(bars[i])lv_bar_set_value(bars[i],strcmp(metrics[i].unit,"%")?0:(int)metrics[i].value,LV_ANIM_OFF);if(status_label&&current==DECK_METRICS)lv_label_set_text(status_label,"");}else{lv_label_set_text(metric_labels[i],"--");if(bars[i])lv_bar_set_value(bars[i],0,LV_ANIM_OFF);}}
 lvgl_port_unlock();
}
void deck_ui_voice(bool available,bool active,int level){
 voice_available=available;voice_active=active;lvgl_port_lock(0);
 if(!moving&&voice_label)lv_label_set_text(voice_label,"等待输入");
 if(!moving&&wave_line&&voice_active){float amplitude=fminf(level,100)*.65f;float phase=esp_timer_get_time()/90000.0f;for(int i=0;i<51;i++){float envelope=sinf(i*3.1415926f/50);wave_points[i]=(lv_point_precise_t){65+i*4.6f,178+amplitude*envelope*sinf(i*.65f+phase)};}lv_obj_invalidate(wave_line);}
 lvgl_port_unlock();
}
