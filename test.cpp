#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "SAMP.h"

perfsmpl sampler;
dbgmem memsyms;

int N = 512;

void sample_handler(perf_event_sample *sample, void *args)
{
    mem_symbol *sym = memsyms.find_symbol(sample->addr);

    if(sym != NULL)
    {
        size_t index = sym->get_index(sample->addr);
        std::cout << "SYM: " << sym->get_name() << '\t';
        std::cout << "INDEX: " << index << '\t';
        std::cout << "X: " << index % N << '\t';
        std::cout << "Y: " << index / N << '\t';
        std::cout << std::endl;
    }
}

void workit()
{
    int i,j,k;

    double *a = (double*)malloc(sizeof(double)*N*N);
    double *b = (double*)malloc(sizeof(double)*N*N);
    double *c = (double*)malloc(sizeof(double)*N*N);

    memsyms.add_symbol("a",a,sizeof(double),N*N);
    memsyms.add_symbol("b",b,sizeof(double),N*N);
    memsyms.add_symbol("c",c,sizeof(double),N*N);

    for(i=0; i<N; ++i)
        for(j=0; j<N; ++j)
            c[i*N+j] = 0;

    for(i=0; i<N; ++i)
        for(j=0; j<N; ++j)
            for(k=0; k<N; ++k)
                c[i*N+j] += a[i*N+k] * b[k*N+j];

}

int main(int argc, char **argv)
{
    sampler.set_sample_mode(SMPL_MEMORY);
    sampler.set_handler(&sample_handler);

    sampler.prepare();

    sampler.set_handler(&sample_handler);
    sampler.begin_sampler();
    workit();
    sampler.end_sampler();
}
