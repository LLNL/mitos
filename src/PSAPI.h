#include "perfsmpl.h"
#include "mattr.h"

/*
 * PSAPI : All programs must invoke from these static methods
 */
static perfsmpl m_perfsmpl;
static mattr m_mattr;

static void PSAPI_set_sample_mode(sample_mode m) { m_perfsmpl.set_sample_mode(m); }
static void PSAPI_set_sample_period(uint64_t p) { m_perfsmpl.set_sample_period(p); }
static void PSAPI_set_sample_threshold(uint64_t t) { m_perfsmpl.set_sample_threshold(t); }
static void PSAPI_set_handler(sample_handler_fn_t h) { m_perfsmpl.set_handler(h); }

static void PSAPI_prepare() { m_perfsmpl.prepare(); }
static void PSAPI_prepare(pid_t pid) { m_perfsmpl.prepare(pid); }
static void PSAPI_begin_sampler() { m_perfsmpl.begin_sampler(); }
static void PSAPI_end_sampler() { m_perfsmpl.end_sampler(); }

static void PSAPI_add_symbol(std::string n, void *a, size_t s, size_t l) { m_mattr.add_symbol(n,a,s,l); }

static mem_symbol* PSAPI_find_symbol(uint64_t addr) { return m_mattr.find_symbol(addr); }

