#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <sstream>
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

void setMitosMPIRunEnv()
{
    std::ostringstream ssbufsz;
    std::ostringstream ssperiod;
    std::ostringstream ssthreshold;

    ssbufsz << bufsz;
    ssperiod << period;
    ssthreshold << thresh;

    setenv("LD_PRELOAD","./libmitosmpihook.so",1);
    setenv("MITOS_SAMPLE_PERIOD",ssperiod.str().c_str(),1);
    setenv("MITOS_SAMPLE_THRESHOLD",ssthreshold.str().c_str(),1);
    setenv("MITOS_SAMPLE_BUFFERSIZE",ssbufsz.str().c_str(),1);
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

    setMitosMPIRunEnv();

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
        return 1;
    }
    else
    {
        int status;
        wait(&status);

        {
            ptrace(PTRACE_CONT,child,0,0);
            wait(&status);
        }
    }
}
