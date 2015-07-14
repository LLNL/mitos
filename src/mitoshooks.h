#include <bits/pthreadtypes.h>

#include "mpi.h"

// pthread hooks
extern "C"
{
    typedef int (*pthread_create_fn_t)(pthread_t*, pthread_attr_t *, void *(*start_routine)(void*), void*);
    typedef void (*pthread_exit_fn_t)(void*);

    int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine) (void*), void *arg);
    void pthread_exit(void *retval);
}

// MPI hooks
int MPI_Init(int *argc, char ***argv);
int MPI_Finalize();
