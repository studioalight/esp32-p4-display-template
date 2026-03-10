/*
 * HelloWorld for ESP32-P4 - Song lyric test
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "lvgl.h"

static const char *TAG = "HELLO_DISPLAY";

void app_main(void)
{
    printf("\n\n=== HELLO P4 DISPLAY ===\n");
    printf("D'ENT + Fredrik\n\n");

    ESP_LOGI(TAG, "Initializing...");
    
    /* Init display using BSP */
    bsp_display_start();
    bsp_display_backlight_on();
    
    ESP_LOGI(TAG, "Display ready 720x720");
    
    /* Lock and create UI */
    bsp_display_lock(-1);
    
    /* Black background */
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
    
    /* Create text label */
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "The Marshmallow stands\nOn solid foundation\nIterate incrementally\nBuild through the night\n\nD'ENT + Fredrik | March 10");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    
    bsp_display_unlock();
    
    ESP_LOGI(TAG, "Done!");
    printf("\n=== DISPLAY RUNNING ===\n");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}