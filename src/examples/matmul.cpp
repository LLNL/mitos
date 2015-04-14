#include <cstdlib>
#include <iostream>

#include <omp.h>

#define ROW_MAJOR(x,y,width) y*width+x

void workit()
{
    int N = 2048;

    double *a;
    double *b;
    double *c;

    int i,j,k;

    a = new double[N*N];
    b = new double[N*N];
    c = new double[N*N];

    for(i=0; i<N; ++i)
    {
        for(j=0; j<N; ++j)
        {
            a[ROW_MAJOR(i,j,N)] = (float)rand();
            b[ROW_MAJOR(i,j,N)] = (float)rand();
            c[ROW_MAJOR(i,j,N)] = 0;
        }
    }

#pragma omp parallel for 
    for(i=0; i<N; ++i)
    {
        for(j=0; j<N; ++j)
        {
            for(k=0; k<N; ++k)
            {
                c[ROW_MAJOR(i,j,N)] += a[ROW_MAJOR(i,k,N)]*b[ROW_MAJOR(k,j,N)];
            }
        }
    }

    int randx = N*((float)rand() / (float)RAND_MAX);
    int randy = N*((float)rand() / (float)RAND_MAX);
    std::cout << c[randx*N+randy] << std::endl;

}

int main(int argc, char **argv)
{
    workit();
}
