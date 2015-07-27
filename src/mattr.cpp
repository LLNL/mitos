#include "mattr.h"

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

void mem_symbol_splay_node::insert(mem_symbol m)
{
    // Find candidate location for m
   if 
    
    // If candidate location is empty, put it there
    
    // Else recurse into candidate location
    mem_symbol_splay_node *node = new mem_symbol_splay_node();
    node->set_symbol(m);
}

mem_symbol_splay_tree::mem_symbol_splay_tree()
{
    root = NULL;
}

mem_symbol_splay_tree::~mem_symbol_splay_tree()
{
    delete root;
}

mem_symbol_splay_node* mem_symbol_splay_tree::insert(mem_symbol m)
{
    if(root == NULL)
    {
        root = new mem_symbol_splay_node();
        root->set_symbol(new mem_symbol(m));
    }

    mem_symbol_splay_node *new_node
}

void mem_symbol_splay_tree::remove(mem_symbol m)
{
    return root->remove(addr);
}

mem_symbol_splay_node* mem_symbol_splay_tree::find(uint64_t addr)
{
    return root->find(addr);
}

mattr::mattr()
{

}

mattr::~mattr()
{

}

void mattr::add_symbol(mem_symbol m)
{
    tree.insert(m);
}

void mattr::add_symbol(const char* n, void *a, size_t s, size_t *d, unsigned int nd)
{
    uint64_t addr = (uint64_t)a;
    tree.insert(mem_symbol(n,addr,s,d,nd));
}

mem_symbol* find_symbol(uint64_t addr)
{
    mem_symbol_splay_node *node = tree.find_mem_symbol(addr);
    return &node->get_symbol();
}

