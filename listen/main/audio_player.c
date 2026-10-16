#include "audio_player.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/i2s_std.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "board_hw.h"

static const char *TAG = "audio";
#define AUDIO_SAMPLE_RATE_HZ 16000
#define AUDIO_MAX_PATH 96
#define AUDIO_OUTPUT_FRAMES 512

typedef struct {
    uint32_t sample_rate;
    uint16_t channels;
    uint16_t bits_per_sample;
    uint32_t data_size;
    long data_offset;
} wav_info_t;

static i2s_chan_handle_t s_tx_channel;
static bool s_spiffs_mounted;

static bool parse_wav(FILE *file, wav_info_t *info)
{
    uint8_t header[12];
    if (fread(header, 1, sizeof(header), file) != sizeof(header) ||
        memcmp(header, "RIFF", 4) != 0 || memcmp(header + 8, "WAVE", 4) != 0) {
        ESP_LOGE(TAG, "file is not RIFF/WAVE");
        return false;
    }

    bool have_fmt = false;
    bool have_data = false;
    while (!have_fmt || !have_data) {
        uint8_t chunk_header[8];
        if (fread(chunk_header, 1, sizeof(chunk_header), file) != sizeof(chunk_header)) {
            ESP_LOGE(TAG, "truncated WAV chunk header");
            return false;
        }
        uint32_t chunk_size = (uint32_t)chunk_header[4] |
                              ((uint32_t)chunk_header[5] << 8) |
                              ((uint32_t)chunk_header[6] << 16) |
                              ((uint32_t)chunk_header[7] << 24);
        long next_chunk = ftell(file) + (long)chunk_size + (chunk_size & 1);

        if (memcmp(chunk_header, "fmt ", 4) == 0) {
            uint8_t fmt[16];
            size_t read_size = chunk_size < sizeof(fmt) ? chunk_size : sizeof(fmt);
            if (read_size < 16 || fread(fmt, 1, read_size, file) != read_size) {
                ESP_LOGE(TAG, "bad fmt chunk");
                return false;
            }
            uint16_t format = (uint16_t)(fmt[0] | (fmt[1] << 8));
            info->channels = (uint16_t)(fmt[2] | (fmt[3] << 8));
            info->sample_rate = (uint32_t)fmt[4] | ((uint32_t)fmt[5] << 8) |
                                ((uint32_t)fmt[6] << 16) | ((uint32_t)fmt[7] << 24);
            info->bits_per_sample = (uint16_t)(fmt[14] | (fmt[15] << 8));
            if (format != 1 || info->channels == 0 || info->channels > 2 ||
                (info->bits_per_sample != 8 && info->bits_per_sample != 16 &&
                 info->bits_per_sample != 24)) {
                ESP_LOGE(TAG, "WAV must be PCM, 1-2 channel, 8/16/24-bit");
                return false;
            }
            have_fmt = true;
        } else if (memcmp(chunk_header, "data", 4) == 0) {
            info->data_offset = ftell(file);
            info->data_size = chunk_size;
            have_data = true;
        }

        if (fseek(file, next_chunk, SEEK_SET) != 0) {
            ESP_LOGE(TAG, "bad WAV chunk boundary");
            return false;
        }
    }
    return true;
}

static int16_t decode_sample(const uint8_t *frame, uint16_t channels, uint16_t bits)
{
    int32_t left = 0;
    int32_t right = 0;
    if (bits == 8) {
        left = (int32_t)frame[0] - 128;
        right = channels == 2 ? (int32_t)frame[1] - 128 : left;
    } else if (bits == 16) {
        left = (int16_t)(frame[0] | (frame[1] << 8));
        right = channels == 2 ? (int16_t)(frame[2] | (frame[3] << 8)) : left;
    } else {
        left = (int32_t)(((uint32_t)frame[0] << 8) | ((uint32_t)frame[1] << 16) |
                         ((uint32_t)frame[2] << 24)) >> 8;
        size_t step = 3;
        right = channels == 2
            ? (int32_t)(((uint32_t)frame[step] << 8) | ((uint32_t)frame[step + 1] << 16) |
                         ((uint32_t)frame[step + 2] << 24)) >> 8
            : left;
    }
    int32_t mixed = (left + right) / 2;
    // The board speaker is close to the user; keep generated TTS comfortable.
    return (int16_t)((mixed * 3) / 4);
}

