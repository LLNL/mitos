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

    Mitos_begin_sampler();

    return routine_struct->func(routine_struct->args);
}

// pthread hooks
int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine) (void*), void *arg)
{
    //fprintf(stderr, "pthread_create hook\n");
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
    //fprintf(stderr, "pthread_exit hook\n");
    static pthread_exit_fn_t og_pthread_exit = NULL;
    if(!og_pthread_exit)
        og_pthread_exit = (pthread_exit_fn_t)dlsym(RTLD_NEXT, "pthread_exit");

    Mitos_end_sampler();

    og_pthread_exit(retval);
}

#ifdef USE_MPI
// MPI hooks
mitos_output mout;

void sample_handler(perf_event_sample *sample, void *args)
{
    //fprintf(stderr, "MPI handler sample: cpu=%d, tid=%d\n", sample->cpu, sample->tid);
    Mitos_write_sample(sample, &mout);
}

int MPI_Init(int *argc, char ***argv)
{
    //fprintf(stderr, "MPI_Init hook\n");
    int ret = PMPI_Init(argc, argv);

    int mpi_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    char rank_prefix[32];
    sprintf(rank_prefix, "rank_%d", mpi_rank);

    Mitos_create_output(&mout, rank_prefix);
    Mitos_pre_process(&mout);

    Mitos_set_handler_fn(&sample_handler,NULL);
    Mitos_set_sample_latency_threshold(3);
    Mitos_set_sample_time_frequency(4000);
    Mitos_begin_sampler();

    return ret;
}

int MPI_Init_thread(int *argc, char ***argv, int required, int *provided)
{
    //fprintf(stderr, "MPI_Init_thread hook\n");
    int ret = PMPI_Init_thread(argc, argv, required, provided);

    int mpi_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    char rank_prefix[32];
    sprintf(rank_prefix, "rank_%d", mpi_rank);

    Mitos_create_output(&mout, rank_prefix);
    Mitos_pre_process(&mout);

    Mitos_set_handler_fn(&sample_handler,NULL);
    Mitos_set_sample_latency_threshold(3);
    Mitos_set_sample_time_frequency(4000);
    Mitos_begin_sampler();

    return ret;
}

int MPI_Finalize()
{
    //fprintf(stderr, "MPI_Finalize hook\n");
    Mitos_end_sampler();
    Mitos_post_process("/proc/self/exe", &mout);

    return PMPI_Finalize();
}
#endif // USE_MPI
