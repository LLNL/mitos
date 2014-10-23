#include "perfsmpl.h"

static void *sample_reader_fn(void *args)
{
    perf_event_prof *pep = (perf_event_prof*)args;

    while(!pep->stop) 
    {
        pep->process_sample_buffer();
    }
}

perf_event_prof::perf_event_prof()
{
    // Defaults
    mmap_pages = 32;
    sample_period = 10;
    pgsz = sysconf(_SC_PAGESIZE);
    mmap_size = (mmap_pages+1)*pgsz;
    pgmsk = mmap_pages*pgsz-1;

    ret = 0;
    ready = 0;
    stop = 0;

    custom_handler = 0;

    collected_samples = 0;
    lost_samples = 0;

    // event attr
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.size = sizeof(struct perf_event_attr);
    pe.type = PERF_TYPE_RAW;
    pe.config = 0x1cd;
    pe.config1 = 3; // ldlat
    pe.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_ADDR | PERF_SAMPLE_WEIGHT | PERF_SAMPLE_DATA_SRC;
    pe.precise_ip = 2;
    pe.sample_period = 4000;
    pe.freq = 0;
    pe.mmap = 1;
    pe.mmap_data = 1;
    pe.comm = 1;
    pe.disabled = 1;
    pe.exclude_user = 0;
    pe.exclude_kernel = 0;
    pe.exclude_hv = 0;
    pe.exclude_idle = 0;
    pe.exclude_host = 0;
    pe.exclude_guest = 1;
    pe.pinned = 0;
}

perf_event_prof::~perf_event_prof()
{
    close(fd);
    munmap(mmap_buf,mmap_size);
}

int perf_event_prof::prepare_perf()
{
    // Setup
    fd = syscall(__NR_perf_event_open, &pe,0,-1,-1,0);

    if(fd == -1) 
    {
       std::cerr << "Error from perf_event_open syscall" << std::endl;
       ready=0;
       return -1;
    }

    // Create mmap buffer for samples
    mmap_buf = (struct perf_event_mmap_page*)
        mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    if(mmap_buf == MAP_FAILED) 
    {
       std::cerr << "Error mmap-ing buffer " << std::endl;
       ready = 0;
       return -1;
    }

    ready = 1;

    return 0;
}

int perf_event_prof::prepare()
{
    ret = prepare_perf();

    if(ret != 0)
    {
        ready = 0;
        return ret;
    }

    ready = 1;

    return 0;
}

int perf_event_prof::init_sample_reader()
{
    return pthread_create(&sample_reader_thr,NULL,sample_reader_fn,(void*)this);
}

int perf_event_prof::begin_prof()
{
    if(!ready)
    {
        std::cerr << "Not ready to begin sampling!\n" << std::endl;
        std::cerr << "Did you prepare()?\n" << std::endl;
        return -1;
    }

    ret = init_sample_reader();

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
    stop = 1;
    pthread_join(sample_reader_thr,NULL);

    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    read(fd, &counter_value, sizeof(uint64_t));

    process_sample_buffer(); // flush out remaining samples
}

size_t perf_event_prof::sample_size()
{
    size_t sz = 0;
    if(has_attribute(PERF_SAMPLE_IP))
        sz += sizeof(uint64_t);
    if(has_attribute(PERF_SAMPLE_TID))
        sz += sizeof(uint64_t);
    if(has_attribute(PERF_SAMPLE_TIME))
        sz += sizeof(uint64_t);
    if(has_attribute(PERF_SAMPLE_ADDR))
        sz += sizeof(uint64_t);
    if(has_attribute(PERF_SAMPLE_CPU))
        sz += sizeof(uint64_t);
    if(has_attribute(PERF_SAMPLE_PERIOD))
        sz += sizeof(uint64_t);
    if(has_attribute(PERF_SAMPLE_WEIGHT))
        sz += sizeof(uint64_t);
    if(has_attribute(PERF_SAMPLE_DATA_SRC))
        sz += sizeof(uint64_t);

    return sz;
}

