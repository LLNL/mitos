
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

#include "mpi_hook.h"

#include "../src/Mitos.h"

struct sample_args
{
    ofstream *fout;
    vector<perf_event_sample> *samples;
};

void sample_handler(perf_event_sample *sample, void *args)
{
    struct sample_args *sargs = (struct sample_args*)args;

    int bufsz = 4096;

    sargs->samples->push_back(*sample);

    if(sargs->samples->size() >= bufsz)
    {
        for(size_t i=0; i<sargs->samples->size(); i++)
        {
            cout << "??,"; // variable
            cout << std::hex << sargs->samples->at(i).ip << ",";
            cout << std::hex << sargs->samples->at(i).time << ",";
            cout << std::dec << sargs->samples->at(i).weight << ",";
            cout << std::hex << sargs->samples->at(i).data_src << ",";
            cout << std::hex << sargs->samples->at(i).addr << ",";
            cout << std::dec << sargs->samples->at(i).cpu << std::endl;
/*
            *(sargs->fout) << "??,"; // variable
            *(sargs->fout) << std::hex << sargs->samples->at(i).ip << ",";
            *(sargs->fout) << std::hex << sargs->samples->at(i).time << ",";
            *(sargs->fout) << std::dec << sargs->samples->at(i).weight << ",";
            *(sargs->fout) << std::hex << sargs->samples->at(i).data_src << ",";
            *(sargs->fout) << std::hex << sargs->samples->at(i).addr << ",";
            *(sargs->fout) << std::dec << sargs->samples->at(i).cpu << std::endl;
*/
        }
        sargs->samples->clear();
    }
}

int MPI_Init( int *argc, char ***argv )
{
    int taskid,len;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    int ret = PMPI_Init(argc,argv);

    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
    MPI_Get_processor_name(hostname, &len);

    struct sample_args *args = new struct sample_args;
    args->fout->open(hostname);
    args->samples = new vector<perf_event_sample>();

    *(args->fout) << "variable,ip,time,latency,dataSource,address,cpu" << std::endl;

    Mitos_set_handler(sample_handler,args);

    Mitos_set_sample_mode(SMPL_MEMORY);
    Mitos_set_sample_period(4000);
    Mitos_set_sample_threshold(10);
    Mitos_prepare(0);

    Mitos_begin_sampler();

    return ret;
}

int MPI_Finalize( )
{
    int taskid,len;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
    MPI_Get_processor_name(hostname, &len);

    return PMPI_Finalize();
}

