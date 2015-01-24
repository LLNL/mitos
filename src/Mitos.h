#include "perfsmpl.h"
#include "mattr.h"

#include <string>

/*
 * Mitos: performance event sampling library 
 * All programs must invoke from these functions.
 */
void Mitos_set_sample_mode(sample_mode m);
void Mitos_set_sample_period(uint64_t p);
void Mitos_set_sample_threshold(uint64_t t);
void Mitos_set_handler(sample_handler_fn_t h);

void Mitos_prepare();
void Mitos_prepare(pid_t pid);
void Mitos_begin_sampler();
void Mitos_end_sampler();

void Mitos_add_symbol(std::string n, void *a, size_t s, size_t l);

mem_symbol* Mitos_find_symbol(uint64_t addr);

