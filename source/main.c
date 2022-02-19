/**
 * MIT License
 * 
 * Copyright (c) 2021 AXIPlus / Adrian Lita / Alex Stancu - www.axiplus.com
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * */

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
    
    //load and parse arguments; if any argument is not present, a default value is assumed
    arguments = arguments_init(argc, argv);

    //initialize logger based on verbosity_level and log_path returned by arguments
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
                log_ni_error("invalid special mode");
                break;
        }

        exit(0);
    }

    //load config from config file; config file may not be nanoinit-specific, 
    config = config_init(arguments->config_file, arguments->config_json_object);
    if(config == 0) {
        log_ni_error("config_init() failed; using zero-config");
    }

    //start supervisor; this function returns only on nanoinit exit; on fork-exec the freeing is taken care of there
    rc = supervisor_start(arguments, config);

    //free resources
    config_free();
    arguments_free();
    log_free();

    return rc;
}
