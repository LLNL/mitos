#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <pthread.h>

#include <iostream>
#include <iomanip>

static void *sample_handler_fn(void *args);

class perf_event_prof
{
    friend void *sample_handler_fn(void *args);

public:
    perf_event_prof();
    ~perf_event_prof();

    int begin_prof();
    void end_prof();

    void set_outputstream(std::ostream *os) { this->os_out = os; }
    void set_errorstream(std::ostream *os) { this->os_err = os; }

private:
    int init_sample_handler();
    int read_single_sample();
    int read_all_samples();

    int read_mmap_buffer(char *out, size_t sz);
    void skip_mmap_buffer(size_t sz);
    
    void process_lost_sample();
    void process_freq_sample();
    void process_exit_sample();
    
private:
    int ret;
    int ready;

    int fd;

    struct perf_event_attr pe;
    struct perf_event_mmap_page *mmap_buf;

    uint64_t sample_period;
    uint64_t counter_value;

    uint64_t collected_samples;
    uint64_t lost_samples;

    size_t mmap_pages;
    size_t mmap_size;
    size_t pgsz;
    size_t pgmsk;

    std::ostream *os_out;
    std::ostream *os_err;
    pthread_t sample_handler_thr;
};
