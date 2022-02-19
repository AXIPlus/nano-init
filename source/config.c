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
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "edJSON/edJSON.h"

#define EDJSON_PATH_MAX             32
#define JSON_PATH_STRING_SIZE       1024

static nanoinit_config_t config = {0};

static int edJSON_callback(const edJSON_path_t *path, size_t path_size, edJSON_value_t value, void *private);

const nanoinit_config_t *config_init(const char *filename, const char *json_object) {
    char *json_content = 0;
    long length = 0;
    FILE *f = fopen(filename, "rb");
    if(f) {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        json_content = (char*)malloc(sizeof(char) * (length + 1));
        if(json_content) {
            size_t s = fread(json_content, 1, length, f);
            (void)s;
            json_content[length] = 0;
        }
        fclose(f);
    }
    else {
        log_ni_error("config_init() JSON file %s could not be opened", filename);
    }

    if(json_object == 0) {
        json_object = "";
    }

    if(json_content) {
        edJSON_path_t edJSON_path[EDJSON_PATH_MAX];
        int rc = edJSON_parse(json_content, edJSON_path, EDJSON_PATH_MAX, edJSON_callback, (void*)json_object);
        free(json_content);

        if(rc != EDJSON_SUCCESS) {
            memset(&config, 0, sizeof(nanoinit_config_t));

            log_ni_error("config_init() JSON parsing error");
        }
    }

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

static int edJSON_callback(const edJSON_path_t *path, size_t path_size, edJSON_value_t value, void *private) {
    const char *json_object = (const char *)private;
    (void)json_object;  //checkAL

    char json_path[JSON_PATH_STRING_SIZE];
    int rc = edJSON_build_path_string(json_path, JSON_PATH_STRING_SIZE, path, path_size);
    if(rc < 0) {
        log_ni_error("edJSON_build_path_string failed to buiild path [%d]", rc);
    }



    printf("%s | value: ", json_path);

    char pv[128];
    if(value.value_type == EDJSON_VT_STRING) {
        int rc = edJSON_string_unescape(pv, 128, value.value.string.value, value.value.string.value_size);
        if(rc < 0) {
            printf("error on edJSON_string_unescape\n");
            return rc;  //stop parsing
        }

        printf("\"%s\"", pv);
    }
    else if(value.value_type == EDJSON_VT_INTEGER) {
        printf("%d", value.value.integer);
    }
    else if(value.value_type == EDJSON_VT_DOUBLE) {
        printf("%.4f", value.value.floating);
    }
    else if(value.value_type == EDJSON_VT_BOOL) {
        printf("%s", value.value.boolean ? "true" : "false");
    }
    else if(value.value_type == EDJSON_VT_NULL) {
        printf("null");
    }
    printf("\n");

    

    return 0;
}
