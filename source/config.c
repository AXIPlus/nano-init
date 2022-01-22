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

#include "config.h"
#include <stdlib.h>
#include "edJSON/edJSON.h"

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
