#ifndef LEFORUM_CLIENT_ARG_H_
#define LEFORUM_CLIENT_ARG_H_

#include <argp.h>

extern struct argp le_argp;
extern const char *argp_program_version;
extern const char *argp_program_bug_address;

struct arguments {

};

/**
 * @brief Argument parser, required by le_argp here (arhp.h). 
 * 
 * @param key Argument key 
 * @param arg Argument value 
 * @param state Context
 * @return Status 
 */
error_t parse_opt(int key, char *arg, struct argp_state *state);

#endif