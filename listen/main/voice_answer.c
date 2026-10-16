#include "voice_answer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "driver/i2s_std.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"
#include "esp_mn_speech_commands.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "model_path.h"

#include "board_hw.h"

static const char *TAG = "voice";
#define VOICE_SAMPLE_RATE_HZ 16000
#define VOICE_CAPTURE_WINDOW_MS 5500

static i2s_chan_handle_t s_rx_channel;
static srmodel_list_t *s_models;
static esp_mn_iface_t *s_multinet;
static model_iface_data_t *s_model_data;
static int s_chunk_samples;
static bool s_ready;

static esp_err_t microphone_channel_init(void)
{
    i2s_chan_config_t channel_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_RETURN_ON_ERROR(i2s_new_channel(&channel_config, NULL, &s_rx_channel),
                        TAG, "create microphone channel");

    i2s_std_config_t standard_config = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(VOICE_SAMPLE_RATE_HZ),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT,
                                                       I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = BOARD_MIC_BCLK_GPIO,
            .ws = BOARD_MIC_WS_GPIO,
            .dout = I2S_GPIO_UNUSED,
            .din = BOARD_MIC_DIN_GPIO,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(s_rx_channel, &standard_config),
                        TAG, "init microphone std mode");
    return i2s_channel_enable(s_rx_channel);
}

esp_err_t voice_answer_init(void)
{
    if (s_ready) {
        return ESP_OK;
    }

    s_models = esp_srmodel_init("model");
    if (s_models == NULL || s_models->num == 0) {
        ESP_LOGE(TAG, "no ESP-SR model found in partition 'model'");
        return ESP_FAIL;
    }

    char *model_name = esp_srmodel_filter(s_models, ESP_MN_PREFIX, NULL);
    if (model_name == NULL) {
        ESP_LOGE(TAG, "no MultiNet model found; check sdkconfig ESP_SR model selection");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "MultiNet model: %s", model_name);

    s_multinet = esp_mn_handle_from_name(model_name);
    if (s_multinet == NULL) {
        ESP_LOGE(TAG, "create MultiNet handle failed");
        return ESP_FAIL;
    }
    s_model_data = s_multinet->create(model_name, 5000);
    if (s_model_data == NULL) {
        ESP_LOGE(TAG, "create MultiNet model data failed");
        return ESP_ERR_NO_MEM;
    }
    s_chunk_samples = s_multinet->get_samp_chunksize(s_model_data);
    if (s_chunk_samples <= 0) {
        ESP_LOGE(TAG, "invalid MultiNet chunk size");
        return ESP_FAIL;
    }

    esp_err_t commands_err = esp_mn_commands_alloc(s_multinet, s_model_data);
    if (commands_err != ESP_OK) {
        ESP_LOGE(TAG, "alloc speech commands failed: %s", esp_err_to_name(commands_err));
        return commands_err;
    }
    (void)esp_mn_commands_clear();
    esp_mn_commands_add(1, "yi");
    esp_mn_commands_add(2, "er");
    esp_mn_commands_add(3, "san");
    esp_mn_commands_add(4, "si");
    esp_mn_error_t *bad_commands = esp_mn_commands_update();
    if (bad_commands != NULL) {
        ESP_LOGE(TAG, "one or more command phrases were rejected");
        return ESP_FAIL;
    }
    if (s_multinet->set_det_threshold != NULL) {
        s_multinet->set_det_threshold(s_model_data, 0.42f);
    }

    int frequency = s_multinet->get_samp_rate(s_model_data);
    if (frequency != VOICE_SAMPLE_RATE_HZ) {
        ESP_LOGE(TAG, "unexpected model rate %d", frequency);
        return ESP_FAIL;
    }
    ESP_RETURN_ON_ERROR(microphone_channel_init(), TAG, "init microphone");

    s_ready = true;
    ESP_LOGI(TAG, "voice answers ready: yi=1 er=2 san=3 si=4, %d samples/chunk",
             s_chunk_samples);
    return ESP_OK;
}

uint8_t voice_answer_listen(void)
{
    if (!s_ready || s_multinet == NULL || s_model_data == NULL || s_chunk_samples <= 0) {
        return VOICE_ANSWER_INVALID;
    }

    size_t raw_bytes = (size_t)s_chunk_samples * sizeof(int32_t);
    int32_t *raw = malloc(raw_bytes);
    int16_t *pcm = malloc((size_t)s_chunk_samples * sizeof(int16_t));
    if (raw == NULL || pcm == NULL) {
        free(raw);
        free(pcm);
        return VOICE_ANSWER_INVALID;
    }

    TickType_t start = xTaskGetTickCount();
    uint8_t answer = VOICE_ANSWER_INVALID;
    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(VOICE_CAPTURE_WINDOW_MS)) {
        size_t bytes_read = 0;
        esp_err_t err = i2s_channel_read(s_rx_channel, raw, raw_bytes,
                                         &bytes_read, pdMS_TO_TICKS(100));
        if (err != ESP_OK || bytes_read != raw_bytes) {
            continue;
        }

        for (int i = 0; i < s_chunk_samples; ++i) {
            // INMP441-style 24-bit data is left-aligned in a 32-bit slot.
            // Shift to 18-bit range and clip, giving useful gain for short replies.
            int32_t sample = raw[i] >> 14;
            if (sample > 32767) {
                sample = 32767;
            } else if (sample < -32768) {
                sample = -32768;
            }
            pcm[i] = (int16_t)sample;
        }

        esp_mn_state_t state = s_multinet->detect(s_model_data, pcm);
        if (state == ESP_MN_STATE_DETECTED) {
            esp_mn_results_t *result = s_multinet->get_results(s_model_data);
            if (result != NULL && result->num > 0 &&
                result->command_id[0] >= 1 && result->command_id[0] <= 4) {
                answer = (uint8_t)result->command_id[0];
                ESP_LOGI(TAG, "recognized answer %u", answer);
            }
            break;
        }
        if (state == ESP_MN_STATE_TIMEOUT) {
            break;
        }
    }

    free(raw);
    free(pcm);
    return answer;
}

bool voice_answer_ready(void)
{
    return s_ready;
}
