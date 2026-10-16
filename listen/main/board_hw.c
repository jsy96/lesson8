#include "board_hw.h"

#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "board_hw";

static const gpio_num_t s_button_gpios[] = {
    BOARD_BUTTON_S1_GPIO, BOARD_BUTTON_S2_GPIO, BOARD_BUTTON_S3_GPIO,
    BOARD_BUTTON_S4_GPIO, BOARD_BUTTON_S5_GPIO, BOARD_BUTTON_S6_GPIO,
    BOARD_BUTTON_S7_GPIO, BOARD_BUTTON_S8_GPIO, BOARD_BUTTON_S9_GPIO,
    BOARD_ENCODER_A_GPIO, BOARD_ENCODER_B_GPIO,
};

esp_err_t board_hw_init(void)
{
    esp_err_t ret = ESP_OK;

    // 1. Make the shared LED/microphone/speaker rail safe while pins switch.
    gpio_config_t pwr_cfg = {
        .pin_bit_mask = 1ULL << BOARD_PWR_EN_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_set_level(BOARD_PWR_EN_GPIO, 0);
    ESP_RETURN_ON_ERROR(gpio_config(&pwr_cfg), TAG, "configure PWR_EN");

    // 2. Pre-drive all audio outputs low before enabling their shared supply.
    const gpio_num_t audio_outputs[] = {
        BOARD_MIC_BCLK_GPIO, BOARD_MIC_WS_GPIO,
        BOARD_SPK_BCLK_GPIO, BOARD_SPK_WS_GPIO, BOARD_SPK_DOUT_GPIO,
    };
    gpio_config_t low_cfg = {
        .pin_bit_mask = 0,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    for (size_t i = 0; i < sizeof(audio_outputs) / sizeof(audio_outputs[0]); ++i) {
        low_cfg.pin_bit_mask |= 1ULL << audio_outputs[i];
        gpio_set_level(audio_outputs[i], 0);
    }
    ESP_RETURN_ON_ERROR(gpio_config(&low_cfg), TAG, "pre-drive audio pins");

    // GPIO11 is microphone data-in. Never drive it.
    gpio_config_t mic_data_cfg = {
        .pin_bit_mask = 1ULL << BOARD_MIC_DIN_GPIO,
        .mode = GPIO_MODE_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&mic_data_cfg), TAG, "make mic data floating");

    // 3. Enable the shared rail and wait for it to stabilize before I2S starts.
    gpio_set_level(BOARD_PWR_EN_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(50));

    gpio_config_t input_cfg = {
        .pin_bit_mask = 0,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    for (size_t i = 0; i < sizeof(s_button_gpios) / sizeof(s_button_gpios[0]); ++i) {
        input_cfg.pin_bit_mask |= 1ULL << s_button_gpios[i];
    }
    ESP_RETURN_ON_ERROR(gpio_config(&input_cfg), TAG, "configure buttons/encoder");

    ESP_LOGI(TAG, "EasyInput board ready: PWR_EN high, %zu inputs configured",
             sizeof(s_button_gpios) / sizeof(s_button_gpios[0]));
    return ret;
}
