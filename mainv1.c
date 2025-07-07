#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "driver/rmt.h"
#include "esp_system.h"

#define LED_STRIP_RMT_CHANNEL RMT_CHANNEL_0
#define LED_STRIP_GPIO 18      // Change to your GPIO
#define LED_NUMBER 100         // Number of LEDs

static led_strip_t *strip;

// Helper: generate random number in range [min, max]
uint32_t random_in_range(uint32_t min, uint32_t max) {
    return min + esp_random() % (max - min + 1);
}

void led_task(void *param) {
    while (1) {
        printf("Changing colors!\n");

        for (int i = 0; i < LED_NUMBER; i++) {
            // Randomly choose red or blue
            bool is_red = esp_random() % 2;

            if (is_red) {
                strip->set_pixel(strip, i, 255, 0, 0); // Red
            } else {
                strip->set_pixel(strip, i, 0, 0, 255); // Blue
            }
        }

        strip->refresh(strip, 100);

        // Wait random 150s–600s
        uint32_t delay_sec = random_in_range(150, 600);
        printf("Next change in %d seconds\n", delay_sec);
        vTaskDelay(pdMS_TO_TICKS(delay_sec * 1000));
    }
}

void app_main(void)
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(LED_STRIP_GPIO, LED_STRIP_RMT_CHANNEL);
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    strip = led_strip_new_rmt_ws2812(&config);
    if (!strip) {
        printf("Failed to install WS2812 driver\n");
        return;
    }

    ESP_ERROR_CHECK(strip->clear(strip, 100));

    // Start LED task
    xTaskCreate(led_task, "led_task", 2048, NULL, 5, NULL);
}
