#include "perfsmpl.h"

void do_werk()
{
    double v = 241209092.548394035;
    for(int i=0; i<512; i++)
        for(int j=0; j<512; j++)
            for(int k=0; k<512; k++)
                v = v*v;
    printf("v : %f\n",v);
}

int main(int argc, char **argv)
{
    perf_event_prof mprof;
    mprof.set_outputstream(&std::cout);
    mprof.begin_prof();
    do_werk();
    mprof.end_prof();
    mprof.readout();
}
