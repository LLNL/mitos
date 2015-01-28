#include <stdint.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
using namespace std;

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <Symtab.h>
#include <LineInformation.h>
using namespace Dyninst;
using namespace SymtabAPI;

char* fout_name;
char* fin_name;
char* bin_name;

ifstream fin;
ofstream fout;
Symtab *obj;

int ip_idx;
bool is_mpi = false;
int taskid;
string hostname;

int dump_header()
{
    string linestr;
    istringstream ssIn;
    vector<string> tokens;

    getline(fin,linestr);

    if(linestr.compare(0,3,"MPI") == 0)
    {
        is_mpi = true;
        boost::split(tokens,linestr,boost::is_any_of("\n:"),boost::token_compress_on);

        // MPI Host
        boost::trim(tokens[1]);
        hostname = tokens[1];
        tokens.clear();

        // MPI Comm Rank
        getline(fin,linestr);
        boost::split(tokens,linestr,boost::is_any_of("\n:"),boost::token_compress_on);
        boost::trim(tokens[1]);
        taskid = boost::lexical_cast<int>(tokens[1]);
        tokens.clear();
        
        fout << "mpi_host,mpi_task,";

        getline(fin,linestr);
    }

    // Tuple header
    ssIn.str(linestr);
    boost::split(tokens,linestr,boost::is_any_of(","),boost::token_compress_on);

    fout << "source,line," << linestr << std::endl;

    ip_idx = 0;
    for(int i=0; i<tokens.size(); i++)
    {
        if(tokens[i] == "ip")
            return 0;
        ip_idx++;
    }

    cerr << "\"ip\" entry not found in header" << endl;

    return 1;
}

void dump_samples()
{
    string line;
    string tok;
    uint64_t ip;
    int sym_success;
    while(getline(fin,line))
    {
        istringstream ssline(line);

        for(int i=0; i<=ip_idx; i++)
            getline(ssline,tok,',');
        
        istringstream sstok(tok);
        sstok >> hex >> ip;

        std::vector<Statement*> stats;
        sym_success = obj->getSourceLines(stats,ip);

        if(is_mpi)
        {
            fout << hostname << ",";
            fout << taskid << ",";
        }

        if(sym_success)
        {
            fout << stats[0]->getFile().c_str() << ",";
            fout << stats[0]->getLine() << ",";
        }
        else
        {
            fout << "??,";
            fout << "??,";
        }

        fout << line << std::endl;
    }
}

void usage(char **argv)
{
    cerr << "Usage: " << endl;
    cerr << argv[0] << " [options] <sample_file> <debug_binary>" << endl;
    cerr << "    [options]:" << endl;
    cerr << "        -o filename (default processed_<sample_file>)" << endl;
    cerr << "    <sample_file>: a csv file created using mitosrun" << endl;
    cerr << "    <debug_binary>: the binary executed with mitosrun (must contain debug symbols to be useful)" << endl;
}

void set_defaults()
{
    fout_name = (char*)malloc(strlen("processed_")+strlen(fin_name));
    sprintf(fout_name,"processed_%s",fin_name);
}

int parse_args(int argc, char **argv)
{
    int c;
    while((c=getopt(argc, argv, "o:")) != -1)
    {
        switch(c)
        {
            case 'o':
                fout_name = optarg;
                break;
            case '?':
                usage(argv);
                return 1;
            default:
                abort();
        }
    }

    if(optind+2 > argc)
    {
        usage(argv);
        return 1;
    }

    fin_name = argv[optind];
    bin_name = argv[optind+1];

    set_defaults();

    return 0;
}

int main(int argc, char **argv)
{
    if(argc < 3)
    {
        usage(argv);
        return 1;
    }

    if(parse_args(argc,argv))
        return 1;

    fin.open(fin_name);
    if(!fin.is_open())
    {
        cerr << "Error opening file " << fin_name << endl;
        cerr << "Aborting!" << endl;
        return 1;
    }

    fout.open(fout_name);
    if(!fout.is_open())
    {
        cerr << "Error opening file " << fout_name << endl;
        cerr << "Aborting!" << endl;
        return 1;
    }

    int sym_success = Symtab::openFile(obj,bin_name);
    if(!sym_success)
    {
        cerr << "Symtab cannot open binary " << bin_name << endl;
        cerr << "Aborting!" << endl;
        return 1;
    }

    if(dump_header())
    {
        cerr << "Aborting!" << endl;
        return 1;
    }

    dump_samples();
    
    return 0;
}
