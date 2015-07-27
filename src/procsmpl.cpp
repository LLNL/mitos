#include "procsmpl.h"
#include "mmap_processor.h"

#include <poll.h>
#include <fcntl.h>

static __thread threadsmpl tsmp;

pid_t gettid(void)
{
	return (pid_t)syscall(__NR_gettid);
}

int perf_event_open(struct perf_event_attr *attr,
                    pid_t pid, int cpu, int group_fd,
                    unsigned long flags)
{
    return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}

void thread_sighandler(int sig, siginfo_t *info, void *extra)
{
    int fd = info->si_fd;

    process_sample_buffer(&tsmp.pes,
                          tsmp.proc_parent->attr.sample_type,
                          tsmp.proc_parent->handler_fn,
                          tsmp.proc_parent->handler_fn_args,
                          tsmp.mmap_buf,
                          tsmp.proc_parent->pgmsk);

    ioctl(fd, PERF_EVENT_IOC_REFRESH, 1);
}

procsmpl::procsmpl()
{
    // Defaults
    mmap_pages = 1;
    sample_period = 4000;
    sample_threshold = 7;
    pgsz = sysconf(_SC_PAGESIZE);
    mmap_size = (mmap_pages+1)*pgsz;
    pgmsk = mmap_pages*pgsz-1;

    sample_pid = 0;
    handler_fn = NULL;
}

procsmpl::~procsmpl()
{
}

void procsmpl::init_attr()
{
    // perf_event attr
    memset(&attr, 0, sizeof(struct perf_event_attr));
    attr.size = sizeof(struct perf_event_attr);

    attr.mmap = 1;
    attr.mmap_data = 1;
    attr.comm = 1;
    attr.disabled = 1;
    attr.exclude_user = 0;
    attr.exclude_kernel = 0;
    attr.exclude_hv = 0;
    attr.exclude_idle = 0;
    attr.exclude_host = 0;
    attr.exclude_guest = 1;
    attr.pinned = 0;
    attr.sample_id_all = 0;
    attr.wakeup_events = 1;

    attr.sample_period = sample_period;
    attr.freq = 0;

    // Set up PEBS load sampling
    attr.type = PERF_TYPE_RAW;
    attr.config = 0x5101cd;          // MEM_TRANS_RETIRED:LATENCY_THRESHOLD
    attr.config1 = sample_threshold; // latency threshold
    attr.sample_type =
        PERF_SAMPLE_IP |
        //PERF_SAMPLE_CALLCHAIN |
        PERF_SAMPLE_ID |
        PERF_SAMPLE_STREAM_ID |
        PERF_SAMPLE_TIME |
        PERF_SAMPLE_TID |
        PERF_SAMPLE_PERIOD |
        PERF_SAMPLE_CPU |
        PERF_SAMPLE_ADDR |
        PERF_SAMPLE_WEIGHT |
        PERF_SAMPLE_DATA_SRC;
    attr.precise_ip = 2;
}

void procsmpl::prepare(pid_t p)
{
    // Sampling on PID p
    sample_pid = p;

    // Set up perf_event_attr
    init_attr();
}

int procsmpl::begin_sampling()
{
    int ret = tsmp.init(this);
    if(ret)
        return ret;

    return tsmp.begin_sampling();
}

void procsmpl::end_sampling()
{
    tsmp.end_sampling();
}

int threadsmpl::init(procsmpl *parent)
{
    int ret;
    
    ready = 0;

    proc_parent = parent;

    ret = init_perf_event(&proc_parent->attr, proc_parent->mmap_size);
    if(ret)
        return ret;

    ret = init_thread_sighandler();
    if(ret)
        return ret;

    // Success
    ready = 1;

    return 0;
}

