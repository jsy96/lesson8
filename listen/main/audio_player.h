#pragma once

#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t audio_player_init(void);
esp_err_t audio_play_file(const char *filename);
esp_err_t audio_play_files(const char *const *filenames, size_t count);
esp_err_t audio_playf(const char *format, ...) __attribute__((format(printf, 1, 2)));

#ifdef __cplusplus
}
#endif
