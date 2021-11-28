#include "config.h"
#include <stdlib.h>

static nanoinit_config_t config = {0};

const nanoinit_config_t *config_init(const char *filename, const char *json_object) {
    (void)filename;
    (void)json_object;

    //checkAL when failing, zeroconfig must be saved to &config

    return &config;
}

void config_free() {
    for(int i = 0; i < config.application_count; i++) {
        free(config.applications[i].name);
        free(config.applications[i].path);

        for(int j = 0; j < config.applications[i].arg_count; j++) {
            free(config.applications[i].args[j]);
        }

        free(config.applications[i].stdout_path);
        free(config.applications[i].stderr_path);
    }

    free(config.applications);
    config.applications = 0;
    config.application_count = 0;
}
