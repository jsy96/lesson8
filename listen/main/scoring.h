#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool heard_chinese_stem;
    bool heard_chinese_options;
} question_score_state_t;

typedef struct {
    uint16_t total_score;
    question_score_state_t question_states[20];
} scoring_state_t;

void scoring_init(scoring_state_t *state);
void scoring_mark_chinese_stem(scoring_state_t *state, uint8_t question_index);
void scoring_mark_chinese_options(scoring_state_t *state, uint8_t question_index);
void scoring_clear_question(scoring_state_t *state, uint8_t question_index);
uint8_t scoring_submit(scoring_state_t *state, uint8_t question_index,
                       uint8_t selected_index, bool *correct);
uint16_t scoring_total(const scoring_state_t *state);
