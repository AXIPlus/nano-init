#include <stdio.h>
#include "arguments.h"
#include "config.h"
#include "log.h"
#include "nanoinit.h"
#include "supervisor.h"

#include <stdlib.h>

int main(int argc, char **argv) {
    const nanoinit_arguments_t *arguments;
    const nanoinit_config_t *config;
    
    arguments = arguments_init(argc, argv);
    int rc = log_init(arguments->verbosity_level, arguments->log_path);
    if(rc != 0) {
        log_ni_error("log_init() failed");
    }

    if(arguments->special_mode != NI_NO_SPECIAL_MODE) {
        switch(arguments->special_mode) {
            case NI_COMMAND_RELOAD:
                nanoinit_send_reload();
                break;

            default:
                break;
        }

        exit(0);
    }


    config = config_init(arguments->config_file, arguments->config_json_object);
    if(config == 0) {
        log_ni_error("config_init() failed; using zero-config");
    }

    rc = supervisor_start(arguments, config);

    config_free();
    arguments_free();
    log_free();

    return rc;
}
