#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <argp.h>
#include <sys/time.h>

#include "lib/constants.h"

extern struct argp le_argp;
extern const char *argp_program_version;
extern const char *argp_program_bug_address;

struct arguments {

};

/**
 * @brief Argument parser, required by le_argp here (arhp.h) 
 * 
 * @param key Argument key 
 * @param arg Argument value 
 * @param state Context
 * @return Status 
 */
error_t parse_opt(int key, char *arg, struct argp_state *state);