#pragma once

#include <stdbool.h>

typedef enum {
    NI_NO_SPECIAL_MODE = 0,
    NI_COMMAND_RELOAD = 1,
} nanoinit_special_mode_t;

typedef struct nanoinit_arguments_s {
    char *config_file;
    char *config_json_object;
    char *log_path;
    bool manual_mode;
    nanoinit_special_mode_t special_mode;
    int verbosity_level;
} nanoinit_arguments_t;

const nanoinit_arguments_t *arguments_init(int argc, char **argv);
void arguments_free();
