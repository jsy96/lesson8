#include <stdbool.h>
#include <stdint.h>

#include "audio_player.h"
#include "board_hw.h"
#include "buttons_encoder.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "questions.h"
#include "scoring.h"
#include "voice_answer.h"

static const char *TAG = "listen";

static void play_question_number(uint8_t index)
{
    audio_playf("ui_number_%02u.wav", index + 1);
}

static void play_correctness(bool correct)
{
    audio_play_file(correct ? "ui_correct.wav" : "ui_wrong.wav");
}

static void play_score_result(uint8_t score, uint16_t total)
{
    audio_playf("ui_score_%u.wav", score);
    audio_playf("ui_total_%u.wav", total);
}

static void handle_answer(scoring_state_t *score, uint8_t question_index, uint8_t selected_index)
{
    bool correct = false;
    uint8_t score_value = scoring_submit(score, question_index, selected_index, &correct);
    play_correctness(correct);
    play_score_result(score_value, scoring_total(score));
}

static void switch_question(scoring_state_t *score, uint8_t *current, uint8_t next)
{
    if (next >= QUESTION_COUNT || next == *current) {
        return;
    }
    // Translation listening state is per visit. Clear the source and destination.
    scoring_clear_question(score, *current);
    scoring_clear_question(score, next);
    *current = next;
    play_question_number(*current);
}

static void handle_voice_answer(scoring_state_t *score, uint8_t question_index)
{
    audio_play_file("ui_listen.wav");
    uint8_t answer = voice_answer_listen();
    if (answer == VOICE_ANSWER_INVALID) {
        audio_play_file("ui_not_recognized.wav");
        return;
    }
    handle_answer(score, question_index, (uint8_t)(answer - 1));
}

static esp_err_t play_english_options(uint8_t question_index, bool chinese)
{
    const question_t *question = questions_get(question_index);
    if (question == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t result = ESP_OK;
    for (uint8_t i = 0; i < OPTION_COUNT; ++i) {
        if (audio_playf("ui_option_%u.wav", i + 1) != ESP_OK) {
            result = ESP_FAIL;
        }
        if (audio_playf("q%02u_%s_opt%u.wav", question_index + 1,
                        chinese ? "cn" : "en", i + 1) != ESP_OK) {
            result = ESP_FAIL;
        }
    }
    return result;
}

void app_main(void)
{
    ESP_ERROR_CHECK(board_hw_init());
    ESP_ERROR_CHECK(audio_player_init());

    scoring_state_t score;
    scoring_init(&score);
    uint8_t current_question = 0;
    input_state_t input;
    input_init(&input);

    esp_err_t voice_status = voice_answer_init();
    if (voice_status != ESP_OK) {
        ESP_LOGE(TAG, "voice answer unavailable; buttons still work (%s)",
                 esp_err_to_name(voice_status));
    }

    ESP_LOGI(TAG, "listen quiz started, question %u", current_question + 1);
    play_question_number(current_question);

    while (true) {
        uint32_t events = input_poll(&input);

        if (events & INPUT_ENCODER_CW) {
            uint8_t next = (uint8_t)((current_question + 1) % QUESTION_COUNT);
            switch_question(&score, &current_question, next);
        } else if (events & INPUT_ENCODER_CCW) {
            uint8_t next = (uint8_t)((current_question + QUESTION_COUNT - 1) % QUESTION_COUNT);
            switch_question(&score, &current_question, next);
        } else if (events & INPUT_EVENT_S1) {
            audio_playf("q%02u_en_stem.wav", current_question + 1);
        } else if (events & INPUT_EVENT_S2) {
            play_english_options(current_question, false);
        } else if (events & INPUT_EVENT_S3) {
            if (audio_playf("q%02u_cn_stem.wav", current_question + 1) == ESP_OK) {
                scoring_mark_chinese_stem(&score, current_question);
            }
        } else if (events & INPUT_EVENT_S4) {
            if (play_english_options(current_question, true) == ESP_OK) {
                scoring_mark_chinese_options(&score, current_question);
            }
        } else if (events & INPUT_EVENT_S5) {
            audio_playf("q%02u_en_opt1.wav", current_question + 1);
        } else if (events & INPUT_EVENT_S6) {
            audio_playf("q%02u_en_opt2.wav", current_question + 1);
        } else if (events & INPUT_EVENT_S7) {
            audio_playf("q%02u_en_opt3.wav", current_question + 1);
        } else if (events & INPUT_EVENT_S8) {
            audio_playf("q%02u_en_opt4.wav", current_question + 1);
        } else if (events & INPUT_EVENT_S9) {
            handle_voice_answer(&score, current_question);
        }

        input_delay_scan();
    }
}
