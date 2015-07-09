#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>

#include <LineInformation.h> // symtabAPI
#include <CodeObject.h> // parseAPI
#include <InstructionDecoder.h> // instructionAPI
using namespace Dyninst;
using namespace SymtabAPI;
using namespace ParseAPI;
using namespace InstructionAPI;

#include "hwloc_dump.h"
#include "x86_util.h"

#include "Mitos.h"

int Mitos_create_output(mitos_output *mout)
{
    // Set top directory name
    std::stringstream ss_dname_topdir;
    ss_dname_topdir << "mitos_" << std::time(NULL);
    mout->dname_topdir = strdup(ss_dname_topdir.str().c_str());

    // Set data directory name
    std::stringstream ss_dname_datadir;
    ss_dname_datadir << ss_dname_topdir.str() << "/data";
    mout->dname_datadir = strdup(ss_dname_datadir.str().c_str());

    // Set src directory name
    std::stringstream ss_dname_srcdir;
    ss_dname_srcdir << ss_dname_topdir.str() << "/src";
    mout->dname_srcdir = strdup(ss_dname_srcdir.str().c_str());

    // Create the directories
    int err;
    err = mkdir(mout->dname_topdir,0777);
    err |= mkdir(mout->dname_datadir,0777);
    err |= mkdir(mout->dname_srcdir,0777);

    if(err)
    {
        std::cerr << "Mitos: Failed to create output directories!\n";
        return 1;
    }

    // Create file for raw sample output
    mout->fname_raw = strdup(std::string(std::string(mout->dname_datadir) + "/raw_samples.csv").c_str());
    mout->fout_raw = fopen(mout->fname_raw,"w");
    if(!mout->fout_raw)
    {
        std::cerr << "Mitos: Failed to create raw output file!\n";
        return 1;
    }

    // Create file for processed sample output
    mout->fname_processed = strdup(std::string(std::string(mout->dname_datadir) + "/samples.csv").c_str());
    mout->fout_processed = fopen(mout->fname_processed,"w");
    if(!mout->fout_processed)
    {
        std::cerr << "Mitos: Failed to create processed output file!\n";
        return 1;
    }

    mout->ok = true;

    return 0;
}

int Mitos_pre_process(mitos_output *mout)
{
    // Create hardware topology file for current hardware
    std::string fname_hardware = std::string(mout->dname_topdir) + "/hardware.xml";
    int err = dump_hardware_xml(fname_hardware.c_str());
    if(err)
    {
        std::cerr << "Mitos: Failed to create hardware topology file!\n";
        return 1;
    }

    // hwloc puts the file in the current directory, need to move it
    err = rename("hardware.xml", fname_hardware.c_str());
    if(err)
    {
        std::cerr << "Mitos: Failed to move hardware topology file to output directory!\n";
        return 1;
    }

    return 0;
}

int Mitos_write_sample(perf_event_sample *sample, mitos_output *mout)
{
    if(!mout->ok)
        return 1;

    Mitos_resolve_symbol(sample);

    fprintf(mout->fout_raw,
            "%llu,%s,%llu,%llu,%llu,%llu,%llu,%u,%u,%llu,%llu,%u,%llu,%llu\n",
            sample->ip,
            sample->data_symbol,
            sample->data_size,
            sample->num_dims,
            sample->access_index[0],
            sample->access_index[1],
            sample->access_index[2],
            sample->pid,
            sample->tid,
            sample->time,
            sample->addr,
            sample->cpu,
            sample->weight,
            sample->data_src);
    
    return 0;
}

int Mitos_post_process(char *bin_name, mitos_output *mout)
{
    // Open Symtab object and code source object
    Symtab *symtab_obj;
    SymtabCodeSource *symtab_code_src;

    int sym_success = Symtab::openFile(symtab_obj,bin_name);
    if(!sym_success)
    {
        std::cerr << "Mitos: Failed to open Symtab object for " << bin_name << std::endl;
        return 1;
    }

    symtab_code_src = new SymtabCodeSource(bin_name);

    // Get machine information
    unsigned int inst_length = InstructionDecoder::maxInstructionLength;
    Architecture arch = symtab_obj->getArchitecture();

    // Open input/output files
    std::ifstream fraw(mout->fname_raw);
    std::ofstream fproc(mout->fname_processed);

    // Write header for processed samples
    fproc << "source,line,instruction,bytes,ip,variable,buffer_size,dims,xidx,yidx,zidx,pid,tid,time,addr,cpu,latency,data_src\n";

    // Read raw samples one by one and get attribute from ip
    uint64_t ip;
    size_t ip_endpos;
    std::string line, ip_str;
    while(std::getline(fraw, line).good())
    {
        // Unknown values
        std::stringstream source;
        std::stringstream line_num;
        std::stringstream instruction;
        std::stringstream bytes;

        // Extract ip
        size_t ip_endpos = line.find(',');
        std::string ip_str = line.substr(0,ip_endpos);
        ip = (uint64_t)strtoull(ip_str.c_str(),NULL,0);

        // Parse ip for source line info
        std::vector<Statement*> stats;
        sym_success = symtab_obj->getSourceLines(stats,ip);
        if(sym_success)
        {
            source << stats[0]->getFile();
            line_num << stats[0]->getLine();
        }

        // Parse ip for instruction info
        void *inst_raw = NULL;
        if(symtab_code_src->isValidAddress(ip))
        {
            inst_raw = symtab_code_src->getPtrToInstruction(ip);

            if(inst_raw)
            {
                // Get instruction
                InstructionDecoder dec(inst_raw,inst_length,arch);
                Instruction::Ptr inst = dec.decode();
                Operation op = inst->getOperation();
                entryID eid = op.getID();
                
                instruction << NS_x86::entryNames_IAPI[eid];

                // Get bytes read
                if(inst->readsMemory())
                    bytes << getReadSize(inst);
            }
        }

        // Write out the sample
        fproc << (source.str().empty()      ? "??" : source.str()       ) << ","
              << (line_num.str().empty()    ? "??" : line_num.str()     ) << ","
              << (instruction.str().empty() ? "??" : instruction.str()  ) << ","
              << (bytes.str().empty()       ? "??" : bytes.str()        ) << ","
              << line << std::endl;
    }

    fproc.close();
    fraw.close();

    int err = remove(mout->fname_raw);
    if(err)
    {
        std::cerr << "Mitos: Failed to delete raw sample file!\n";
        return 1;
    }

    return 0;
}

