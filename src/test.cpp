#include <iostream>
#include <fstream>
#include <vector>

#include <Symtab.h>
#include <LineInformation.h>
using namespace Dyninst;
using namespace SymtabAPI;

#include "PSAPI.h"

std::vector<perf_event_sample> samples;
std::vector<Statement*> line_info;

void postprocess()
{
    Symtab *obj;

    // Open current binary
    int success = Symtab::openFile(obj,"test");
    if(!success)
        std::cerr << "cant open symtab" << std::endl;
    
    // Populate line info
    line_info.resize(samples.size(),NULL);

    for(int i=0; i<samples.size(); i++)
    {
       std::vector<Statement*> stats;
       success = obj->getSourceLines(stats,samples[i].ip);
       
       if(success)
       {
           std::cout << "sucksess" << std::endl;
           line_info[i] = stats[0];
       }
    }
}

void dump()
{
    // Header
    std::cout << "variable,source,line,time,latency,dataSource,address,cpu" << std::endl;

    // Tuples
    for(int i=0; i<samples.size(); i++)
    {
        std::cout << "??,"; // variable
        std::cout << (line_info[i] ? line_info[i]->getFile().c_str() : "??") << ","; // source
        std::cout << std::dec << (line_info[i] ? line_info[i]->getLine() : -1) << ","; // line
        std::cout << std::hex << samples[i].time << ","; 
        std::cout << std::dec << samples[i].weight << ","; 
        std::cout << std::hex << samples[i].data_src << ","; 
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

    PSAPI_add_symbol("a",a,sizeof(double),N*N);
    PSAPI_add_symbol("b",b,sizeof(double),N*N);
    PSAPI_add_symbol("c",c,sizeof(double),N*N);

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
    PSAPI_set_sample_mode(SMPL_MEMORY);
    PSAPI_set_handler(&sample_handler);

    PSAPI_prepare();

    PSAPI_begin_sampler();
    workit();
    PSAPI_end_sampler();

    std::cout << "post" << std::endl;
    postprocess();
    std::cout << "dump" << std::endl;
    dump();
}
