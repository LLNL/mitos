#include "perfsmpl.h"
#include "mattr.h"

/*
 * SAMP : All programs must invoke from these static methods
 */
static perfsmpl m_perfsmpl;
static mattr m_mattr;

static void SAMP_set_sample_mode(sample_mode m) { m_perfsmpl.set_sample_mode(m); }
static void SAMP_set_handler(sample_handler_fn_t h) { m_perfsmpl.set_handler(h); }

static void SAMP_prepare() { m_perfsmpl.prepare(); }
static void SAMP_prepare(pid_t pid) { m_perfsmpl.prepare(pid); }
static void SAMP_begin_sampler() { m_perfsmpl.begin_sampler(); }
static void SAMP_end_sampler() { m_perfsmpl.end_sampler(); }

static void SAMP_add_symbol(std::string n, void *a, size_t s, size_t l) { m_mattr.add_symbol(n,a,s,l); }

static mem_symbol* SAMP_find_symbol(uint64_t addr) { return m_mattr.find_symbol(addr); }

