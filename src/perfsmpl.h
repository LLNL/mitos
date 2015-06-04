#ifndef PERFSMPL_H
#define PERFSMPL_H

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

class perfsmpl
{
    friend class perf_event_sample;
    friend void signal_thread_handler(int sig, siginfo_t *info, void *extra);

public:
    perfsmpl();
    ~perfsmpl();

    int prepare(pid_t PID);

    int begin_sampler();
    void end_sampler();

    void set_sample_mode(sample_mode m) { mode = m; }
    void set_sample_period(uint64_t p) { sample_period = p; }
    void set_sample_threshold(uint64_t t) { sample_threshold = t; }

    void set_handler_fn(sample_handler_fn_t h, void* args) { handler_fn = h; handler_fn_args = args; handler_fn_defined = 1; }
    void set_end_fn(end_fn_t h, void* args) { end_fn = h; end_fn_args = args; end_fn_defined = 1; }

    inline bool has_attribute(int attr) { return this->pe.sample_type & attr; }

private:
    void init_attr();
    int init_perf();
    int init_sighandler();

    int process_sample_buffer();
    int process_single_sample(struct perf_event_mmap_page *mmap_buf);

    int read_mmap_buffer(struct perf_event_mmap_page *mmap_buf, char *out, size_t sz);
    void skip_mmap_buffer(struct perf_event_mmap_page *mmap_buf, size_t sz);

    void process_lost_sample(struct perf_event_mmap_page *mmap_buf);
    void process_freq_sample(struct perf_event_mmap_page *mmap_buf);
    void process_exit_sample(struct perf_event_mmap_page *mmap_buf);

private:
    // status
    int ret;
    int ready;
    int stop;

    int mode;

    // perf_event variables
    int fd;
    pid_t mPID;
    struct perf_event_mmap_page *mmap_buf;
    perf_event_sample pes;
    uint64_t counter_value;

    // event_attr variables
    struct perf_event_attr pe;

    uint64_t sample_period;
    uint64_t sample_threshold;
    uint64_t sample_type;

    size_t mmap_pages;
    size_t mmap_size;
    size_t pgsz;
    size_t pgmsk;

    uint64_t collected_samples;
    uint64_t lost_samples;

    pthread_t sample_reader_thr;

    sample_handler_fn_t handler_fn;
    void *handler_fn_args;
    int handler_fn_defined;

    end_fn_t end_fn;
    void *end_fn_args;
    int end_fn_defined;
};

#endif
