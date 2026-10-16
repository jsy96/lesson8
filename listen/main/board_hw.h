#pragma once

#include <stdint.h>
#include "esp_err.h"

#define BOARD_PWR_EN_GPIO       GPIO_NUM_8
#define BOARD_BUTTON_S1_GPIO    GPIO_NUM_2
#define BOARD_BUTTON_S2_GPIO    GPIO_NUM_47
#define BOARD_BUTTON_S3_GPIO    GPIO_NUM_38
#define BOARD_BUTTON_S4_GPIO    GPIO_NUM_41
#define BOARD_BUTTON_S5_GPIO    GPIO_NUM_1
#define BOARD_BUTTON_S6_GPIO    GPIO_NUM_6
#define BOARD_BUTTON_S7_GPIO    GPIO_NUM_7
#define BOARD_BUTTON_S8_GPIO    GPIO_NUM_48
#define BOARD_BUTTON_S9_GPIO    GPIO_NUM_18
#define BOARD_ENCODER_A_GPIO    GPIO_NUM_17
#define BOARD_ENCODER_B_GPIO    GPIO_NUM_16

#define BOARD_MIC_BCLK_GPIO     GPIO_NUM_9
#define BOARD_MIC_WS_GPIO       GPIO_NUM_10
#define BOARD_MIC_DIN_GPIO      GPIO_NUM_11
#define BOARD_SPK_BCLK_GPIO     GPIO_NUM_14
#define BOARD_SPK_WS_GPIO       GPIO_NUM_13
#define BOARD_SPK_DOUT_GPIO     GPIO_NUM_15

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Bring up the EasyInput shared peripheral power rail and local inputs.
 *
 * GPIO8 must stay low while audio pins are pre-driven low. GPIO11 is an input
 * from the microphone and is deliberately left floating/disabled.
 */
esp_err_t board_hw_init(void);

#ifdef __cplusplus
}
#endif