static esp_err_t play_wav_stream(FILE *file, const char *filename)
{
    wav_info_t info = {0};
    if (!parse_wav(file, &info)) {
        return ESP_ERR_INVALID_STATE;
    }
    if (info.sample_rate < 4000 || info.sample_rate > 48000) {
        ESP_LOGE(TAG, "%s unsupported rate %" PRIu32, filename, info.sample_rate);
        return ESP_ERR_NOT_SUPPORTED;
    }
    size_t frame_size = (size_t)info.channels * info.bits_per_sample / 8;
    if (frame_size == 0 || info.data_size == 0 || info.data_size > 4U * 1024U * 1024U) {
        ESP_LOGE(TAG, "%s invalid data size", filename);
        return ESP_ERR_INVALID_SIZE;
    }

    uint8_t *source = malloc(info.data_size);
    int16_t *output = malloc(AUDIO_OUTPUT_FRAMES * 2 * sizeof(int16_t));
    if (source == NULL || output == NULL) {
        free(source);
        free(output);
        return ESP_ERR_NO_MEM;
    }
    if (fseek(file, info.data_offset, SEEK_SET) != 0 ||
        fread(source, 1, info.data_size, file) != info.data_size) {
        free(source);
        free(output);
        ESP_LOGE(TAG, "%s truncated data chunk", filename);
        return ESP_ERR_INVALID_SIZE;
    }

    size_t source_frames = info.data_size / frame_size;
    uint64_t target_frames64 = ((uint64_t)source_frames * AUDIO_SAMPLE_RATE_HZ +
                                info.sample_rate - 1) / info.sample_rate;
    if (target_frames64 > 20U * 60U * AUDIO_SAMPLE_RATE_HZ) {
        free(source);
        free(output);
        return ESP_ERR_INVALID_SIZE;
    }
    size_t target_frames = (size_t)target_frames64;
    size_t output_count = 0;
    uint32_t step_q32 = (uint32_t)((((uint64_t)info.sample_rate) << 32) / AUDIO_SAMPLE_RATE_HZ);

    for (size_t target_index = 0; target_index < target_frames; ++target_index) {
        size_t source_index = (size_t)((((uint64_t)target_index * step_q32) >> 32));
        if (source_index >= source_frames) {
            source_index = source_frames - 1;
        }
        int16_t sample = decode_sample(&source[source_index * frame_size],
                                       info.channels, info.bits_per_sample);
        output[output_count * 2] = sample;
        output[output_count * 2 + 1] = sample;
        ++output_count;

        if (output_count == AUDIO_OUTPUT_FRAMES) {
            size_t bytes_written = 0;
            esp_err_t err = i2s_channel_write(s_tx_channel, output,
                                              AUDIO_OUTPUT_FRAMES * 2 * sizeof(int16_t),
                                              &bytes_written, portMAX_DELAY);
            if (err != ESP_OK) {
                free(source);
                free(output);
                return err;
            }
            output_count = 0;
        }
    }
    if (output_count != 0) {
        size_t bytes_written = 0;
        i2s_channel_write(s_tx_channel, output, output_count * 2 * sizeof(int16_t),
                          &bytes_written, portMAX_DELAY);
    }

    free(source);
    free(output);
    // Let the final DMA samples drain before a microphone capture starts.
    vTaskDelay(pdMS_TO_TICKS(100));
    return ESP_OK;
}

esp_err_t audio_player_init(void)
{
    if (!s_spiffs_mounted) {
        esp_vfs_spiffs_conf_t cfg = {
            .base_path = "/assets",
            .partition_label = "assets",
            .max_files = 512,
            .format_if_mount_failed = false,
        };
        ESP_RETURN_ON_ERROR(esp_vfs_spiffs_register(&cfg), TAG, "mount assets");
        s_spiffs_mounted = true;
        size_t total = 0;
        size_t used = 0;
        esp_spiffs_info("assets", &total, &used);
        ESP_LOGI(TAG, "assets: used %u / %u bytes", used, total);
    }

    i2s_chan_config_t channel_config = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    channel_config.auto_clear = true;
    ESP_RETURN_ON_ERROR(i2s_new_channel(&channel_config, &s_tx_channel, NULL),
                        TAG, "create speaker channel");

    i2s_std_config_t standard_config = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(AUDIO_SAMPLE_RATE_HZ),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                       I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = BOARD_SPK_BCLK_GPIO,
            .ws = BOARD_SPK_WS_GPIO,
            .dout = BOARD_SPK_DOUT_GPIO,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(s_tx_channel, &standard_config),
                        TAG, "init speaker std mode");
    ESP_RETURN_ON_ERROR(i2s_channel_enable(s_tx_channel), TAG, "enable speaker");
    return ESP_OK;
}

esp_err_t audio_play_file(const char *filename)
{
    char path[AUDIO_MAX_PATH];
    if (snprintf(path, sizeof(path), "/assets/%s", filename) >= (int)sizeof(path)) {
        return ESP_ERR_INVALID_SIZE;
    }
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        ESP_LOGE(TAG, "missing asset %s", filename);
        return ESP_ERR_NOT_FOUND;
    }
    esp_err_t err = play_wav_stream(file, filename);
    fclose(file);
    return err;
}

esp_err_t audio_play_files(const char *const *filenames, size_t count)
{
    esp_err_t result = ESP_OK;
    for (size_t i = 0; i < count; ++i) {
        esp_err_t err = audio_play_file(filenames[i]);
        if (err != ESP_OK) {
            result = err;
        }
    }
    return result;
}

esp_err_t audio_playf(const char *format, ...)
{
    char filename[AUDIO_MAX_PATH];
    va_list args;
    va_start(args, format);
    int length = vsnprintf(filename, sizeof(filename), format, args);
    va_end(args);
    if (length < 0 || length >= (int)sizeof(filename)) {
        return ESP_ERR_INVALID_SIZE;
    }
    return audio_play_file(filename);
}
