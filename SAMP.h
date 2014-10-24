#include "perfsmpl.h"
#include "dbgmem.h"

/*
 * SAMP : All programs must invoke from these static methods
 */
namespace SAMP
{
    static perfsmpl m_perfsmpl;
    static dbgmem m_dbgmem;
    static memstat m_memstat;

    static void set_sample_mode(sample_mode m) { m_perfsmpl.set_sample_mode(m); }
    static void set_handler(sample_handler_fn_t h) { m_perfsmpl.set_handler(h); }

    static void prepare() { m_perfsmpl.prepare(); }
    static void begin_m_perfsmpl() { m_perfsmpl.begin_m_perfsmpl(); }
    static void end_m_perfsmpl() { m_perfsmpl.end_m_perfsmpl(); }

    static void add_symbol(std::string n, void *a, size_t s, size_t l) { m_dbgmem.add_symbol(n,a,s,l); }
    static void add_static_symbols() { m_dbgmem.add_symbol_vec(m_memstat.get_static_symbols()); }

    static mem_symbol* find_symbol(uint64_t addr) { return m_dbgmem.find_symbol(addr); }
}

