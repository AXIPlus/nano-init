#include "config.h"

static nanoinit_config_t config = {0};

const nanoinit_config_t *config_init(const char *filename, const char *json_object) {
    (void)filename;
    (void)json_object;

    return &config;
}

void config_free() {

}
