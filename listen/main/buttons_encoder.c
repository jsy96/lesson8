#include "buttons_encoder.h"

#include "board_hw.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const uint8_t DEBOUNCE_TICKS = 4; // 4 * 5 ms = 20 ms.

static const gpio_num_t s_buttons[] = {
    BOARD_BUTTON_S1_GPIO, BOARD_BUTTON_S2_GPIO, BOARD_BUTTON_S3_GPIO,
    BOARD_BUTTON_S4_GPIO, BOARD_BUTTON_S5_GPIO, BOARD_BUTTON_S6_GPIO,
    BOARD_BUTTON_S7_GPIO, BOARD_BUTTON_S8_GPIO, BOARD_BUTTON_S9_GPIO,
};

// A mechanical encoder detent normally emits four quadrature state changes.
static const int8_t s_quadrature_delta[16] = {
    0, -1, +1,  0,
    +1,  0,  0, -1,
    -1,  0,  0, +1,
    0, +1, -1,  0,
};

void input_init(input_state_t *state)
{
    state->stable_buttons = 0;
    state->previous_buttons = 0;
    state->candidate_buttons = 0;
    state->candidate_ticks = 0;
    uint8_t a = gpio_get_level(BOARD_ENCODER_A_GPIO) ? 1 : 0;
    uint8_t b = gpio_get_level(BOARD_ENCODER_B_GPIO) ? 1 : 0;
    state->previous_quadrature = (uint8_t)((a << 1) | b);
    state->quadrature_counter = 0;
}

uint32_t input_poll(input_state_t *state)
{
    uint32_t events = INPUT_EVENT_NONE;
    uint8_t raw = 0;

    for (size_t i = 0; i < sizeof(s_buttons) / sizeof(s_buttons[0]); ++i) {
        raw |= gpio_get_level(s_buttons[i]) ? 0 : (uint8_t)(1U << i);
    }

    if (raw == state->candidate_buttons) {
        if (state->candidate_ticks < DEBOUNCE_TICKS) {
            ++state->candidate_ticks;
        }
    } else {
        state->candidate_buttons = raw;
        state->candidate_ticks = 0;
    }

    if (state->candidate_ticks >= DEBOUNCE_TICKS && raw != state->stable_buttons) {
        uint8_t falling = (uint8_t)(raw & ~state->stable_buttons);
        state->previous_buttons = state->stable_buttons;
        state->stable_buttons = raw;
        for (int i = 0; i < 9; ++i) {
            if (falling & (1U << i)) {
                events |= 1U << i;
            }
        }
    }

    uint8_t a = gpio_get_level(BOARD_ENCODER_A_GPIO) ? 1 : 0;
    uint8_t b = gpio_get_level(BOARD_ENCODER_B_GPIO) ? 1 : 0;
    uint8_t quadrature = (uint8_t)((a << 1) | b);
    uint8_t index = (uint8_t)((state->previous_quadrature << 2) | quadrature);
    state->quadrature_counter = (int8_t)(state->quadrature_counter + s_quadrature_delta[index]);
    state->previous_quadrature = quadrature;

    if (state->quadrature_counter >= 4) {
        state->quadrature_counter = 0;
        events |= INPUT_ENCODER_CW;
    } else if (state->quadrature_counter <= -4) {
        state->quadrature_counter = 0;
        events |= INPUT_ENCODER_CCW;
    }

    return events;
}

void input_delay_scan(void)
{
    vTaskDelay(pdMS_TO_TICKS(5));
}
