#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_random.h"

#define LED_STRIP_RMT_CHANNEL 0
#define LED_STRIP_GPIO 18
#define LED_NUMBERS 100

static const char *TAG = "LED_STRIP_EXAMPLE";

void app_main(void) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO,
        .max_leds = LED_NUMBERS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10 MHz
        .mem_block_symbols = 64,
        .flags.with_dma = false,
    };

    led_strip_handle_t strip;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &strip));
    ESP_ERROR_CHECK(led_strip_clear(strip));

    ESP_LOGI(TAG, "Starting random color loop");

    while (1) {
        uint8_t r = 255, g = 0, b = 0;
        if (esp_random() % 2) {
            r = 0; b = 255; g = 0;
        }

        for (int i = 0; i < LED_NUMBERS; i++) {
            ESP_ERROR_CHECK(led_strip_set_pixel(strip, i, r, g, b));
        }
        ESP_ERROR_CHECK(led_strip_refresh(strip));

        uint32_t delay_s = (esp_random() % (600 - 150) + 150); // 150–600 sec
        ESP_LOGI(TAG, "Next color in %lu sec", delay_s);
        vTaskDelay(pdMS_TO_TICKS(delay_s * 1000));
    }
}
