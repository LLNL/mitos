#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

int
perf_read_buffer(struct perf_event_mmap_page *hdr, size_t pgmsk, void *buf, size_t sz)
{
	void *data;
	unsigned long tail;
	size_t avail_sz, m, c;
	
	data = ((void *)hdr)+sysconf(_SC_PAGESIZE);
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


static long
perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                   group_fd, flags);
    return ret;
}

static size_t pgmsk;

void
read_sample(void *mmap_buf)
{
    int ret;
    uint64_t val;

    // READ IP
    ret = perf_read_buffer(mmap_buf,pgmsk,&val,sizeof(uint64_t));
    if(ret)
        printf("Can't read buffer!\n");
    else
        printf("IP : %llx\n",val);

    // READ TIMESTAMP
    ret = perf_read_buffer(mmap_buf,pgmsk,&val,sizeof(uint64_t));
    if(ret)
        printf("Can't read buffer!\n");
    else
        printf("TS : %llx\n",val);
}


void
read_samples(void *mmap_buf)
{
    struct perf_event_header ehdr;
    int ret;

    for(;;) {
        ret = perf_read_buffer(mmap_buf, pgmsk, &ehdr, sizeof(ehdr));
        if(ret)
            return;
        //printf("HEADER size=%d misc=%d type=%d\n",ehdr.size,ehdr.misc,ehdr.type);

        switch(ehdr.type) {
            case PERF_RECORD_SAMPLE:
                read_sample(mmap_buf);
                break;
            case PERF_RECORD_EXIT:
                printf("exit!\n");
                break;
            case PERF_RECORD_LOST:
                printf("lost!\n");
                break;
            case PERF_RECORD_THROTTLE:
                printf("throttle!\n");
                break;
            case PERF_RECORD_UNTHROTTLE:
                printf("unthrottle!\n");
                break;
            default:
                printf("unknown sample type %d\n", ehdr.type);
        }
    }
}

int
main(int argc, char **argv)
{
    struct perf_event_attr pe;
    void *mmap_buf;
    long long count;
    int fd;
    size_t pgsz = sysconf(_SC_PAGESIZE);
    int mmap_pages = 8;
    size_t map_size = (mmap_pages+1)*pgsz;

    pgmsk = mmap_pages*pgsz-1;

    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TIME;
    pe.mmap = 1;
    pe.mmap_data = 1;
    pe.sample_period = 10;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (fd == -1) {
       fprintf(stderr, "Error opening leader %llx\n", pe.config);
       exit(EXIT_FAILURE);
    }

    mmap_buf = mmap(NULL, map_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(mmap_buf == MAP_FAILED) {
       fprintf(stderr, "Error mmapping buffer\n");
       exit(EXIT_FAILURE);
    }

    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

    printf("Measuring instruction count for this printf\n");
    read_samples(mmap_buf);

    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    read(fd, &count, sizeof(long long));

    printf("Used %lld instructions\n", count);

    munmap(mmap_buf,map_size);

    close(fd);
}

