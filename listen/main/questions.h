#pragma once

#include <stddef.h>
#include <stdint.h>

#define QUESTION_COUNT 20
#define OPTION_COUNT 4

typedef struct {
    const char *english_stem;
    const char *english_options[OPTION_COUNT];
    const char *chinese_stem;
    const char *chinese_options[OPTION_COUNT];
    uint8_t correct_index;
} question_t;

const question_t *questions_get(size_t index);
const char *questions_get_language(void);
size_t questions_text_count(void);
