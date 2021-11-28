/**
 * MIT License
 * 
 * Copyright (c) 2021 AXIPlus / Adrian Lita - www.axiplus.com
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

#define _GNU_SOURCE         //for asprintf

#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

static int instances = 0;
static int app_verbosity_level = 0;
static FILE *log_file = 0;

int log_init(int verbosity_level, const char *log_path) {
    instances++;
    if(instances > 1) {
        log_ni_error("log_init() called too many times");
        return -1;
    }

    if((verbosity_level < 0) || (verbosity_level > 2)) {
        log_ni_error("log_init() invalid verbosity level: %d", verbosity_level);
        return -2;
    }

    app_verbosity_level = verbosity_level;
    if(log_path) {
        log_file = fopen(log_path, "w");
        if(log_file == 0) {
            log_ni_error("log_init() could not open logfile '%s' for writing; logging to file is disabled", log_path);
            return -3;
        }
    }

    return 0;
}

void log_free(void) {
    if(log_file) {
        fclose(log_file);
        log_file = 0;
    }

    instances--;
    app_verbosity_level = 0;
}

void _log_add(int verbosity_level, const char *format, ...) {
    if((verbosity_level < 0) || (verbosity_level > 2)) {
        log_ni_error("_log_add() invalid verbosity level: %d; assuming NI-ERROR", verbosity_level);
        verbosity_level = 0;
    }

    if(format == 0) {
        log_ni_error("_log_add() invalid format parameter");
        return;
    }

    
    struct timeval tv;
    int result = gettimeofday(&tv, 0);
    if(result != 0) {
        static int shown = 0;
        if(shown == 0) {
            log_ni_error("_log_add() could not get time");  //show this just one time to prevent recursion
            shown = 1;
        }
        void *result = memset(&tv, 0, sizeof(tv));
        (void)result;
    }

    unsigned long long ts_sec = (unsigned long long)(tv.tv_sec);
    unsigned int ts_msec = (unsigned int)(tv.tv_usec) / 1000;
    
    char *format_with_timestamp = 0;
    result = asprintf(&format_with_timestamp, "[%llu.%03u] [nanoinit] %s\n", ts_sec, ts_msec, format);
    (void)result;
    
    if(format_with_timestamp == 0) {
        //asprintf failed, fallback
        format_with_timestamp = (char *)format;
        if(verbosity_level != 0) {  //prevent recursion
            log_ni_error("_log_add() asprintf() failed to generate string");
        }
    }

    
    va_list arg;

    //log to file
    if(log_file) {
        va_start(arg, format);
        vfprintf(log_file, format_with_timestamp, arg);
        fflush(log_file);
        va_end(arg);
    }

    //print
    if(verbosity_level <= app_verbosity_level) {
        FILE *output = stderr;
        if(verbosity_level > 1) {
            output = stdout;
        }

        va_start(arg, format);
        vfprintf(output, format_with_timestamp, arg);
        va_end(arg);
        if(format_with_timestamp == format) {
            fprintf(output, "\n"); //newline is missing
        }
    }

    if(format_with_timestamp != format) {
        free(format_with_timestamp);
    }
}
