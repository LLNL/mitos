#ifndef MITOS_H
#define MITOS_H

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

struct mem_symbol;
struct perf_event_sample;
struct sample_buffer;

typedef void (*sample_handler_fn_t)(struct perf_event_sample *sample, void *args);
typedef void (*end_fn_t)(void *args);

enum sample_mode
{
    SMPL_MEMORY,
    SMPL_INSTRUCTIONS
};

/*
 * Mitos: 
 * All programs must invoke from these functions.
 */

#ifdef __cplusplus
extern "C" {
#endif

// Sampler configuration
void Mitos_set_sample_mode(enum sample_mode m);
void Mitos_set_sample_period(uint64_t p);
void Mitos_set_sample_threshold(uint64_t t);
void Mitos_set_handler_fn(sample_handler_fn_t h, void *args);
void Mitos_set_end_fn(end_fn_t h, void *args);

// Sampler invocation
void Mitos_prepare(pid_t pid);
void Mitos_begin_sampler();
void Mitos_end_sampler();

// Memory attribution
void Mitos_add_symbol(const char* n, void *a, size_t s, size_t *dims, unsigned int ndims);
void Mitos_resolve_symbol(struct perf_event_sample *s);

// Get friendly sample information
size_t Mitos_x_index(struct perf_event_sample *s);
size_t Mitos_y_index(struct perf_event_sample *s);
size_t Mitos_z_index(struct perf_event_sample *s);
const char* Mitos_hit_type(struct perf_event_sample *s);
const char* Mitos_data_source(struct perf_event_sample *s);

#ifdef __cplusplus
} // extern "C"
#endif

/*
 * perf_event_sample:
 * Struct containing all raw perf event information
 */

struct perf_event_sample 
{
    //struct perf_event_header header;
    uint64_t   sample_id;           /* if PERF_SAMPLE_IDENTIFIER */
    uint64_t   ip;                  /* if PERF_SAMPLE_IP */
    uint32_t   pid, tid;            /* if PERF_SAMPLE_TID */
    uint64_t   time;                /* if PERF_SAMPLE_TIME */
    uint64_t   addr;                /* if PERF_SAMPLE_ADDR */
    uint64_t   id;                  /* if PERF_SAMPLE_ID */
    uint64_t   stream_id;           /* if PERF_SAMPLE_STREAM_ID */
    uint32_t   cpu, res;            /* if PERF_SAMPLE_CPU */
    uint64_t   period;              /* if PERF_SAMPLE_PERIOD */
    //struct read_format v;         /* if PERF_SAMPLE_READ */
    uint64_t   nr;                  /* if PERF_SAMPLE_CALLCHAIN */
    uint64_t  *ips;                 /* if PERF_SAMPLE_CALLCHAIN */
    uint32_t   raw_size;            /* if PERF_SAMPLE_RAW */
    char      *raw_data;            /* if PERF_SAMPLE_RAW */
    uint64_t   bnr;                 /* if PERF_SAMPLE_BRANCH_STACK */
    //struct perf_branch_entry *lbr; /* if PERF_SAMPLE_BRANCH_STACK */
    uint64_t   abi;                 /* if PERF_SAMPLE_REGS_USER */
    uint64_t  *regs;                /* if PERF_SAMPLE_REGS_USER */
    uint64_t   stack_size;          /* if PERF_SAMPLE_STACK_USER */
    char      *stack_data;          /* if PERF_SAMPLE_STACK_USER */
    uint64_t   dyn_size;            /* if PERF_SAMPLE_STACK_USER */
    uint64_t   weight;              /* if PERF_SAMPLE_WEIGHT */
    uint64_t   data_src;            /* if PERF_SAMPLE_DATA_SRC */
    uint64_t   transaction;         /* if PERF_SAMPLE_TRANSACTION */

    size_t num_dims;
    size_t *access_index;
    const char *data_symbol;
};

#endif
