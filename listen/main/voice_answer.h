#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VOICE_ANSWER_INVALID 0

/**
 * Load the offline Chinese MultiNet model and prepare the microphone I2S RX channel.
 * The model partition must have label "model".
 */
esp_err_t voice_answer_init(void);

/**
 * Capture for a short answer window and recognize one of:
 * yi -> 1, er -> 2, san -> 3, si -> 4.
 *
 * Returns 0 when no valid answer is recognized.
 */
uint8_t voice_answer_listen(void);

/**
 * Return true once the model and microphone are available.
 */
bool voice_answer_ready(void);

#ifdef __cplusplus
}
#endif
