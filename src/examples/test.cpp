#include <iostream>
#include <fstream>
#include <vector>

#include <Symtab.h>
#include <LineInformation.h>
using namespace Dyninst;
using namespace SymtabAPI;

#include "Mitos.h"

std::vector<perf_event_sample> samples;

char* cmd;

void dump()
{
    // Header
    std::cout << "variable,ip,time,latency,dataSource,hit,address,cpu" << std::endl;

    // Tuples
    for(size_t i=0; i<samples.size(); i++)
    {
        std::cout << "??,"; // variable
        std::cout << std::hex << samples[i].ip << ",";
        std::cout << std::hex << samples[i].time << ",";
        std::cout << std::dec << samples[i].weight << ",";
        std::cout << std::hex << samples[i].dataSourceString() << ",";
        std::cout << std::hex << samples[i].hitOrMiss() << ",";
        std::cout << std::hex << samples[i].addr << ",";
        std::cout << std::dec << samples[i].cpu << std::endl;
    }
}

void sample_handler(perf_event_sample *sample, void *args)
{
   samples.push_back(*sample);
}

void workit()
{
    int N = 512;

    double *a;
    double *b;
    double *c;

    int i,j,k;

    a = (double*)malloc(sizeof(double)*N*N);
    b = (double*)malloc(sizeof(double)*N*N);
    c = (double*)malloc(sizeof(double)*N*N);

    Mitos_add_symbol("a",a,sizeof(double),N*N);
    Mitos_add_symbol("b",b,sizeof(double),N*N);
    Mitos_add_symbol("c",c,sizeof(double),N*N);

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
    cmd = argv[0];

    Mitos_set_sample_mode(SMPL_MEMORY);
    Mitos_set_handler_fn(&sample_handler);

    Mitos_prepare();

    Mitos_begin_sampler();
    workit();
    Mitos_end_sampler();

    dump();
}
