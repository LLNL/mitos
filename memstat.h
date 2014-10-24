#include <Dyninst/SymtabAPI.h>

#include "dbgmem.h"

class memstat
{
    public:
        memstat();
        ~memstat();

        std::vector<mem_symbol> get_static_symbols();

    private:
};
