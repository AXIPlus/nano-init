#include "arguments.h"
#include <argp.h>
#include <string.h>
#include <stdlib.h>

static nanoinit_arguments_t arguments = {0};

const char *argp_program_version = "nanoinit v0.0.1 build 123451234";
const char *argp_program_bug_address = "<adrian@axiplus.com>";
static char doc[] = "nanoinit - for documentation and usage check https://github.com/AXIPlus/nanoinit";

static struct argp_option options[] = {
    { "config-file", 'c', "/path/to/config.json", 0, "Specifies the configuration JSON file. Default value is null, which means that no apps will be run, but nanoinit will sleep for infinity and wait for a kill signal.", 0 },
    { "config-json-object", 'j', "nanoinit-settings", 0, "Specifies the parent JSON object. Default value is null, which means that it will look directly into the root of the JSON file.", 0},
    { "log-path", 'l', "/path/to/log.txt", 0, "Specified the path for writing log-files. Default only uses stderr and stdout for logging.", 0 },
    { "manual-mode", 'm', 0, 0, "Enable manual mode. This option is recommended to be set via the NANOINIT_MANUAL_MODE environment variable, as it is more useful that way. Default is manual-mode disabled.", 0 },
    { "reload", 'r', 0, 0, "Looks for top nanoinit process and sends a SIGSUSR1 signal to it, forcing it to terminate all apps, reload config and restart apps.", 0 },
    { "verbose", 'v', "0-2", 0, "Specified application print verbosity level. Values are 0(nanoinit ERR)-default, 1(application ERR), 2(LOG).", 0 },
    { 0 } 
};

static error_t argp_parse_cb(int key, char *arg, struct argp_state *state);

const nanoinit_arguments_t *arguments_init(int argc, char **argv) {
    //parse provided command line arguments
    struct argp argp = { options, argp_parse_cb, 0, doc, 0, 0, 0 };
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    //check manual mode enviroment variable
    char *manual_mode_env = getenv("NANOINIT_MANUAL_MODE");
    if(manual_mode_env != 0) {
        arguments.manual_mode = true;
    }

    return &arguments;
}

static error_t argp_parse_cb(int key, char *arg, struct argp_state *state) {
    nanoinit_arguments_t *iter_arguments = state->input;
    switch (key) {
        case 'c':
            if(arg == 0) {
                return ARGP_ERR_UNKNOWN;
            }

            iter_arguments->config_file = strdup(arg);
            if(iter_arguments->config_file == 0) {
                return ARGP_ERR_UNKNOWN;
            }
            break;

        case 'j':
            if(arg == 0) {
                return ARGP_ERR_UNKNOWN;
            }

            iter_arguments->config_json_object = strdup(arg);
            if(iter_arguments->config_json_object == 0) {
                return ARGP_ERR_UNKNOWN;
            }
            break;

        case 'l':
            if(arg == 0) {
                return ARGP_ERR_UNKNOWN;
            }

            iter_arguments->log_path = strdup(arg);
            if(iter_arguments->log_path == 0) {
                return ARGP_ERR_UNKNOWN;
            }
            break;

        case 'm':
            iter_arguments->manual_mode = true;
            break;

        case 'r':
            iter_arguments->special_mode = NI_COMMAND_RELOAD;
            break;

        case 'v':
            if(arg == 0) {
                return ARGP_ERR_UNKNOWN;
            }

            iter_arguments->verbosity_level  = arg[0] - '0';
            if((iter_arguments->verbosity_level < 0) || (iter_arguments->verbosity_level > 2)) {
                //invalid verbosity level
                argp_usage(state);
            }
            break;

        case ARGP_KEY_ARG:
            if (state->arg_num >= 2) {
                argp_usage(state);
            }
            break;

        case ARGP_KEY_END:

            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    
    return 0;
}

void arguments_free() {
    free(arguments.config_file);
    free(arguments.config_json_object);
    free(arguments.log_path);
}
