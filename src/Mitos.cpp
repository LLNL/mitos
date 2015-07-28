#include "Mitos.h"

#include "procsmpl.h"
#include "mattr.h"

static procsmpl m_procsmpl;
static mattr m_mattr;

void Mitos_set_sample_period(uint64_t p) 
{ 
    m_procsmpl.set_sample_period(p); 
}

void Mitos_set_sample_threshold(uint64_t t) 
{ 
    m_procsmpl.set_sample_threshold(t); 
}

void Mitos_set_handler_fn(sample_handler_fn_t h, void* args) 
{
    m_procsmpl.set_handler_fn(h,args); 
}

void Mitos_begin_sampler() 
{
    m_procsmpl.begin_sampling(); 
}

void Mitos_end_sampler() 
{
    m_procsmpl.end_sampling(); 
}

void Mitos_add_symbol(const char* n, void *a, size_t s, size_t *dims, unsigned int ndims)
{
    m_mattr.add_symbol(n,a,s,dims,ndims); 
}

void Mitos_resolve_symbol(struct perf_event_sample *s)
{
    mem_symbol *m = m_mattr.find_symbol(s->addr);
    if(m)
    {
        s->data_size = m->get_sz();
        s->num_dims = m->get_num_dims();
        s->data_symbol = m->get_name();

        m->get_index(s->addr, s->access_index);
    }
    else
    {
        s->data_size = 0;
        s->num_dims = 1;
        s->access_index[0] = 0;
        s->access_index[1] = 0;
        s->access_index[2] = 0;
        s->data_symbol = "??";
    }
}

long Mitos_x_index(struct perf_event_sample *s)
{
    if(s->access_index)
    {
        if(s->num_dims > 0)
            return s->access_index[0];
        else
            return 0;
    }
    return -1;
}

long Mitos_y_index(struct perf_event_sample *s)
{
    if(s->access_index)
    {
        if(s->num_dims > 1)
            return s->access_index[1];
        else
            return 0;
    }
    return -1;
}

long Mitos_z_index(struct perf_event_sample *s)
{
    if(s->access_index)
    {
        if(s->num_dims > 2)
            return s->access_index[2];
        else
            return 0;
    }
    return -1;
}

