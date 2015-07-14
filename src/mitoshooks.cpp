#include "mitoshooks.h"

#include <stdio.h>
#include <dlfcn.h>

// pthread hooks
int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine) (void*), void *arg)
{
    static pthread_create_fn_t og_pthread_create = NULL;
    if(!og_pthread_create)
        og_pthread_create = (pthread_create_fn_t)dlsym(RTLD_NEXT, "pthread_create");

    fprintf(stderr, "pthread_create!\n");

    return og_pthread_create(thread, attr, start_routine, arg);
}

void pthread_exit(void *retval)
{
    static pthread_exit_fn_t og_pthread_exit = NULL;
    if(!og_pthread_exit)
        og_pthread_exit = (pthread_exit_fn_t)dlsym(RTLD_NEXT, "pthread_exit");

    fprintf(stderr, "pthread_exit!\n");

    og_pthread_exit(retval);
}

// MPI hooks
int MPI_Init(int *argc, char ***argv)
{
    fprintf(stderr, "MPI_Init!\n");
    return PMPI_Init(argc, argv);
}

int MPI_Finalize()
{
    fprintf(stderr, "MPI_Finalize!\n");
    return PMPI_Finalize();
}
