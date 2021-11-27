#pragma once

typedef struct nanoinit_arguments_s {
    char *config_file;
    char *config_json_object;
    char *log_path;
    int verbosity_level;
} nanoinit_arguments_t;

const nanoinit_arguments_t *arguments_init(int argc, char **argv);
void arguments_free();
