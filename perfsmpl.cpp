#include "perfsmpl.h"

static void *sample_handler_fn(void *args)
{
    perf_event_prof *pep = (perf_event_prof*)args;

    for(;;) {
        pep->read_all_samples();
    }
}

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
       ready=0;
       return;
    }

    // Create mmap buffer for samples
    mmap_buf = (struct perf_event_mmap_page*)
        mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    if(mmap_buf == MAP_FAILED) {
       fprintf(stderr, "Error mmapping buffer\n");
       ready=0;
       return;
    }

    ready=1;
}

perf_event_prof::~perf_event_prof()
{
    munmap(mmap_buf,mmap_size);
    close(fd);
}

int perf_event_prof::init_sample_handler()
{
    return pthread_create(&sample_handler_thr,NULL,sample_handler_fn,(void*)this);
}

int perf_event_prof::begin_prof()
{
    ret = init_sample_handler();

    if(ret)
    {
        std::cerr << "Couldn't initialize sample handler thread, aborting!\n";
        std::cerr << std::endl;
        return ret;
    }

    ioctl(fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

    return ret;
}

void perf_event_prof::end_prof()
{
    pthread_cancel(sample_handler_thr);
    pthread_join(sample_handler_thr,NULL);

    read_all_samples(); // flush out remaining samples

    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    read(fd, &counter_value, sizeof(uint64_t));
}

void perf_event_prof::readout()
{
    *os_out << "**** Sampling Summary ****" << std::endl;
    *os_out << "counter value : " << counter_value << std::endl;
    *os_out << "collected samples : " << collected_samples << std::endl;
    *os_out << "lost samples : " << lost_samples << std::endl;
}

int perf_event_prof::read_single_sample()
{
    int ret;
    uint64_t val;

    // READ IP
    ret = read_mmap_buffer((char*)&val,sizeof(uint64_t));
    if(ret)
        *os_err << "Can't read mmap buffer!\n" << std::endl;
    else
        *os_out << "IP : " << std::hex << val << std::endl;

    // READ TIMESTAMP
    ret |= read_mmap_buffer((char*)&val,sizeof(uint64_t));
    if(ret)
        *os_err << "Can't read mmap buffer!\n" << std::endl;
    else
        *os_out << "TS : " << std::hex << val << std::endl;

    return ret;
}

int perf_event_prof::read_all_samples()
{
    struct perf_event_header ehdr;
    int ret;

    for(;;) {
        ret = read_mmap_buffer((char*)&ehdr,sizeof(ehdr));
        if(ret)
            return 0; // no more samples

        switch(ehdr.type) {
            case PERF_RECORD_SAMPLE:
                read_single_sample();
                break;
            case PERF_RECORD_EXIT:
                process_exit_sample();
                break;
            case PERF_RECORD_LOST:
                process_lost_sample();
                break;
            case PERF_RECORD_THROTTLE:
                process_freq_sample();
                break;
            case PERF_RECORD_UNTHROTTLE:
                process_freq_sample();
                break;
            default:
                //printf("unknown sample type %d\n", ehdr.type);
                skip_mmap_buffer(sizeof(ehdr));
        }
    }
}

int perf_event_prof::read_mmap_buffer(char *out, size_t sz)
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

void perf_event_prof::skip_mmap_buffer(size_t sz)
{
    if ((mmap_buf->data_tail + sz) > mmap_buf->data_head)
        sz = mmap_buf->data_head - mmap_buf->data_tail;

    mmap_buf->data_tail += sz;
}

void perf_event_prof::process_lost_sample()
{
	struct { uint64_t id, lost; } lost;
	const char *str;

	ret = read_mmap_buffer((char*)&lost,sizeof(lost));

    /*
	if (ret)
		errx(1, "cannot read lost info");

	e = perf_id2event(fds, num_fds, lost.id);
	if (e == -1)
		str = "unknown lost event";
	else
		str = fds[e].name;

	fprintf(options.output_file,
		"<<<LOST %"PRIu64" SAMPLES FOR EVENT %s>>>\n", lost.lost, str);
    */
	lost_samples += lost.lost;
}

void perf_event_prof::process_exit_sample()
{
	struct { pid_t pid, ppid, tid, ptid; } grp;
	int ret;

	ret = read_mmap_buffer((char*)&grp, sizeof(grp));
    /*
	if (ret)
		errx(1, "cannot read exit info");

	fprintf(options.output_file,"[%d] exited\n", grp.pid);
    */
}

void perf_event_prof::process_freq_sample()
{
	struct { uint64_t time, id, stream_id; } thr;
	int ret;

	ret = read_mmap_buffer((char*)&thr, sizeof(thr));
    /*
	if (ret)
		errx(1, "cannot read throttling info");

	fprintf(options.output_file,"%s value=%"PRIu64" event ID=%"PRIu64"\n", mode ? "Throttled" : "Unthrottled", thr.id, thr.stream_id);
    */
}
