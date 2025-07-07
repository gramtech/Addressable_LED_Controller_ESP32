#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_random.h"

#define LED_STRIP_RMT_CHANNEL 0
#define LED_STRIP_GPIO 18
#define LED_NUMBERS 100

// === Configurable timings ===
#define TRANSITION_TIME_MS 5000      // Total time for transition wipe in milliseconds
#define INTERVAL_MIN_S     150       // Minimum time between transitions in seconds (2.5 minutes)
#define INTERVAL_MAX_S     600       // Maximum time between transitions in seconds (10 minutes)

// #define TRANSITION_TIME_MS 500      // Total time for transition wipe in milliseconds
// #define INTERVAL_MIN_S     2       // Minimum time between transitions in seconds
// #define INTERVAL_MAX_S     11       // Maximum time between transitions in seconds


static const char *TAG = "LED_STRIP";

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

    ESP_LOGI(TAG, "Starting random color wipe loop");

    while (1) {
        // Pick next color: red or blue
        uint8_t r = 255, g = 0, b = 0;
        if (esp_random() % 2) {
            r = 0; g = 0; b = 255;
        }

        ESP_LOGI(TAG, "Transitioning to color: R=%d G=%d B=%d", r, g, b);

        // Calculate per-pixel delay
        int delay_per_pixel_ms = TRANSITION_TIME_MS / LED_NUMBERS;

        for (int i = 0; i < LED_NUMBERS; i++) {
            ESP_ERROR_CHECK(led_strip_set_pixel(strip, i, r, g, b));
            ESP_ERROR_CHECK(led_strip_refresh(strip));
            vTaskDelay(pdMS_TO_TICKS(delay_per_pixel_ms));
        }

        uint32_t delay_s = (esp_random() % (INTERVAL_MAX_S - INTERVAL_MIN_S) + INTERVAL_MIN_S);
        ESP_LOGI(TAG, "Holding for %lu seconds", delay_s);
        vTaskDelay(pdMS_TO_TICKS(delay_s * 1000));
    }
}
