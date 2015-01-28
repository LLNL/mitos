#include <iostream>

void workit()
{
    int N = 512;

    double *a;
    double *b;
    double *c;

    int i,j,k;

    a = new double[N*N];
    b = new double[N*N];
    c = new double[N*N];

    for(i=0; i<N; ++i)
        for(j=0; j<N; ++j)
            c[i*N+j] = 0;

    for(i=0; i<N; ++i)
        for(j=0; j<N; ++j)
            for(k=0; k<N; ++k)
                c[i*N+j] += a[i*N+k] * b[k*N+j];

}

int main(int argc, char **argv)
{
    workit();
}
