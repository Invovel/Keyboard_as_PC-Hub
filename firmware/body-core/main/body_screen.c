#include "body_screen.h"
#include <inttypes.h>
#include "driver/spi_master.h"
#include "driver/i2c_master.h"
#include "esp_lcd_st77916.h"
#include "esp_lcd_touch_cst816s.h"
#include "esp_lvgl_port.h"
#include "esp_check.h"
#include "panel_verified_init.h"

static const char *TAG = "body-screen";
static esp_lcd_touch_handle_t touch;

esp_err_t body_screen_init(void)
{
    // Reuse the module-specific register data previously verified on this panel.
    // The Espressif transport does not establish the physical controller ID.
    static st77916_lcd_init_cmd_t commands[sizeof(panel_init)/sizeof(panel_init[0])];
    for (size_t i=0; i<sizeof(commands)/sizeof(commands[0]); ++i) {
        commands[i] = (st77916_lcd_init_cmd_t){panel_init[i].cmd, panel_init[i].data,
                                             panel_init[i].len, panel_init[i].delay_ms};
    }
    spi_bus_config_t bus = ST77916_PANEL_BUS_QSPI_CONFIG(6,15,7,11,12,360*20*2);
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI2_HOST,&bus,SPI_DMA_CH_AUTO),TAG,"SPI bus");
    esp_lcd_panel_io_spi_config_t io_cfg = ST77916_PANEL_IO_QSPI_CONFIG(16,NULL,NULL);
    io_cfg.spi_mode=3; io_cfg.pclk_hz=20000000;
    esp_lcd_panel_io_handle_t io;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi(SPI2_HOST,&io_cfg,&io),TAG,"SPI IO");
    st77916_vendor_config_t vendor = {.init_cmds=commands,
        .init_cmds_size=sizeof(commands)/sizeof(commands[0]), .flags.use_qspi_interface=true};
    esp_lcd_panel_dev_config_t panel_cfg = {.reset_gpio_num=-1,
        .rgb_ele_order=LCD_RGB_ELEMENT_ORDER_RGB,.bits_per_pixel=16,.vendor_config=&vendor};
    esp_lcd_panel_handle_t panel;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_st77916(io,&panel_cfg,&panel),TAG,"panel create");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(panel),TAG,"software reset");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel),TAG,"panel init");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(panel,true),TAG,"panel on");
    lvgl_port_cfg_t port = ESP_LVGL_PORT_INIT_CONFIG();
    port.task_max_sleep_ms=10;
    ESP_RETURN_ON_ERROR(lvgl_port_init(&port),TAG,"LVGL port");
    lvgl_port_display_cfg_t display = {.io_handle=io,.panel_handle=panel,
        .buffer_size=360*20,.double_buffer=true,.hres=360,.vres=360,
        .color_format=LV_COLOR_FORMAT_RGB565,.flags={.buff_dma=true,.swap_bytes=true}};
    if (!lvgl_port_add_disp(&display)) return ESP_ERR_NO_MEM;
    const uint32_t colors[]={0xff0000,0x00ff00,0x0000ff};
    for (int i=0;i<3;++i) {
        lvgl_port_lock(0);
        lv_obj_set_style_bg_color(lv_screen_active(),lv_color_hex(colors[i]),0);
        lvgl_port_unlock();
        ESP_LOGI(TAG,"Color test %06"PRIx32,colors[i]);
        vTaskDelay(pdMS_TO_TICKS(700));
    }
    ESP_LOGI(TAG,"360x360 QSPI mode3 20MHz ready; visual verification required");
    return ESP_OK;
}

esp_err_t body_touch_init(void)
{
    i2c_master_bus_config_t cfg={.i2c_port=I2C_NUM_0,.sda_io_num=8,.scl_io_num=9,
        .clk_source=I2C_CLK_SRC_DEFAULT,.glitch_ignore_cnt=7,.flags.enable_internal_pullup=true};
    i2c_master_bus_handle_t bus;
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&cfg,&bus),TAG,"touch bus");
    ESP_RETURN_ON_ERROR(i2c_master_probe(bus,0x15,100),TAG,"touch 0x15 absent");
    esp_lcd_panel_io_i2c_config_t io_cfg=ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
    esp_lcd_panel_io_handle_t io;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(bus,&io_cfg,&io),TAG,"touch IO");
    esp_lcd_touch_config_t tc={.x_max=360,.y_max=360,.rst_gpio_num=-1,.int_gpio_num=-1};
    ESP_RETURN_ON_ERROR(esp_lcd_touch_new_i2c_cst816s(io,&tc,&touch),TAG,"touch driver");
    ESP_LOGI(TAG,"CST816S-compatible coordinate reader at 0x15; chip ID not asserted");
    return ESP_OK;
}

esp_err_t body_touch_read(body_touch_t *point)
{
    *point=(body_touch_t){0};
    if (!touch) return ESP_ERR_INVALID_STATE;
    esp_err_t err=esp_lcd_touch_read_data(touch);
    if (err!=ESP_OK) return err;
    uint8_t count=0;
    esp_lcd_touch_point_data_t data={0};
    err=esp_lcd_touch_get_data(touch,&data,&count,1);
    if(err!=ESP_OK)return err;
    point->down=count>0;
    point->x=data.x;point->y=data.y;
    if (point->down && (point->x>=360 || point->y>=360)) {
        point->down=false;
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}