int threadsmpl::init_perf_event(struct perf_event_attr *attr, size_t mmap_size)
{
    // Create attr according to sample mode
    fd = perf_event_open(attr, gettid(), -1, -1, 0);
    if(fd == -1)
    {
        perror("perf_event_open");
        return 1;
    }

    // Create mmap buffer for samples
    mmap_buf = (struct perf_event_mmap_page*)
        mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(mmap_buf == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    return 0;
}

int threadsmpl::init_thread_sighandler()
{
    int ret;
    struct f_owner_ex fown_ex;
    struct sigaction sact;

    // Set up signal handler
    memset(&sact, 0, sizeof(sact));
    sact.sa_sigaction = &thread_sighandler;
    sact.sa_flags = SA_SIGINFO;

    ret = sigaction(SIGIO, &sact, NULL);
    if(ret)
    {
        perror("sigaction");
        return ret;
    }

    // Unblock SIGIO signal if necessary
    /*
    sigset_t sold, snew;
    sigemptyset(&sold);
    sigemptyset(&snew);
    sigaddset(&snew, SIGIO);

    ret = sigprocmask(SIG_SETMASK, NULL, &sold);
    if(ret)
    {
        perror("sigaction");
        return 1;
    }

	if(sigismember(&sold, SIGIO))
    {
		ret = sigprocmask(SIG_UNBLOCK, &snew, NULL);
        if(ret)
        {
            perror("sigaction");
            return 1;
        }
    } 
    */

    // Set perf event fd to signal
    ret = fcntl(fd, F_SETSIG, SIGIO);
    if(ret)
    {
        perror("fcntl");
        return 1;
    }
    ret = fcntl(fd, F_SETFL, O_NONBLOCK | O_ASYNC);
    if(ret)
    {
        perror("fcntl");
        return 1;
    }
    // Set owner to current thread
    fown_ex.type = F_OWNER_TID;
    fown_ex.pid = gettid();
    ret = fcntl(fd, F_SETOWN_EX, (unsigned long)&fown_ex);
    if(ret)
    {
        perror("fcntl");
        return 1;
    } 

    return 0;
}


int threadsmpl::begin_sampling()
{
    int ret;

    if(!ready)
    {
        std::cerr << "Not ready to begin sampling!" << std::endl;
        return -1;
    }

    ret = ioctl(fd, PERF_EVENT_IOC_RESET, 1);
    if(ret)
        perror("ioctl");

    ret = ioctl(fd, PERF_EVENT_IOC_ENABLE, 1);
    if(ret)
        perror("ioctl");

    return ret;
}

void threadsmpl::end_sampling()
{
    int ret;
    
    ret = ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    if(ret)
        perror("ioctl");

    ret = read(fd, &counter_value, sizeof(uint64_t));
    if(ret == -1)
        perror("read");
}

const char* Mitos_hit_type(struct perf_event_sample *s)
{
    uint64_t lvl_bits = s->data_src >> PERF_MEM_LVL_SHIFT;

    if(lvl_bits & PERF_MEM_LVL_NA)
        return "Not Available";
    else if(lvl_bits & PERF_MEM_LVL_HIT)
        return "Hit";
    else if(lvl_bits & PERF_MEM_LVL_MISS)
        return "Miss";

    return "Invalid Data Source";
}

const char* Mitos_data_source(struct perf_event_sample *s)
{
    uint64_t lvl_bits = s->data_src >> PERF_MEM_LVL_SHIFT;

    if(lvl_bits & PERF_MEM_LVL_NA)
        return "Not Available";
    else if(lvl_bits & PERF_MEM_LVL_L1)
        return "L1";
    else if(lvl_bits & PERF_MEM_LVL_LFB)
        return "LFB";
    else if(lvl_bits & PERF_MEM_LVL_L2)
        return "L2";
    else if(lvl_bits & PERF_MEM_LVL_L3)
        return "L3";
    else if(lvl_bits & PERF_MEM_LVL_LOC_RAM)
        return "Local RAM";
    else if(lvl_bits & PERF_MEM_LVL_REM_RAM1)
        return "Remote RAM 1 Hop";
    else if(lvl_bits & PERF_MEM_LVL_REM_RAM2)
        return "Remote RAM 2 Hops";
    else if(lvl_bits & PERF_MEM_LVL_REM_CCE1)
        return "Remote Cache 1 Hops";
    else if(lvl_bits & PERF_MEM_LVL_REM_CCE2)
        return "Remote Cache 2 Hops";
    else if(lvl_bits & PERF_MEM_LVL_IO)
        return "I/O Memory";
    else if(lvl_bits & PERF_MEM_LVL_UNC)
        return "Uncached Memory";

    return "Invalid Data Source";
}

