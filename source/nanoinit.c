#define _GNU_SOURCE

#include "nanoinit.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <dirent.h>
#include <unistd.h>

static pid_t get_nanoinit_pid();

void nanoinit_send_reload() {
    pid_t pid = get_nanoinit_pid();
    if(pid < 0) {
        log_ni_error("nanoinit_send_reload() could not get nanoinit PID");
    }

    int result = kill(pid, SIGUSR1);
    if(result != 0) {
        log_ni_error("nanoinit_send_reload() could not send signal to nanoinit");
    }

    exit(0);
}

static pid_t get_nanoinit_pid() {
    
    pid_t res = (pid_t)-1;
    pid_t mypid = getpid();

    struct dirent *files;
    DIR *dir = opendir("/proc");
    if(dir == 0) {
        log_ni_error("get_nanoinit_pid() could not open /proc");
    }

    while((files = readdir(dir)) != 0) {
        if(files->d_type == 4) {
            int cpid = 0;
            int i = 0;
            while(files->d_name[i]) {
                if((files->d_name[i] >= '0') && (files->d_name[i] <= '9')) {
                    cpid *= 10;
                    cpid += files->d_name[i] - '0';
                }
                else {
                    cpid = -1;
                    break;
                }
                i++;
            }

            if(cpid > 0) {
                if((pid_t)cpid != mypid) {
                    char *procfname;
                    int a = asprintf(&procfname, "/proc/%d/cmdline", cpid);
                    (void)a;
                    if(procfname == 0) {
                        log_ni_error("get_nanoinit_pid() failed memory allocation");
                        return -1;
                    }

                    FILE *f = fopen(procfname, "r");
                    free(procfname);
                    if(f) {
                        size_t size;
                        char buffer[4096];
                        size = fread(buffer, sizeof(char), 4096, f);
                        fclose(f);

                        log("%s", buffer);

                        if(size) {
                            if(strstr(buffer, "nanoinit") != 0) {
                                //found!
                                res = (pid_t)cpid;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    closedir(dir);
    return res;
}