int perf_event_prof::process_single_sample(struct perf_event_mmap_page *mmap_buf)
{
    // Read a sample from the mmap buf
    char *sample_data = (char*)malloc(sample_size());
    ret = read_mmap_buffer(mmap_buf,sample_data,sample_size());
    if(ret)
    {
        std::cerr << "Can't read mmap buffer!\n" << std::endl;
        return -1;
    }

    collected_samples++;

    // Create and fill up a new sample
    perf_event_sample *sample = new perf_event_sample();
    sample->parent = this;

    if(has_attribute(PERF_SAMPLE_IP))
    {
        memcpy(&sample->ip,sample_data,sizeof(uint64_t));
        sample_data += sizeof(uint64_t);
    }
    if(has_attribute(PERF_SAMPLE_TID))
    {
        memcpy(&sample->pid,sample_data,sizeof(uint32_t));
        sample_data += sizeof(uint32_t);
        memcpy(&sample->tid,sample_data,sizeof(uint32_t));
        sample_data += sizeof(uint32_t);
    }

    if(has_attribute(PERF_SAMPLE_TIME))
    {
        memcpy(&sample->time,sample_data,sizeof(uint64_t));
        sample_data += sizeof(uint64_t);
    }

    if(has_attribute(PERF_SAMPLE_ADDR))
    {
        memcpy(&sample->addr,sample_data,sizeof(uint64_t));
        sample_data += sizeof(uint64_t);
    }

    if(has_attribute(PERF_SAMPLE_CPU))
    {
        memcpy(&sample->cpu,sample_data,sizeof(uint32_t));
        sample_data += sizeof(uint32_t);
        memcpy(&sample->res,sample_data,sizeof(uint32_t));
        sample_data += sizeof(uint32_t);
    }

    if(has_attribute(PERF_SAMPLE_PERIOD))
    {
        memcpy(&sample->period,sample_data,sizeof(uint64_t));
        sample_data += sizeof(uint64_t);
    }

    if(has_attribute(PERF_SAMPLE_WEIGHT))
    {
        memcpy(&sample->weight,sample_data,sizeof(uint64_t));
        sample_data += sizeof(uint64_t);
    }

    if(has_attribute(PERF_SAMPLE_DATA_SRC))
    {
        memcpy(&sample->data_src,sample_data,sizeof(uint64_t));
        sample_data += sizeof(uint64_t);
    }

    if(custom_handler)
    {
        handler(sample,NULL);
    }

    return ret;
}

int perf_event_prof::process_sample_buffer()
{
    struct perf_event_header ehdr;
    int ret;

    for(;;) {
        ret = read_mmap_buffer(mmap_buf,(char*)&ehdr,sizeof(ehdr));
        if(ret)
            return 0; // no more samples

        switch(ehdr.type) {
            case PERF_RECORD_SAMPLE:
                process_single_sample(mmap_buf);
                break;
            case PERF_RECORD_EXIT:
                process_exit_sample(mmap_buf);
                break;
            case PERF_RECORD_LOST:
                process_lost_sample(mmap_buf);
                break;
            case PERF_RECORD_THROTTLE:
                process_freq_sample(mmap_buf);
                break;
            case PERF_RECORD_UNTHROTTLE:
                process_freq_sample(mmap_buf);
                break;
            default:
                std::cerr << "Unknown sample type ";
                std::cerr << ehdr.type << std::endl;
                skip_mmap_buffer(mmap_buf,sizeof(ehdr));
        }
    }
}

int perf_event_prof::read_mmap_buffer(struct perf_event_mmap_page *mmap_buf, char *out, size_t sz)
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

void perf_event_prof::skip_mmap_buffer(struct perf_event_mmap_page *mmap_buf, size_t sz)
{
    if ((mmap_buf->data_tail + sz) > mmap_buf->data_head)
        sz = mmap_buf->data_head - mmap_buf->data_tail;

    mmap_buf->data_tail += sz;
}

void perf_event_prof::process_lost_sample(struct perf_event_mmap_page *mmap_buf)
{
	struct { uint64_t id, lost; } lost;
	const char *str;

	ret = read_mmap_buffer(mmap_buf,(char*)&lost,sizeof(lost));

	lost_samples += lost.lost;
}

void perf_event_prof::process_exit_sample(struct perf_event_mmap_page *mmap_buf)
{
	struct { pid_t pid, ppid, tid, ptid; } grp;
	int ret;

	ret = read_mmap_buffer(mmap_buf,(char*)&grp,sizeof(grp));
}

void perf_event_prof::process_freq_sample(struct perf_event_mmap_page *mmap_buf)
{
	struct { uint64_t time, id, stream_id; } thr;
	int ret;

	ret = read_mmap_buffer(mmap_buf,(char*)&thr, sizeof(thr));
}

