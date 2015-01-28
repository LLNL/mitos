#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

#include "mitosmpihook.h"

#include "Mitos.h"

ofstream fout;
vector<perf_event_sample> samples;

void dump_samples()
{
    for(size_t i=0; i<samples.size(); i++)
    {
        fout << "??,"; // TODO: variable here
        fout << std::hex << samples.at(i).ip       << ",";
        fout << std::hex << samples.at(i).time     << ",";
        fout << std::dec << samples.at(i).weight   << ",";
        fout << std::hex << samples.at(i).data_src << ",";
        fout << std::hex << samples.at(i).addr     << ",";
        fout << std::dec << samples.at(i).cpu      << std::endl;
    }
}

void sample_handler(perf_event_sample *sample, void *args)
{
    samples.push_back(*sample);

    if(samples.size() >= samples.capacity()-1)
    {
        dump_samples();
        samples.clear();
    }
}

void end_dump(void *args)
{
    dump_samples();
    fout.close();
}

int MPI_Init( int *argc, char ***argv )
{
    /* Set Mitos variables */
    uint64_t period = 500;
    uint64_t threshold = 3;
    uint64_t bufsz = 1024;

    char *envPeriod = getenv("MITOS_SAMPLE_PERIOD");
    char *envThreshold = getenv("MITOS_SAMPLE_THRESHOLD");
    char *envBufsz = getenv("MITOS_SAMPLE_BUFFERSIZE");

    if(envPeriod)
        period = atol(envPeriod);
    if(envThreshold)
        threshold = atol(envThreshold);
    if(envBufsz)
        bufsz = atol(envBufsz);

    samples.reserve(bufsz);

    /* Prepare Mitos */
    Mitos_set_handler_fn(sample_handler,NULL);
    Mitos_set_sample_mode(SMPL_MEMORY);
    Mitos_set_sample_period(period);
    Mitos_set_sample_threshold(threshold);
    Mitos_prepare(0); // 0 for current pid

    /* PMPI Call */
    int ret = PMPI_Init(argc,argv);

    /* Write Header */
    int len,taskid;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
    MPI_Get_processor_name(hostname, &len);

    fout.open(hostname);
    fout << "MPI Host : " << hostname << std::endl;
    fout << "MPI Comm Rank : " << taskid << std::endl;
    fout << "variable,ip,time,latency,dataSource,address,cpu" << std::endl;

    /* Begin Mitos */
    Mitos_begin_sampler();

    return ret;
}

int MPI_Finalize( )
{
    Mitos_end_sampler();
    dump_samples();
    return PMPI_Finalize();
}

