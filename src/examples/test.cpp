#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>

#include <Mitos.h>

#include <omp.h>

void sample_handler(perf_event_sample *sample, void *args)
{
    Mitos_resolve_symbol(sample);

    std::cerr << sample->data_symbol;
    std::cerr << ",";
    std::cerr << std::dec << Mitos_x_index(sample);
    std::cerr << ",";
    std::cerr << std::dec << Mitos_y_index(sample);
    std::cerr << ",";
    std::cerr << std::hex << sample->ip ;
    std::cerr << ",";
    std::cerr << std::hex << sample->time ;
    std::cerr << ",";
    std::cerr << std::dec << sample->weight ;
    std::cerr << ",";
    std::cerr << std::hex << Mitos_data_source(sample);
    std::cerr << ",";
    std::cerr << std::hex << Mitos_hit_type(sample);
    std::cerr << ",";
    std::cerr << std::hex << sample->addr ;
    std::cerr << ",";
    std::cerr << std::dec << sample->cpu << std::endl;
    std::cerr << std::endl;
}

#define ROW_MAJOR(x,y,width) y*width+x

void init_matrices(int N, double **a, double **b, double **c)
{
    int i,j,k;

    *a = new double[N*N];
    *b = new double[N*N];
    *c = new double[N*N];

    size_t dims[2] = {N,N};
    Mitos_add_symbol("a",*a,sizeof(double),dims,2);
    Mitos_add_symbol("b",*b,sizeof(double),dims,2);
    Mitos_add_symbol("c",*c,sizeof(double),dims,2);

    for(i=0; i<N; ++i)
    {
        for(j=0; j<N; ++j)
        {
            (*a)[ROW_MAJOR(i,j,N)] = (double)rand();
            (*b)[ROW_MAJOR(i,j,N)] = (double)rand();
            (*c)[ROW_MAJOR(i,j,N)] = 0;
        }
    }
}

void matmul(int N, double *a, double *b, double *c)
{
#pragma omp parallel for
    for(int i=0; i<N; ++i)
    {
        for(int j=0; j<N; ++j)
        {
            for(int k=0; k<N; ++k)
            {
                c[ROW_MAJOR(i,j,N)] += a[ROW_MAJOR(i,k,N)]*b[ROW_MAJOR(k,j,N)];
            }
        }
    }

    int randx = N*((float)rand() / (float)RAND_MAX+1);
    int randy = N*((float)rand() / (float)RAND_MAX+1);
    std::cout << c[ROW_MAJOR(randx,randy,N)] << std::endl;
}

int main(int argc, char **argv)
{
    // Print header
    std::cout << "variable,xidx,yidx,ip,time,latency,dataSource,hit,address,cpu" << std::endl;

    // Initialize memory
    int N = (argc == 2) ? atoi(argv[1]) : 1024;
    double *a,*b,*c;
    init_matrices(N,&a,&b,&c);

    Mitos_set_sample_mode(SMPL_MEMORY);
    Mitos_set_sample_period(10000);
    Mitos_set_sample_threshold(30);
    Mitos_set_handler_fn(&sample_handler,NULL);

    Mitos_prepare(0);

    Mitos_begin_sampler();
    matmul(N,a,b,c);
    Mitos_end_sampler();
}
