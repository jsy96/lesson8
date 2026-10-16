#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INPUT_EVENT_NONE       0
#define INPUT_EVENT_S1         (1 << 0)
#define INPUT_EVENT_S2         (1 << 1)
#define INPUT_EVENT_S3         (1 << 2)
#define INPUT_EVENT_S4         (1 << 3)
#define INPUT_EVENT_S5         (1 << 4)
#define INPUT_EVENT_S6         (1 << 5)
#define INPUT_EVENT_S7         (1 << 6)
#define INPUT_EVENT_S8         (1 << 7)
#define INPUT_EVENT_S9         (1 << 8)
#define INPUT_ENCODER_CW       (1 << 9)
#define INPUT_ENCODER_CCW      (1 << 10)

typedef struct {
    uint8_t stable_buttons;
    uint8_t previous_buttons;
    uint8_t candidate_buttons;
    uint8_t candidate_ticks;
    uint8_t previous_quadrature;
    int8_t quadrature_counter;
} input_state_t;

void input_init(input_state_t *state);
uint32_t input_poll(input_state_t *state);
void input_delay_scan(void);

#ifdef __cplusplus
}
#endif
