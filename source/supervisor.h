#pragma once

#include "arguments.h"
#include "config.h"

int supervisor_start(const nanoinit_arguments_t *arguments, const nanoinit_config_t *config);

void supervisor_send_reload();  //looks for main nanoinit and sends SIGUSR1 signal to reload
