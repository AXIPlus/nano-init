#pragma once

#include <stdbool.h>

typedef struct nanoinit_application_config_s {
    char *name;
    char *path;

    int arg_count;
    char **args;

    bool autorestart;
    bool manual;

    char *stdout_path;
    char *stderr_path;
} nanoinit_application_config_t;

typedef struct nanoinit_config_s {
    nanoinit_application_config_t *applications;
    int application_count;
} nanoinit_config_t;

const nanoinit_config_t *config_init(const char *filename, const char *json_object);
void config_free();
