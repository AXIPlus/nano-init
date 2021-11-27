#pragma once

typedef struct nanoinit_config_s {
    int checkAL;
} nanoinit_config_t;

const nanoinit_config_t *config_init(const char *filename, const char *json_object);
void config_free();
