#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum { BODY_INPUT_KEY, BODY_INPUT_PRESS, BODY_INPUT_ROTATE } body_input_type_t;
typedef struct {
    body_input_type_t type;
    int index; // Keys 1..4; rotate index is signed full-cycle direction.
    bool down;
} body_input_event_t;
typedef struct { uint32_t a_edges, b_edges, invalid, dropped; int32_t pcnt_count; } body_input_stats_t;
void body_input_init(void);
bool body_input_read(body_input_event_t *event);
body_input_stats_t body_input_stats(void);
