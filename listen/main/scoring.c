#include "scoring.h"

#include "questions.h"
#include <string.h>

#define MAX_TOTAL_SCORE 100

void scoring_init(scoring_state_t *state)
{
    memset(state, 0, sizeof(*state));
}

void scoring_mark_chinese_stem(scoring_state_t *state, uint8_t question_index)
{
    if (question_index < QUESTION_COUNT) {
        state->question_states[question_index].heard_chinese_stem = true;
    }
}

void scoring_mark_chinese_options(scoring_state_t *state, uint8_t question_index)
{
    if (question_index < QUESTION_COUNT) {
        state->question_states[question_index].heard_chinese_options = true;
    }
}

void scoring_clear_question(scoring_state_t *state, uint8_t question_index)
{
    if (question_index < QUESTION_COUNT) {
        state->question_states[question_index].heard_chinese_stem = false;
        state->question_states[question_index].heard_chinese_options = false;
    }
}

uint8_t scoring_submit(scoring_state_t *state, uint8_t question_index,
                       uint8_t selected_index, bool *correct)
{
    if (question_index >= QUESTION_COUNT || selected_index >= OPTION_COUNT) {
        if (correct != NULL) {
            *correct = false;
        }
        return 0;
    }

    const question_t *question = questions_get(question_index);
    bool is_correct = selected_index == question->correct_index;
    const question_score_state_t *q = &state->question_states[question_index];
    uint8_t score = 0;

    if (is_correct) {
        if (!q->heard_chinese_stem && !q->heard_chinese_options) {
            score = 5;
        } else if (q->heard_chinese_stem != q->heard_chinese_options) {
            score = q->heard_chinese_stem ? 4 : 3;
        } else {
            score = 2;
        }
    } else {
        score = (q->heard_chinese_stem || q->heard_chinese_options) ? 1 : 0;
    }

    // Repeated answers intentionally earn points again, but the total is capped.
    if (state->total_score < MAX_TOTAL_SCORE) {
        uint16_t addition = (uint16_t)score;
        uint16_t room = (uint16_t)(MAX_TOTAL_SCORE - state->total_score);
        state->total_score = (uint16_t)(state->total_score +
                                        (addition < room ? addition : room));
    }

    if (correct != NULL) {
        *correct = is_correct;
    }
    return score;
}

uint16_t scoring_total(const scoring_state_t *state)
{
    return state->total_score;
}
