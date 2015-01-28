#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

#include "mpi_hook.h"

#include "Mitos.h"

struct sample_args
{
    int bufsz;
    ofstream fout;
    vector<perf_event_sample> samples;
};

void dump_samples(ofstream *fout, vector<perf_event_sample> *samples)
{
    for(size_t i=0; i<samples->size(); i++)
    {
        *fout << "??,"; // TODO: variable here
        *fout << std::hex << samples->at(i).ip       << ",";
        *fout << std::hex << samples->at(i).time     << ",";
        *fout << std::dec << samples->at(i).weight   << ",";
        *fout << std::hex << samples->at(i).data_src << ",";
        *fout << std::hex << samples->at(i).addr     << ",";
        *fout << std::dec << samples->at(i).cpu      << std::endl;
    }
}

void sample_handler(perf_event_sample *sample, void *args)
{
    struct sample_args *sargs = (struct sample_args*)args;

    sargs->samples.push_back(*sample);

    if(sargs->samples.size() >= sargs->bufsz)
    {
        dump_samples(&sargs->fout,&sargs->samples);
        sargs->samples.clear();
    }
}

void end_dump(void *args)
{
    struct sample_args *sargs = (struct sample_args*)args;
    dump_samples(&sargs->fout,&sargs->samples);
    sargs->fout.close();
}

int MPI_Init( int *argc, char ***argv )
{

    struct sample_args *args = new struct sample_args;
    args->bufsz = 1024;

    /* Prepare Mitos */
    Mitos_set_handler_fn(sample_handler,args);
    Mitos_set_end_fn(end_dump,args);

    Mitos_set_sample_mode(SMPL_MEMORY);
    Mitos_set_sample_period(500);
    Mitos_set_sample_threshold(3);
    Mitos_prepare(0);

    /* PMPI Call */
    int ret = PMPI_Init(argc,argv);

    /* Write Header */
    int len,taskid;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
    MPI_Get_processor_name(hostname, &len);

    args->fout.open(hostname);
    args->fout << "variable,ip,time,latency,dataSource,address,cpu" << std::endl;

    /* Begin Mitos */
    Mitos_begin_sampler();

    return ret;
}

int MPI_Finalize( )
{
    Mitos_end_sampler();
    return PMPI_Finalize();
}

