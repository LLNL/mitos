#include "perfsmpl.h"

static void *sample_handler_fn(void *args)
{
    perf_event_prof *pep = (perf_event_prof*)args;

    while(!pep->stop) {
        pep->read_all_samples();
    }
}

perf_event_prof::perf_event_prof()
{
    // Defaults
    mmap_pages = 8;
    sample_period = 10;
    pgsz = sysconf(_SC_PAGESIZE);
    mmap_size = (mmap_pages+1)*pgsz;
    pgmsk = mmap_pages*pgsz-1;

    os_out = &std::cout;
    os_err = &std::cerr;

    ret = 0;
    ready = 0;
    stop = 0;

    collected_samples = 0;
    lost_samples = 0;

}

perf_event_prof::~perf_event_prof()
{
    munmap(mmap_buf,mmap_size);
    close(fd);
}

int perf_event_prof::prepare_perf()
{
    // Setup
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_REF_CPU_CYCLES;
    pe.sample_type = PERF_SAMPLE_CALLCHAIN;
    pe.sample_period = sample_period;
    pe.mmap = 1;
    pe.mmap_data = 1;
    pe.disabled = 1;
    pe.exclude_hv = 1;
    pe.exclude_kernel = 1;

    // Calling pid, all cpus, single event group, no flags
    fd = syscall(__NR_perf_event_open, &pe,0,-1,-1,0);

    if (fd == -1) {
       *os_err << "Error opening leader " << pe.config << std::endl;
       ready=0;
       return -1;
    }

    // Create mmap buffer for samples
    mmap_buf = (struct perf_event_mmap_page*)
        mmap(NULL, mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    if(mmap_buf == MAP_FAILED) {
       *os_err << "Error mmap-ing buffer " << std::endl;
       ready=0;
       return -1;
    }

    ready=1;

    return 0;
}

int perf_event_prof::prepare_symtab()
{
    avail_line_num = Dyninst::SymtabAPI::Symtab::openFile(symtab_obj,"/proc/self/exe");
}

int perf_event_prof::prepare()
{
    ret = 0;
    ret |= prepare_perf();
    ret |= prepare_symtab();

    if(ret == 0)
        ready = 1;

    return ret;
}

int perf_event_prof::init_sample_handler()
{
    return pthread_create(&sample_handler_thr,NULL,sample_handler_fn,(void*)this);
}

int perf_event_prof::begin_prof()
{
    if(!ready)
    {
        *os_err << "Not ready to begin sampling!\n" << std::endl;
        *os_err << "Did you prepare()?\n" << std::endl;
        return -1;
    }

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
    stop = 1;
    pthread_join(sample_handler_thr,NULL);

    read_all_samples(); // flush out remaining samples

    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    read(fd, &counter_value, sizeof(uint64_t));
}

void perf_event_prof::readout()
{
    *os_out << "**** Sampling Summary ****" << std::endl;
    *os_out << std::dec;
    *os_out << "counter value : " << counter_value << std::endl;
    *os_out << "collected samples : " << collected_samples << std::endl;
    *os_out << "lost samples : " << lost_samples << std::endl;
    os_out->flush();
}

int perf_event_prof::read_single_sample()
{
    uint64_t val;
    uint64_t type = pe.sample_type;

    ret = 0;

    //struct {
    //    struct perf_event_header header;
    //    u64   sample_id;  /* if PERF_SAMPLE_IDENTIFIER */
    //    u64   ip;         /* if PERF_SAMPLE_IP */
    //    u32   pid, tid;   /* if PERF_SAMPLE_TID */
    //    u64   time;       /* if PERF_SAMPLE_TIME */
    //    u64   addr;       /* if PERF_SAMPLE_ADDR */
    //    u64   id;         /* if PERF_SAMPLE_ID */
    //    u64   stream_id;  /* if PERF_SAMPLE_STREAM_ID */
    //    u32   cpu, res;   /* if PERF_SAMPLE_CPU */
    //    u64   period;     /* if PERF_SAMPLE_PERIOD */
    //    struct read_format v; /* if PERF_SAMPLE_READ */
    //    u64   nr;         /* if PERF_SAMPLE_CALLCHAIN */
    //    u64   ips[nr];    /* if PERF_SAMPLE_CALLCHAIN */
    //    u32   size;       /* if PERF_SAMPLE_RAW */
    //    char  data[size]; /* if PERF_SAMPLE_RAW */
    //    u64   bnr;        /* if PERF_SAMPLE_BRANCH_STACK */
    //    struct perf_branch_entry lbr[bnr];
    //                    /* if PERF_SAMPLE_BRANCH_STACK */
    //    u64   abi;        /* if PERF_SAMPLE_REGS_USER */
    //    u64   regs[weight(mask)];
    //                    /* if PERF_SAMPLE_REGS_USER */
    //    u64   size;       /* if PERF_SAMPLE_STACK_USER */
    //    char  data[size]; /* if PERF_SAMPLE_STACK_USER */
    //    u64   dyn_size;   /* if PERF_SAMPLE_STACK_USER */
    //    u64   weight;     /* if PERF_SAMPLE_WEIGHT */
    //    u64   data_src;   /* if PERF_SAMPLE_DATA_SRC */
    //    u64   transaction;/* if PERF_SAMPLE_TRANSACTION */
    //};


    if(type & PERF_SAMPLE_IP)
    {
        ret |= read_mmap_buffer((char*)&val,sizeof(uint64_t));
        if(ret)
            *os_err << "Can't read mmap buffer!\n" << std::endl;
        else
        {
            //*os_out << "IP : " << std::hex << val << std::endl;

            if(avail_line_num)
            {
                std::vector<Dyninst::SymtabAPI::Statement*> stats;
                ret = symtab_obj->getSourceLines(stats,val);
                if(ret)
                {
                    *os_out << "Source : " << stats[0]->getFile() << std::endl;
                    *os_out << "Line : " << stats[0]->getLine() << std::endl;
                }
            }
        }

    }

    if(type & PERF_SAMPLE_TID)
    {
        struct { uint32_t pid, tid; } pid;
        ret |= read_mmap_buffer((char*)&pid,sizeof(pid));
        if(ret)
            *os_err << "Can't read mmap buffer!\n" << std::endl;
        else
        {
            *os_out << "PID : " << std::dec << pid.pid << std::endl;
            *os_out << "TID : " << std::dec << pid.tid << std::endl;
        }
    }

    if(type & PERF_SAMPLE_TIME)
    {
        ret |= read_mmap_buffer((char*)&val,sizeof(uint64_t));
        if(ret)
            *os_err << "Can't read mmap buffer!\n" << std::endl;
        else
            *os_out << "TS : " << std::hex << val << std::endl;
    }

    if(type & PERF_SAMPLE_CPU)
    {
        struct { uint32_t cpu, reserved; } cpu;    
        ret |= read_mmap_buffer((char*)&cpu,sizeof(cpu));
        if(ret)
            *os_err << "Can't read mmap buffer!\n" << std::endl;
        else
            *os_out << "CPU : " << std::dec << cpu.cpu << std::endl;
    }

    if(type & PERF_SAMPLE_CALLCHAIN)
    {
        ret |= read_mmap_buffer((char*)&val,sizeof(uint64_t));

        uint64_t *ips = new uint64_t[val];

        ret |= read_mmap_buffer((char*)ips,val*sizeof(uint64_t));

        *os_out << " **** CALLCHAIN **** " << std::endl;

        for(int i=0; i<val; i++)
        {
            if(avail_line_num)
            {
                std::vector<Dyninst::SymtabAPI::Statement*> stats;
                ret = symtab_obj->getSourceLines(stats,ips[i]);
                if(ret)
                {
                    *os_out << "Source : " << stats[0]->getFile() << std::endl;
                    *os_out << "Line : " << stats[0]->getLine() << std::endl;
                }
            }
        }
    }

    collected_samples++;

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

    //*os_err << "thr.id : " << thr.id << std::endl;
    //*os_err << "thr.stream_id : " << thr.stream_id << std::endl;
}
