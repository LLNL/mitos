#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <vector>

#include "SAPI.h"

std::vector<perf_event_sample> samples;

size_t bufsz = 4096;
uint64_t period = 4000;
uint64_t thresh = 7;
char* fout_name = strdup("samples.out");
std::ofstream fout;

void dump_header()
{
    fout << "variable,time,latency,dataSource,address,cpu" << std::endl;
}

void dump_samples()
{
    for(int i=0; i<samples.size(); i++)
    {
        fout << "??,"; // variable
        fout << std::hex << samples[i].time << ","; 
        fout << std::dec << samples[i].weight << ","; 
        fout << std::hex << samples[i].data_src << ","; 
        fout << std::hex << samples[i].addr << ","; 
        fout << std::dec << samples[i].cpu << std::endl;
    }
}

void sample_handler(perf_event_sample *sample, void *args)
{
    samples.push_back(*sample);

    if(samples.size() >= bufsz)
    {
        dump_samples();
        samples.clear();
    }
}

void usage(char *prog)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << prog << " [options] <cmd> [args]" << std::endl;
    std::cerr << "[options]:" << std::endl;
    std::cerr << "    " << "-o filename" << std::endl;
    std::cerr << "    " << "-b sample buffer size" << std::endl;
    std::cerr << "    " << "-p sample period" << std::endl;
    std::cerr << "    " << "-t sample latency threshold" << std::endl;
}

int parse_args(int argc, char **argv)
{
    int c;
    while((c=getopt(argc, argv, "o:b:p:t:")) != -1)
    {
        switch(c)
        {
            case 'o':
                fout_name = optarg;
                break;
            case 'b':
                bufsz = atoi(optarg);
                break;
            case 'p':
                period = atoi(optarg);
                break;
            case 't':
                thresh = atoi(optarg);
                break;
            case '?':
                usage(argv[0]);
                return 1;
            default:
                abort();
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    int cmdarg = -1;
    for(int i=1; i<argc; i++)
    {
        // First non-argument is start of child command
        if(argv[i][0] != '-')
        {
            cmdarg = i;
            break;
        }
    }

    if(cmdarg == -1)
    {
        usage(argv[0]);
        return 1;
    }

    if(parse_args(cmdarg,argv))
        return 1;

    pid_t child = fork();

    if(child == 0) 
    {
        ptrace(PTRACE_TRACEME,0,0,0);
        int err = execvp(argv[cmdarg],&argv[cmdarg]);
        if(err)
        {
            perror("execvp");
        }
    }
    else if(child < 0)
    {
        std::cerr << "Error forking!" << std::endl;
    } 
    else
    {
        int status;
        wait(&status);

        fout.open(fout_name, std::ofstream::out | std::ofstream::trunc);
        if(!fout.is_open())
        {
            std::cerr << "Cannot open output file " << fout_name << std::endl;
            std::cerr << "Aborting..." << std::endl;
            kill(child, SIGKILL);
            return 1;
        }

        dump_header();

        SAPI_set_sample_mode(SMPL_MEMORY);

        SAPI_set_sample_period(period);
        SAPI_set_sample_threshold(thresh);

        SAPI_set_handler(&sample_handler);
        
        SAPI_prepare(child);

        SAPI_begin_sampler();
        {
            ptrace(PTRACE_CONT,child,0,0);
            wait(&status);
        }
        SAPI_end_sampler();

        dump_samples(); // anything left over
    }
}
