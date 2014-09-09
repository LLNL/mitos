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

class perf_event_prof
{
public:
    perf_event_prof();
    ~perf_event_prof();

    void begin_prof();
    void end_prof();

    void print_samples();

public:
    int read_single_sample();
    int read_all_samples();

private:
    int ret;

    int fd;

    struct perf_event_attr pe;
    struct perf_event_mmap_page *mmap_buf;

    uint64_t sample_period;
    uint64_t counter_value;
    uint64_t collected_samples;

    size_t mmap_pages;
    size_t mmap_size;
    size_t pgsz;
    size_t pgmsk;
};


// the following was shamelessly stolen from libpfm
int
perf_read_buffer(struct perf_event_mmap_page *hdr, size_t pgmsk, char *buf, size_t sz)
{
	char *data;
	unsigned long tail;
	size_t avail_sz, m, c;
	
	data = ((char *)hdr)+sysconf(_SC_PAGESIZE);
	tail = hdr->data_tail & pgmsk;
	avail_sz = hdr->data_head - hdr->data_tail;
	if (sz > avail_sz)
		return -1;
	c = pgmsk + 1 -  tail;
	m = c < sz ? c : sz;
	memcpy(buf, data+tail, m);
	if ((sz - m) > 0)
		memcpy(buf+m, data, sz - m);
	hdr->data_tail += sz;

	return 0;
}

void
perf_skip_buffer(struct perf_event_mmap_page *hdr, size_t sz)
{
    if ((hdr->data_tail + sz) > hdr->data_head)
        sz = hdr->data_head - hdr->data_tail;

    hdr->data_tail += sz;
}

