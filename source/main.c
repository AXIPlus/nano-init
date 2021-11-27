#include <stdio.h>
#include "arguments.h"
#include "config.h"
#include "log.h"
#include "supervisor.h"

int main(int argc, char **argv) {
    const nanoinit_arguments_t *arguments;
    const nanoinit_config_t *config;
    
    arguments = arguments_init(argc, argv);
    int rc = log_init(arguments->verbosity_level, arguments->log_path);
    if(rc != 0) {
        log_ni_error("log_init() failed");
    }
    config = config_init(arguments->config_file, arguments->config_json_object);
    rc = supervisor_start(arguments, config);

    config_free();
    arguments_free();
    log_free(); 
    return rc;
}
