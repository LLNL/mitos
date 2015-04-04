#ifndef MITOS_H
#define MITOS_H

#include "perfsmpl.h"
#include "mattr.h"

/*
 * Mitos: performance event sampling library 
 * All programs must invoke from these functions.
 */

class mem_symbol;
class perf_event_sample;

#ifdef __cplusplus
extern "C" {
#endif

void Mitos_set_sample_mode(sample_mode m);
void Mitos_set_sample_period(uint64_t p);
void Mitos_set_sample_threshold(uint64_t t);
void Mitos_set_handler_fn(sample_handler_fn_t h, void *args=NULL);
void Mitos_set_end_fn(end_fn_t h, void *args=NULL);

void Mitos_prepare(pid_t pid=0);
void Mitos_begin_sampler();
void Mitos_end_sampler();

void Mitos_add_symbol(std::string n, void *a, size_t s, size_t l);

mem_symbol* Mitos_find_symbol(uint64_t addr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
