#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include "perfsmpl.h"

void profile_handler(perf_event_sample *sample, void *args)
{
    // TODO: try getting regs
    std::cout << "\tIP:" << sample->ip;
    std::cout << "\tADDR:" << sample->addr;
    std::cout << "\tTIME:" << sample->addr;
    std::cout << "\tWEIGHT:" << sample->weight;
    std::cout << "\tDATA_SRC:" << sample->data_src;
    std::cout << std::endl;
    std::cout << "\tCALLCHAIN:" << std::endl;
    for(int i=0; i<sample->nr; i++)
    {
        std::cout << sample->ips[i] << "  ->  ";
    }
    std::cout << std::endl;
}

void do_werk()
{
    double v = 241209092.548394035;
    for(int i=0; i<512; i++)
        for(int j=0; j<1024; j++)
            for(int k=0; k<1024; k++)
                v = v*v;
    std::cout << v << std::endl;
}

int main(int argc, char **argv)
{
    perf_event_prof mprof;

    mprof.set_sample_mode(SMPL_MEMORY);
    mprof.set_handler(&profile_handler);

    mprof.prepare();

    mprof.set_handler(&profile_handler);
    mprof.begin_prof();
    do_werk();
    mprof.end_prof();
}
