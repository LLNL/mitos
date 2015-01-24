#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>

#include "Mitos.h"

size_t bufsz;
uint64_t period;
uint64_t thresh;
char* fout_name;

#define DEFAULT_BUFSZ       4096
#define DEFAULT_THRESH      10
#define DEFAULT_PERIOD      4000
#define DEFAULT_FOUT_NAME   "samples.out"

std::ofstream fout;
std::vector<perf_event_sample> samples;

void dump_header()
{
    fout << "variable,ip,time,latency,dataSource,address,cpu" << std::endl;
}

void dump_samples()
{
    for(size_t i=0; i<samples.size(); i++)
    {
        fout << "??,"; // variable
        fout << std::hex << samples[i].ip << ",";
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

void usage(char **argv)
{
    std::cerr << "Usage:" << std::endl;
    std::cerr << argv[0] << " [options] <cmd> [args]" << std::endl;
    std::cerr << "    [options]:" << std::endl;
    std::cerr << "        -o filename (default samples.out)" << std::endl;
    std::cerr << "        -b sample buffer size (default 4096)" << std::endl;
    std::cerr << "        -p sample period (default 4000)" << std::endl;
    std::cerr << "        -t sample latency threshold (default 10)" << std::endl;
    std::cerr << "    <cmd>: command to sample on (required)" << std::endl;
    std::cerr << "    [args]: command arguments" << std::endl;
}

void set_defaults()
{
    bufsz = DEFAULT_BUFSZ;
    period = DEFAULT_PERIOD;
    thresh = DEFAULT_THRESH;
    fout_name = strdup(DEFAULT_FOUT_NAME);
}

int parse_args(int argc, char **argv)
{
    set_defaults();

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
                usage(argv);
                return 1;
            default:
                abort();
        }
    }

    return 0;
}

int findCmdArgId(int argc, char **argv)
{
    // case 1: argv[0] -f1000 cmd
    // case 2: argv[0] -f 1000 cmd
    int cmdarg = -1;
    bool isarg = false;
    for(int i=1; i<argc; i++)
    {
        if(argv[i][0] != '-')
        {
            if(isarg)
                isarg = false;
            else
                return i;
        }
        else
        {
            if(strlen(argv[i]) > 2)
                isarg = false;
            else
                isarg = true;
        }
    }
    return cmdarg;
}

int main(int argc, char **argv)
{
    int cmdarg = findCmdArgId(argc,argv);

    if(cmdarg == -1)
    {
        usage(argv);
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

        Mitos_set_sample_mode(SMPL_MEMORY);

        Mitos_set_sample_period(period);
        Mitos_set_sample_threshold(thresh);

        Mitos_set_handler(&sample_handler);

        Mitos_prepare(child);

        Mitos_begin_sampler();
        {
            ptrace(PTRACE_CONT,child,0,0);
            wait(&status);
        }
        Mitos_end_sampler();

        dump_samples(); // anything left over
    }
}
