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

#include <iostream>
#include <iomanip>
#include <vector>

#include "Mitos.h"

class procsmpl;
class threadsmpl;

// Process-wide sampler
class procsmpl
{
    friend class threadsmpl;
    friend class perf_event_sample;

public:
    procsmpl();
    ~procsmpl();

    int prepare(pid_t PID);

    void set_sample_period(uint64_t p) 
        { sample_period = p; }
    void set_sample_threshold(uint64_t t) 
        { sample_threshold = t; }

    void set_handler_fn(sample_handler_fn_t h, void* args) 
        { handler_fn = h; handler_fn_args = args; }

    inline bool has_attribute(uint32_t a) 
        { return this->attr.sample_type & a; }

private:
    // set up perf_event_attr
    void init_attr();
    int init_proc_sighandler();

private:
    // perf event configuration
    struct perf_event_attr attr;

    pid_t sample_pid;
    uint64_t sample_period;
    uint64_t sample_threshold;
    uint64_t sample_type;

    size_t mmap_pages;
    size_t mmap_size;
    size_t pgsz;
    size_t pgmsk;

    // user-defined handler
    sample_handler_fn_t handler_fn;
    void *handler_fn_args;

    // per-thread sampler instances
    std::vector<threadsmpl> thread_samplers;
};

// Thread-local Sampler
class threadsmpl
{
    friend class procsmpl;
    friend void thread_sighandler(int sig, siginfo_t *info, void *extra);

public:
    threadsmpl(procsmpl *parent);
    ~threadsmpl();

    int begin_sampling();
    void end_sampling();

    int init();

private:
    int init_perf_event(struct perf_event_attr *attr, size_t mmap_size);
    int init_thread_sighandler();

private:
    procsmpl *proc_parent;

    int ready;
    int fd;
    struct perf_event_mmap_page *mmap_buf;
    perf_event_sample pes;
    uint64_t counter_value;
};

#endif
