#include <stdio.h>
#include "arguments.h"
#include "config.h"
#include "supervisor.h"

int main(int argc, char **argv) {
    const nanoinit_arguments_t *arguments;
    const nanoinit_config_t *config;
    
    arguments = arguments_init(argc, argv);
    config = config_init(arguments->config_file, arguments->config_json_object);
    int rc = supervisor_start(arguments, config);

    config_free();
    arguments_free();    
    return rc;
}
