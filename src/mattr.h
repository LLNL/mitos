#ifndef MATTR_H
#define MATTR_H

#include <stdlib.h>
#include <inttypes.h>

#include <map>
#include <vector>

class mem_symbol;
class mem_symbol_splay_tree;
class mattr;

class mem_symbol
{
    friend class mem_symbol_splay_tree;

    public:
        mem_symbol();
        mem_symbol(const char* n, uint64_t a, size_t s, size_t *d, unsigned int nd);
        ~mem_symbol();

        bool contains(uint64_t addr);

    public:
        char* get_name() {return name;}
        uint64_t get_addr() {return addr;}
        size_t get_sz() {return sz;}
        size_t get_len() {return len;}
        unsigned int get_num_dims() {return num_dims;}

        void get_index(uint64_t a, size_t *d);

    private:
        char* name;
        uint64_t addr;
        size_t sz;
        size_t len;
        size_t *dims;
        unsigned int num_dims;
};

class mem_symbol_splay_tree
{

    public:
        mem_symbol_splay_tree();
        ~mem_symbol_splay_tree();

        void insert(mem_symbol m);
        void remove(mem_symbol m);
        mem_symbol* find_container(uint64_t addr);
		mem_symbol* find_symbol(const char* name);
        std::vector<mem_symbol>::iterator find_container_itr(uint64_t addr);
        std::vector<mem_symbol>::iterator find_symbol_itr(const char* name);
    private:
        std::vector<mem_symbol> all_mem_symbols;
};

class mattr
{
    public:
        mattr();
        ~mattr();

        void add_symbol(mem_symbol m);
        void add_symbol(const char* n, void *a, size_t s, size_t *d, unsigned int nd);
        void add_symbol_vec(std::vector<mem_symbol> &v);

		void remove_symbol(const char* name);

        mem_symbol* find_symbol(uint64_t addr) { return syms.find_container(addr); }

    private:
        mem_symbol_splay_tree syms;
};

#endif
