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
#include <vector>

class perfsmpl;
class perf_event_sample;

typedef void (*sample_handler_fn_t)(perf_event_sample *sample, void *args);
void* sample_reader_fn(void *args);

enum sample_mode
{
    SMPL_MEMORY,
    SMPL_INSTRUCTIONS
};

class perfsmpl
{
    friend void *sample_reader_fn(void *args);
    friend class perf_event_sample;

public:
    perfsmpl();
    ~perfsmpl();

    int prepare();
    void prepare(pid_t PID) { mPID=PID; prepare(); }

    int begin_sampler();
    void end_sampler();

    void set_sample_mode(sample_mode m) { this->mode = m; }
    void set_sample_period(uint64_t p) { sample_period = p; }
    void set_sample_threshold(uint64_t t) { sample_threshold = t; }

    void set_handler(sample_handler_fn_t h) { this->handler = h; custom_handler = 1; }

    inline bool has_attribute(int attr) { return this->pe.sample_type & attr; }

private:
    void init_attr();
    int init_perf();
    int init_sample_reader();

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
    int custom_handler;

    // perf_event variables
    int fd;
    pid_t mPID;
    struct perf_event_mmap_page *mmap_buf;
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

    sample_handler_fn_t handler;
};

class perf_event_sample {

    friend class perfsmpl;

public:
    perf_event_sample() { memset(this,0,sizeof(class perf_event_sample)); }
    inline bool has_attribute(int attr) { return parent->has_attribute(attr); }

private:
    perfsmpl *parent;

public:
    // Raw perf_event sample data

    //struct perf_event_header header;
    uint64_t   sample_id;  /* if PERF_SAMPLE_IDENTIFIER */
    uint64_t   ip;         /* if PERF_SAMPLE_IP */
    uint32_t   pid, tid;   /* if PERF_SAMPLE_TID */
    uint64_t   time;       /* if PERF_SAMPLE_TIME */
    uint64_t   addr;       /* if PERF_SAMPLE_ADDR */
    uint64_t   id;         /* if PERF_SAMPLE_ID */
    uint64_t   stream_id;  /* if PERF_SAMPLE_STREAM_ID */
    uint32_t   cpu, res;   /* if PERF_SAMPLE_CPU */
    uint64_t   period;     /* if PERF_SAMPLE_PERIOD */
    //struct read_format v; /* if PERF_SAMPLE_READ */
    uint64_t   nr;         /* if PERF_SAMPLE_CALLCHAIN */
    uint64_t  *ips;    /* if PERF_SAMPLE_CALLCHAIN */
    uint32_t   raw_size;       /* if PERF_SAMPLE_RAW */
    char      *raw_data; /* if PERF_SAMPLE_RAW */
    uint64_t   bnr;        /* if PERF_SAMPLE_BRANCH_STACK */
    //struct perf_branch_entry *lbr; /* if PERF_SAMPLE_BRANCH_STACK */
    uint64_t   abi;        /* if PERF_SAMPLE_REGS_USER */
    uint64_t  *regs; /* if PERF_SAMPLE_REGS_USER */
    uint64_t   stack_size;       /* if PERF_SAMPLE_STACK_USER */
    char      *stack_data; /* if PERF_SAMPLE_STACK_USER */
    uint64_t   dyn_size;   /* if PERF_SAMPLE_STACK_USER */
    uint64_t   weight;     /* if PERF_SAMPLE_WEIGHT */
    uint64_t   data_src;   /* if PERF_SAMPLE_DATA_SRC */
    uint64_t   transaction;/* if PERF_SAMPLE_TRANSACTION */
};

