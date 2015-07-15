#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include "Mitos.h"
#include "mmap_processor.h"

void skip_mmap_buffer(struct perf_event_mmap_page *mmap_buf, size_t sz)
{
    if ((mmap_buf->data_tail + sz) > mmap_buf->data_head)
        sz = mmap_buf->data_head - mmap_buf->data_tail;

    mmap_buf->data_tail += sz;
}

int read_mmap_buffer(struct perf_event_mmap_page *mmap_buf, size_t pgmsk, char *out, size_t sz)
{
	char *data;
	unsigned long tail;
	size_t avail_sz, m, c;

	data = ((char *)mmap_buf)+sysconf(_SC_PAGESIZE);
	tail = mmap_buf->data_tail & pgmsk;
	avail_sz = mmap_buf->data_head - mmap_buf->data_tail;
	if (sz > avail_sz)
		return -1;
	c = pgmsk + 1 -  tail;
	m = c < sz ? c : sz;
	memcpy(out, data+tail, m);
	if ((sz - m) > 0)
		memcpy(out+m, data, sz - m);
	mmap_buf->data_tail += sz;

	return 0;
}

void process_lost_sample(struct perf_event_mmap_page *mmap_buf, size_t pgmsk)
{
    int ret;
	struct { uint64_t id, lost; } lost;

	ret = read_mmap_buffer(mmap_buf, pgmsk, (char*)&lost, sizeof(lost));
}

void process_exit_sample(struct perf_event_mmap_page *mmap_buf, size_t pgmsk)
{
	int ret;
	struct { pid_t pid, ppid, tid, ptid; } grp;

	ret = read_mmap_buffer(mmap_buf, pgmsk, (char*)&grp, sizeof(grp));
}

void process_freq_sample(struct perf_event_mmap_page *mmap_buf, size_t pgmsk)
{
	int ret;
	struct { uint64_t time, id, stream_id; } thr;

	ret = read_mmap_buffer(mmap_buf, pgmsk, (char*)&thr, sizeof(thr));
}

int process_single_sample(struct perf_event_sample *pes, 
                          uint32_t event_type, 
                          sample_handler_fn_t handler_fn,
                          void* handler_fn_args,
                          struct perf_event_mmap_page *mmap_buf, 
                          size_t pgmsk)
{
    int ret = 0;

    memset(pes,0, sizeof(struct perf_event_sample));
    
    if(event_type &(PERF_SAMPLE_IP))
    {
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->ip, sizeof(uint64_t));
    }

    if(event_type &(PERF_SAMPLE_TID))
    {
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->pid, sizeof(uint32_t));
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->tid, sizeof(uint32_t));
    }

    if(event_type &(PERF_SAMPLE_TIME))
    {
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->time, sizeof(uint64_t));
    }

    if(event_type &(PERF_SAMPLE_ADDR))
    {
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->addr, sizeof(uint64_t));
    }

    if(event_type &(PERF_SAMPLE_ID))
    {
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->id, sizeof(uint64_t));
    }

    if(event_type &(PERF_SAMPLE_STREAM_ID))
    {
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->stream_id, sizeof(uint64_t));
    }

    if(event_type &(PERF_SAMPLE_CPU))
    {
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->cpu, sizeof(uint32_t));
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->res, sizeof(uint32_t));
    }

    if(event_type &(PERF_SAMPLE_PERIOD))
    {
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->period, sizeof(uint64_t));
    }

    /*
    if(event_type &(PERF_SAMPLE_CALLCHAIN))
    {
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->nr, sizeof(uint64_t));
        pes->ips = (uint64_t*)malloc(pes->nr*sizeof(uint64_t));
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)pes->ips,pes->nr*sizeof(uint64_t));
    }
    */

    if(event_type &(PERF_SAMPLE_WEIGHT))
    {
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->weight, sizeof(uint64_t));
    }

    if(event_type &(PERF_SAMPLE_DATA_SRC))
    {
        ret |= read_mmap_buffer(mmap_buf, pgmsk, (char*)&pes->data_src, sizeof(uint64_t));
    }

    if(handler_fn)
    {
        handler_fn(pes, handler_fn_args);
    }

    return ret;
}

int process_sample_buffer(struct perf_event_sample *pes,
                          uint32_t event_type, 
                          sample_handler_fn_t handler_fn,
                          void* handler_fn_args,
                          struct perf_event_mmap_page *mmap_buf, 
                          size_t pgmsk)
{
    int ret;
    struct perf_event_header ehdr;

    for(;;) 
    {
        ret = read_mmap_buffer(mmap_buf, pgmsk, (char*)&ehdr, sizeof(ehdr));
        if(ret)
            return 0; // no more samples

        switch(ehdr.type) 
        {
            case PERF_RECORD_SAMPLE:
                process_single_sample(pes, event_type, handler_fn, 
                                      handler_fn_args,mmap_buf, pgmsk);
                break;
            case PERF_RECORD_EXIT:
                process_exit_sample(mmap_buf, pgmsk);
                break;
            case PERF_RECORD_LOST:
                process_lost_sample(mmap_buf, pgmsk);
                break;
            case PERF_RECORD_THROTTLE:
                process_freq_sample(mmap_buf, pgmsk);
                break;
            case PERF_RECORD_UNTHROTTLE:
                process_freq_sample(mmap_buf, pgmsk);
                break;
            default:
                skip_mmap_buffer(mmap_buf, sizeof(ehdr));
        }
    }
}


