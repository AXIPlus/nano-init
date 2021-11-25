#pragma once

typedef struct nanoinit_config_s {
    char *config_file;
    char *config_json_object;
    char *log_path;
} nanoinit_config_t;

const nanoinit_config_t *config_init(const char *filename, const char *json_object);
void config_free();
