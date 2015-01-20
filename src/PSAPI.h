#include "perfsmpl.h"
#include "mattr.h"

#include <string>

/*
 * PSAPI: Performance Sampling API
 * All programs must invoke from these functions.
 */
void PSAPI_set_sample_mode(sample_mode m);
void PSAPI_set_sample_period(uint64_t p);
void PSAPI_set_sample_threshold(uint64_t t);
void PSAPI_set_handler(sample_handler_fn_t h);

void PSAPI_prepare();
void PSAPI_prepare(pid_t pid);
void PSAPI_begin_sampler();
void PSAPI_end_sampler();

void PSAPI_add_symbol(std::string n, void *a, size_t s, size_t l);

mem_symbol* PSAPI_find_symbol(uint64_t addr);

