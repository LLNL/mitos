#include <bits/pthreadtypes.h>


// pthread hooks
extern "C"
{
    typedef int (*pthread_create_fn_t)(pthread_t*, pthread_attr_t *, void *(*start_routine)(void*), void*);
    typedef void (*pthread_exit_fn_t)(void*);

    int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine) (void*), void *arg);
    void pthread_exit(void *retval);
}

#ifdef USE_MPI

#include "mpi.h"

// MPI hooks
int MPI_Init(int *argc, char ***argv);
int MPI_Init_thread(int *argc, char ***argv, int required, int *provided);
int MPI_Finalize();

#endif // USE_MPI
