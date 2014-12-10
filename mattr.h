#include <inttypes.h>

#include <map>
#include <vector>
#include <string>

class mem_symbol;
class mem_symbol_splay_tree;
class mattr;

class mem_symbol
{
    friend class mem_symbol_splay_tree;

    public:
        mem_symbol();
        mem_symbol(std::string n, uint64_t a, size_t s, size_t l);
        ~mem_symbol();

        bool contains(uint64_t addr);

    public:
        std::string get_name() {return name;}
        uint64_t get_addr() {return addr;}
        size_t get_sz() {return sz;}
        size_t get_len() {return len;}

        size_t get_index(uint64_t a) {return (a-addr)/sz;}

    private:
        std::string name;
        uint64_t addr;
        size_t sz;
        size_t len;
};

class mem_symbol_splay_tree
{

    public:
        mem_symbol_splay_tree();
        ~mem_symbol_splay_tree();

        void insert(mem_symbol m);
        void remove(mem_symbol m);
        mem_symbol* find_container(uint64_t addr);
        std::vector<mem_symbol>::iterator find_container_itr(uint64_t addr);
    private:
        std::vector<mem_symbol> all_mem_symbols;
};

class mattr
{
    public:
        mattr();
        ~mattr();

        void add_symbol(mem_symbol m);
        void add_symbol(std::string n, void *a, size_t s, size_t l);
        void add_symbol_vec(std::vector<mem_symbol> &v);

        mem_symbol* find_symbol(uint64_t addr) { return syms.find_container(addr); }

    private:
        mem_symbol_splay_tree syms;
};

