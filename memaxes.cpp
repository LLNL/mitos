#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <iostream>
#include <fstream>
#include <vector>

#include <Symtab.h>
#include <LineInformation.h>
using namespace Dyninst;
using namespace SymtabAPI;

#include "SAMP.h"

//char **cmd;
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
           line_info[i] = stats[0];
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

int main(int argc, char **argv)
{
    // Fork 
    pid_t child = fork();

    if(child == 0) 
    {
        ptrace(PTRACE_TRACEME,0,0,0);
	//execl("/bin/dd", "/bin/dd", "if=/dev/urandom", "of=/dev/null", "count=10000", (char *) 0);
        int err = execv(argv[1],&argv[1]);
        if(err)
        {
            perror("execv");
            std::cerr << "Note: \'" << argv[1] << "\' must be an absolute path!" << std::endl;
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

        SAMP_set_sample_mode(SMPL_MEMORY);
        SAMP_set_handler(&sample_handler);
        
        SAMP_prepare(child);

        SAMP_begin_sampler();
        {
            ptrace(PTRACE_CONT,child,0,0);
            wait(&status);
        }
        SAMP_end_sampler();

        postprocess();
        dump();
    }

}
