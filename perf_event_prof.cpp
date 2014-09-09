#include "perf_event_prof.h"

perf_event_prof::perf_event_prof()
{
    // Defaults
    ret = 0;
    mmap_pages = 8;
    sample_period = 10;
    pgsz = sysconf(_SC_PAGESIZE);
    mmap_size = (mmap_pages+1)*pgsz;
    pgmsk = mmap_pages*pgsz-1;

    // Setup
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_INSTRUCTIONS;
    pe.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TIME;
    pe.mmap = 1;
    pe.mmap_data = 1;
    pe.sample_period = sample_period;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    // Calling pid, all cpus, single event group, no flags
    fd = syscall(__NR_perf_event_open, &pe,0,-1,-1,0);

    if (fd == -1) {
       fprintf(stderr, "Error opening leader %llx\n", pe.config);
       exit(EXIT_FAILURE);
    }

    // Create mmap buffer for samples
    mmap_buf = (struct perf_event_mmap_page*)
        mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    if(mmap_buf == MAP_FAILED) {
       fprintf(stderr, "Error mmapping buffer\n");
       exit(EXIT_FAILURE);
    }
}

int perf_event_prof::read_single_sample()
{
    int ret;
    uint64_t val;

    // READ IP
    ret = perf_read_buffer(mmap_buf,pgmsk,(char*)&val,sizeof(uint64_t));
    if(ret)
        printf("Can't read buffer!\n");
    else
        printf("IP : %llx\n",val);

    // READ TIMESTAMP
    ret = perf_read_buffer(mmap_buf,pgmsk,(char*)&val,sizeof(uint64_t));
    if(ret)
        printf("Can't read buffer!\n");
    else
        printf("TS : %llx\n",val);
}

void perf_event_prof::begin_prof()
{
    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
}

void perf_event_prof::end_prof()
{
    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    read(fd, &counter_value, sizeof(uint64_t));
}

int perf_event_prof::read_all_samples()
{
    struct perf_event_header ehdr;
    int ret;

    for(;;) {
        ret = perf_read_buffer(mmap_buf,pgmsk,(char*)&ehdr,sizeof(ehdr));
        if(ret)
            return 0;

        switch(ehdr.type) {
            case PERF_RECORD_SAMPLE:
                read_single_sample();
                break;
            case PERF_RECORD_EXIT:
                //display_exit();
                //break;
            case PERF_RECORD_LOST:
                //display_lost();
                //break;
            case PERF_RECORD_THROTTLE:
                //display_freq();
                //break;
            case PERF_RECORD_UNTHROTTLE:
                //display_freq();
                //break;
            default:
                //printf("unknown sample type %d\n", ehdr.type);
                perf_skip_buffer(mmap_buf,sizeof(ehdr));
        }
    }
}

perf_event_prof::~perf_event_prof()
{
    munmap(mmap_buf,mmap_size);
    close(fd);
}

void
do_werk()
{
    double v = 241209092.548394035;
    for(int i=0; i<512; i++)
        for(int j=0; j<512; j++)
            for(int k=0; k<512; k++)
                v = v*v;
    printf("v : %f\n",v);
}

int
main(int argc, char **argv)
{
    perf_event_prof mprof;
    mprof.begin_prof();
    do_werk();
    mprof.end_prof();
    mprof.read_all_samples();
}
