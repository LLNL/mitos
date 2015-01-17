#include "mattr.h"

mem_symbol::mem_symbol()
{

}

mem_symbol::mem_symbol(std::string n, uint64_t a, size_t s, size_t l)
    :
    name(n),
    addr(a),
    sz(s),
    len(l)
{

}

mem_symbol::~mem_symbol()
{

}

bool mem_symbol::contains(uint64_t addr)
{
    return this->addr <= addr &&  addr < this->addr+sz*len;
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

void mattr::add_symbol(std::string n, void *a, size_t s, size_t l)
{
    uint64_t addr = (uint64_t)a;
    syms.insert(mem_symbol(n,addr,s,l));
}

void mattr::add_symbol_vec(std::vector<mem_symbol> &v)
{
    for(int i=0; i<v.size(); ++i)
        add_symbol(v[i]);
}
