#include "mattr.h"

#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>


mem_symbol::mem_symbol()
{

}

mem_symbol::mem_symbol(const char* n, uint64_t a, size_t s, size_t *d, unsigned int nd)
    :
    addr(a),
    sz(s),
    num_dims(nd)
{
    name = strdup(n);
    dims = (size_t*)malloc(sizeof(size_t)*num_dims);

    len = 1;
    for(int i=0; i<num_dims; i++)
    {
        dims[i] = d[i];
        len *= dims[i];
    }
}

mem_symbol::~mem_symbol()
{

}

bool mem_symbol::contains(uint64_t addr)
{
    return this->addr <= addr &&  addr < this->addr+sz*len;
}

void mem_symbol::get_index(uint64_t a, size_t *d) 
{
    unsigned int idx = (a-addr)/sz;

    for(int i=0; i<num_dims; i++)
    {
        d[i] = idx % dims[i];
        idx = idx / dims[i];
    }
}

mem_symbol_splay_tree::mem_symbol_splay_tree()
{

}

mem_symbol_splay_tree::~mem_symbol_splay_tree()
{

}

void mem_symbol_splay_tree::insert(mem_symbol m)
{
    all_mem_symbols.push_back(m);

}

void mem_symbol_splay_tree::remove(mem_symbol m)
{
    std::vector<mem_symbol>::iterator r;
    r = find_container_itr(m.addr);
    all_mem_symbols.erase(r);
}

mem_symbol* mem_symbol_splay_tree::find_container(uint64_t addr)
{
    std::vector<mem_symbol>::iterator r;
    r = find_container_itr(addr);
    if(r == all_mem_symbols.end())
        return NULL;
    return (mem_symbol*)&(*r);
}

std::vector<mem_symbol>::iterator
mem_symbol_splay_tree::find_container_itr(uint64_t addr)
{
    std::vector<mem_symbol>::iterator r;
    for(r=all_mem_symbols.begin();r<all_mem_symbols.end();r++)
    {
        if(r->contains(addr))
            return r;
    }
    return all_mem_symbols.end();
}

mem_symbol* mem_symbol_splay_tree::find_symbol(const char* name)
{
	std::vector<mem_symbol>::iterator r = find_symbol_itr(name);
	if (r == all_mem_symbols.end())
		return NULL;
	return (mem_symbol*) &(*r);
}

std::vector<mem_symbol>::iterator
mem_symbol_splay_tree::find_symbol_itr(const char* name)
{
	std::vector<mem_symbol>::iterator r;
	for (r = all_mem_symbols.begin(); r != all_mem_symbols.end(); r++)
	{
		if (std::string(r->get_name()) == std::string(name))
			return r;
	}

	return all_mem_symbols.end();
}

mattr::mattr()
{

}

mattr::~mattr()
{

}

void mattr::add_symbol(mem_symbol m)
{
    syms.insert(m);
}

void mattr::add_symbol(const char* n, void *a, size_t s, size_t *d, unsigned int nd)
{
    uint64_t addr = (uint64_t)a;
    syms.insert(mem_symbol(n,addr,s,d,nd));
}

void mattr::add_symbol_vec(std::vector<mem_symbol> &v)
{
    for(size_t i=0; i<v.size(); ++i)
        add_symbol(v[i]);
}

void mattr::remove_symbol(const char* name)
{
	mem_symbol* rm_symbol = syms.find_symbol(name);
	if (!rm_symbol)
	{
		std::cerr << "Unable to find symbol: " << name << std::endl;
		return;
	}
	syms.remove(*rm_symbol);
}
