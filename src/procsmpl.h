#ifndef PROCSMPL_H
#define PROCSMPL_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <pthread.h>
#include <signal.h>

#include "Mitos.h"

class procsmpl;
class threadsmpl;

struct perf_event_container
{
    int fd;
    struct perf_event_attr attr;
    struct perf_event_mmap_page *mmap_buf;
};

// Process-wide sampler
class procsmpl
{
    friend class threadsmpl;
    friend class perf_event_sample;
    friend void thread_sighandler(int sig, siginfo_t *info, void *extra);

public:
    procsmpl();
    ~procsmpl();

    int begin_sampling();
    void end_sampling();

    void set_pid(pid_t p) 
        { target_pid = p; }
    void set_sample_time_frequency(uint64_t p) 
        { use_frequency = 1; sample_frequency = p; }
    void set_sample_event_period(uint64_t p) 
        { use_frequency = 0; sample_period = p; }
    void set_sample_latency_threshold(uint64_t t) 
        { sample_latency_threshold = t; }

    void set_handler_fn(sample_handler_fn_t h, void* args) 
        { handler_fn = h; handler_fn_args = args; }

private:
    // set up perf_event_attr
    void init_attrs();

private:
    // perf event configuration
    int num_attrs;
    struct perf_event_attr *attrs;

    pid_t target_pid;

    uint64_t use_frequency;

    uint64_t sample_period;
    uint64_t sample_frequency;
    uint64_t sample_latency_threshold;
    uint64_t sample_type;

    size_t mmap_pages;
    size_t mmap_size;
    size_t pgsz;
    size_t pgmsk;

    // user-defined handler
    sample_handler_fn_t handler_fn;
    void *handler_fn_args;

    // misc
    bool first_time;
};

// Thread-local Sampler
class threadsmpl
{
    friend class procsmpl;
    friend void thread_sighandler(int sig, siginfo_t *info, void *extra);

public:
    int begin_sampling();
    void end_sampling();

    int init(procsmpl *parent);

//private:
    int init_perf_events(struct perf_event_attr *attrs, int num_attrs, size_t mmap_size);
    int init_thread_sighandler();

//private:
    procsmpl *proc_parent;

    int ready;

    int num_events;
    struct perf_event_container *events;

    perf_event_sample pes;
};

#endif
