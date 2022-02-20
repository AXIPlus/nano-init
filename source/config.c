/**
 * MIT License
 * 
 * Copyright (c) 2022 AXIPlus / Adrian Lita / Alex Stancu - www.axiplus.com
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

static nanoinit_config_t config = {0};


#define EDJSON_PATH_MAX             32      //this practically depends on the tree depth of the JSON object; nanoinit needs only 3 levels when used without a JSON object
#define JSON_PARSE_BUFFER_SIZE      1024    //this should fit max build path length

typedef struct config_message_s {
    //input
    const char *json_object;
    uint32_t json_object_length;
    uint8_t json_object_components;

    //output
    int return_code;

    //state variables
    enum {
        CONFIG_STATE_SEARCHING = 0,
        CONFIG_STATE_FOUND,
        CONFIG_STATE_FINISHED,
    } state;

    char *current_app;
} config_message_t;

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
        else {
            log_ni_error("config_init() bad memory allocation");
        }
        fclose(f);
    }
    else {
        log_ni_error("config_init() JSON file %s could not be opened", filename);
    }

    if(json_content) {
        edJSON_path_t edJSON_path[EDJSON_PATH_MAX];
        config_message_t config_message;

        if(json_object == 0) {
            config_message.state = CONFIG_STATE_FOUND;
            config_message.json_object = "";
            config_message.json_object_components = 0;
            config_message.return_code = 0;     //by default, object is found
        }
        else {
            config_message.state = CONFIG_STATE_SEARCHING;
            config_message.json_object = json_object;
            config_message.json_object_components = 0;
            config_message.return_code = 1;     //by default, object is not found
        }
        
        config_message.json_object_length = strlen(config_message.json_object);
        
        //calculate json_object components
        size_t i = 0;
        while(i < strlen(config_message.json_object)) {
            if(config_message.json_object[i] == '/') {
                config_message.json_object_components++;
            }
            i++;
        }

        config_message.current_app = strdup("");
        if(config_message.current_app == 0) {
            log_ni_error("config_init() bad memory allocation");
            free(json_content);
            return &config;
        }

        int rc = edJSON_parse(json_content, edJSON_path, EDJSON_PATH_MAX, edJSON_callback, (void*)&config_message);
        free(json_content);
        free(config_message.current_app);

        bool has_config = false;
        if(rc != EDJSON_SUCCESS) {
            log_ni_error("config_init() JSON parsing error");
        }
        else {
            switch(config_message.return_code) {
                case 0:
                    has_config = true;
                    break;

                case 1:
                    log_ni_error("config_init() JSON parsing: object '%s' not found in config", json_object);
                    break;

                case 2:
                    log_ni_error("config_init() JSON parsing: bad config parameters");
                    break;

                case 3:
                    log_ni_error("config_init() JSON parsing: no memory left");
                    break;

                case 4:
                    log_ni_error("config_init() JSON parsing: invalid string object found");
                    break;

                default:
                    log_ni_error("config_init() JSON parsing ok; unknown error");
                    break;
            }
        }

        //validate config data
        if(has_config) {
            for(int i = 0; i < config.application_count; i++) {
                if(config.applications[i].name == 0) {
                    has_config = false;
                    break;
                }

                if(config.applications[i].path == 0) {
                    has_config = false;
                    break;
                }
            }
        }
        
        //zero out config
        if(!has_config) {
            config_free();
            memset(&config, 0, sizeof(nanoinit_config_t));
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
    config_message_t *config_message = (config_message_t *)private;
    

    char pv[JSON_PARSE_BUFFER_SIZE];
    int rc = edJSON_build_path_string(pv, JSON_PARSE_BUFFER_SIZE, path, path_size);
    if(rc < 0) {
        log_ni_error("edJSON_callback() edJSON_build_path_string failed");
        config_message->return_code = 3;    //no memory left
        return rc;  //stop parsing
    }

    
switch_config_message_state:
    switch(config_message->state) {
        case CONFIG_STATE_SEARCHING: {
            if(config_message->json_object_length) {
                if((strlen(pv) > config_message->json_object_length) && (strncmp(config_message->json_object, pv, config_message->json_object_length) == 0)) {
                    config_message->state = CONFIG_STATE_FOUND;
                    config_message->return_code = 0;
                    goto switch_config_message_state;
                }
            }
        } break;

        case CONFIG_STATE_FOUND: {
            if(config_message->json_object_length) {
                if((strlen(pv) <= config_message->json_object_length) || (strncmp(config_message->json_object, pv, config_message->json_object_length) != 0)) {
                    config_message->state = CONFIG_STATE_FINISHED;
                    goto switch_config_message_state;
                }
            }
            
            //parse
            if(path_size <= config_message->json_object_components) {
                config_message->return_code = 2;
                return 1;
            }

            char current_value[JSON_PARSE_BUFFER_SIZE];
            size_t component = config_message->json_object_components;

            if(path[component].index >= 0) {
                config_message->return_code = 2;
                return 1;
            }

            int rc = edJSON_string_unescape(current_value, JSON_PARSE_BUFFER_SIZE, path[component].value, path[component].value_size);
            if(rc < EDJSON_SUCCESS) {
                config_message->return_code = 4;
                return 1;
            }

            if(strcmp(config_message->current_app, current_value) != 0) {
                free(config_message->current_app);
                config_message->current_app = strdup(current_value);
                if(config_message->current_app == 0) {
                    log_ni_error("edJSON_callback() bad memory allocation");
                    config_message->return_code = 3;
                    return 1;
                }

                config.application_count++;
                config.applications = (nanoinit_application_config_t *)realloc(config.applications, sizeof(nanoinit_application_config_t) * config.application_count);
                if(config.applications == 0) {
                    log_ni_error("edJSON_callback() bad memory allocation");
                    config_message->return_code = 3;
                    return 1;
                }

                //init memory
                memset(&config.applications[config.application_count - 1], 0, sizeof(nanoinit_application_config_t));

                //set name
                config.applications[config.application_count - 1].name = strdup(current_value);
                if(config.applications[config.application_count - 1].name == 0) {
                    log_ni_error("edJSON_callback() bad memory allocation");
                    config_message->return_code = 3;
                    return 1;
                }
            }

            //move on to first property of the application
            component++;
            if(component >= path_size) {
                config_message->return_code = 2;
                return 1;
            }

            if(path[component].index >= 0) {
                config_message->return_code = 2;
                return 1;
            }

            rc = edJSON_string_unescape(current_value, JSON_PARSE_BUFFER_SIZE, path[component].value, path[component].value_size);
            if(rc < EDJSON_SUCCESS) {
                config_message->return_code = 4;
                return 1;
            }

            component++;

            //if component is path
            if(strcmp(current_value, "path") == 0) {
                if(path_size != component) {
                    log_ni_error("edJSON_callback() path should not have child objects for app %s", config.applications[config.application_count - 1].name);
                    config_message->return_code = 2;
                    return 1;
                }

                if(value.value_type != EDJSON_VT_STRING) {
                    log_ni_error("edJSON_callback() path value type should be string for app %s", config.applications[config.application_count - 1].name);
                    config_message->return_code = 2;
                    return 1;
                }

                rc = edJSON_string_unescape(current_value, JSON_PARSE_BUFFER_SIZE, value.value.string.value, value.value.string.value_size);
                if(rc < EDJSON_SUCCESS) {
                    config_message->return_code = 4;
                    return 1;
                }

                //set path
                config.applications[config.application_count - 1].path = strdup(current_value);
                if(config.applications[config.application_count - 1].path == 0) {
                    log_ni_error("edJSON_callback() bad memory allocation");
                    config_message->return_code = 3;
                    return 1;
                }
            }

            //if component is args
            else if(strcmp(current_value, "args") == 0) {
                if(path[component].index >= 0) {
                    component++;
                }

                if(path_size != component) {
                    log_ni_error("edJSON_callback() argument itself should not have child objects for app %s", config.applications[config.application_count - 1].name);
                    config_message->return_code = 2;
                    return 1;
                }

                if(value.value_type != EDJSON_VT_STRING) {
                    log_ni_error("edJSON_callback() argument value type should be string for app %s", config.applications[config.application_count - 1].name);
                    config_message->return_code = 2;
                    return 1;
                }

                rc = edJSON_string_unescape(current_value, JSON_PARSE_BUFFER_SIZE, value.value.string.value, value.value.string.value_size);
                if(rc < EDJSON_SUCCESS) {
                    config_message->return_code = 4;
                    return 1;
                }

                //add argument
                config.applications[config.application_count - 1].arg_count++;
                config.applications[config.application_count - 1].args = (char **)realloc(config.applications[config.application_count - 1].args, sizeof(char *) * config.applications[config.application_count - 1].arg_count);
                if(config.applications[config.application_count - 1].args == 0) {
                    log_ni_error("edJSON_callback() bad memory allocation");
                    config_message->return_code = 3;
                    return 1;
                }

                config.applications[config.application_count - 1].args[config.applications[config.application_count - 1].arg_count - 1] = strdup(current_value);
                if(config.applications[config.application_count - 1].args[config.applications[config.application_count - 1].arg_count - 1] == 0) {
                    log_ni_error("edJSON_callback() bad memory allocation");
                    config_message->return_code = 3;
                    return 1;
                }
            }

            //if component is autorestart
            else if(strcmp(current_value, "autorestart") == 0) {
                if(path_size != component) {
                    log_ni_error("edJSON_callback() autorestart should not have child objects for app %s", config.applications[config.application_count - 1].name);
                    config_message->return_code = 2;
                    return 1;
                }

                if(value.value_type != EDJSON_VT_BOOL) {
                    log_ni_error("edJSON_callback() autorestart value type should be boolean for app %s", config.applications[config.application_count - 1].name);
                    config_message->return_code = 2;
                    return 1;
                }

                //set autorestart
                config.applications[config.application_count - 1].autorestart = value.value.boolean;
            }

            //if component is manual
            else if(strcmp(current_value, "manual") == 0) {
                if(path_size != component) {
                    log_ni_error("edJSON_callback() manual should not have child objects for app %s", config.applications[config.application_count - 1].name);
                    config_message->return_code = 2;
                    return 1;
                }

                if(value.value_type != EDJSON_VT_BOOL) {
                    log_ni_error("edJSON_callback() manual value type boolean be string for app %s", config.applications[config.application_count - 1].name);
                    config_message->return_code = 2;
                    return 1;
                }

                //set manual
                config.applications[config.application_count - 1].manual = value.value.boolean;
            }

            //if component is stdout
            else if(strcmp(current_value, "stdout") == 0) {
                if(path_size != component) {
                    log_ni_error("edJSON_callback() stdout should not have child objects for app %s", config.applications[config.application_count - 1].name);
                    config_message->return_code = 2;
                    return 1;
                }

                if(value.value_type != EDJSON_VT_STRING) {
                    log_ni_error("edJSON_callback() stdout value type should be string for app %s", config.applications[config.application_count - 1].name);
                    config_message->return_code = 2;
                    return 1;
                }

                rc = edJSON_string_unescape(current_value, JSON_PARSE_BUFFER_SIZE, value.value.string.value, value.value.string.value_size);
                if(rc < EDJSON_SUCCESS) {
                    config_message->return_code = 4;
                    return 1;
                }

                //set stdout_path
                config.applications[config.application_count - 1].stdout_path = strdup(current_value);
                if(config.applications[config.application_count - 1].stdout_path == 0) {
                    log_ni_error("edJSON_callback() bad memory allocation");
                    config_message->return_code = 3;
                    return 1;
                }
            }

            //if component is stderr
            else if(strcmp(current_value, "stderr") == 0) {
                if(path_size != component) {
                    log_ni_error("edJSON_callback() stderr should not have child objects for app %s", config.applications[config.application_count - 1].name);
                    config_message->return_code = 2;
                    return 1;
                }

                if(value.value_type != EDJSON_VT_STRING) {
                    log_ni_error("edJSON_callback() stderr value type should be string for app %s", config.applications[config.application_count - 1].name);
                    config_message->return_code = 2;
                    return 1;
                }

                rc = edJSON_string_unescape(current_value, JSON_PARSE_BUFFER_SIZE, value.value.string.value, value.value.string.value_size);
                if(rc < EDJSON_SUCCESS) {
                    config_message->return_code = 4;
                    return 1;
                }

                //set stderr_path
                config.applications[config.application_count - 1].stderr_path = strdup(current_value);
                if(config.applications[config.application_count - 1].stderr_path == 0) {
                    log_ni_error("edJSON_callback() bad memory allocation");
                    config_message->return_code = 3;
                    return 1;
                }
            }

            //if component is anything lese
            else {
                config_message->return_code = 2;    //invalid parameter
                return 1;
            }
        } break;

        case CONFIG_STATE_FINISHED: {
            config_message->return_code = 0;
            return 1;   //stop parsing
        }

        default:
            break;
    }

    return 0;
}
