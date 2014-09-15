#include <iostream>
#include <fstream>
#include <map>

#include "perfsmpl.h"

std::map<struct LineTuple,int> profile;

bool operator<(const LineTuple &l,const LineTuple &r) {
    return 0;
}

void prof_handler(perf_event_sample *sample, void *args)
{
    for(int c=0; c<sample->callchainLineInfo.size(); c++)
    {
        for(int i=0; i<sample->callchainLineInfo[c].size(); i++)
        {
            struct LineTuple lt = sample->callchainLineInfo[c][i];

            if(profile.count(lt))
                profile[lt] = profile[lt]+1;
            else
                profile[lt] = 0;
        }
    }
}

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

    mprof.set_period(1);
    mprof.set_handler(&prof_handler);
    mprof.prepare();

    mprof.begin_prof();
    do_werk();
    mprof.end_prof();

    mprof.readout();

    std::map<struct LineTuple,int>::iterator it;
    for(it = profile.begin(); it != profile.end(); it++)
    {
        std::cout << it->first.srcFile << " : " << it->first.srcLine << " : " << it->second << std::endl;
    }
}
