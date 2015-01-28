#include "Mitos.h"

static perfsmpl m_perfsmpl;
static mattr m_mattr;

void Mitos_set_sample_mode(sample_mode m) { m_perfsmpl.set_sample_mode(m); }
void Mitos_set_sample_period(uint64_t p) { m_perfsmpl.set_sample_period(p); }
void Mitos_set_sample_threshold(uint64_t t) { m_perfsmpl.set_sample_threshold(t); }
void Mitos_set_handler_fn(sample_handler_fn_t h, void* args) { m_perfsmpl.set_handler_fn(h,args); }
void Mitos_set_end_fn(end_fn_t h, void* args) { m_perfsmpl.set_end_fn(h,args); }

void Mitos_prepare() { m_perfsmpl.prepare(); }
void Mitos_prepare(pid_t pid) { m_perfsmpl.prepare(pid); }
void Mitos_begin_sampler() { m_perfsmpl.begin_sampler(); }
void Mitos_end_sampler() { m_perfsmpl.end_sampler(); }

void Mitos_add_symbol(std::string n, void *a, size_t s, size_t l) { m_mattr.add_symbol(n,a,s,l); }

mem_symbol* Mitos_find_symbol(uint64_t addr) { return m_mattr.find_symbol(addr); }
