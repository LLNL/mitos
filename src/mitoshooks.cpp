#include "mitoshooks.h"

#include "Mitos.h"

#include <stdio.h>
#include <dlfcn.h>

#include <unistd.h>
#include <sys/syscall.h>

struct func_args
{
    void *(*func)(void*);
    void *args;
};

void* routine_wrapper(void *args)
{
    func_args *routine_struct = (func_args*)args;

    fprintf(stderr, "Beginning Mitos sampler on thread %d\n", syscall(__NR_gettid));

    Mitos_begin_sampler();

    return routine_struct->func(routine_struct->args);
}

// pthread hooks
int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine) (void*), void *arg)
{
    static pthread_create_fn_t og_pthread_create = NULL;
    if(!og_pthread_create)
        og_pthread_create = (pthread_create_fn_t)dlsym(RTLD_NEXT, "pthread_create");

    struct func_args *f = (struct func_args*)malloc(sizeof(struct func_args));
    f->func = start_routine;
    f->args = arg;

    return og_pthread_create(thread, attr, routine_wrapper, f);
}

void pthread_exit(void *retval)
{
    static pthread_exit_fn_t og_pthread_exit = NULL;
    if(!og_pthread_exit)
        og_pthread_exit = (pthread_exit_fn_t)dlsym(RTLD_NEXT, "pthread_exit");

    Mitos_end_sampler();

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
