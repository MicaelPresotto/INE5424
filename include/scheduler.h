// EPOS Scheduler Component Declarations

#ifndef __scheduler_h
#define __scheduler_h

#include <architecture/cpu.h>
#include <architecture/pmu.h>
#include <architecture/tsc.h>
#include <utility/scheduling.h>
#include <utility/math.h>
#include <utility/convert.h>
#include <utility/list.h>

__BEGIN_SYS

// All scheduling criteria, or disciplines, must define operator int() with
// the semantics of returning the desired order of a given object within the
// scheduling list
class Scheduling_Criterion_Common
{
    friend class Thread;                // for handle(), init 
    friend class Periodic_Thread;       // for handle()
    friend class RT_Thread;             // for handle()

protected:
    typedef Timer_Common::Tick Tick;

public:
    // Priorities
    enum : int {
        CEILING = -1000,
        MAIN    = -1,
        HIGH    = 0,
        NORMAL  = (unsigned(1) << (sizeof(int) * 8 - 3)) - 1,
        LOW     = (unsigned(1) << (sizeof(int) * 8 - 2)) - 1,
        IDLE    = (unsigned(1) << (sizeof(int) * 8 - 1)) - 1
    };

    // Constructor helpers
    enum : unsigned int {
        SAME        = 0,
        NOW         = 0,
        UNKNOWN     = 0,
        ANY         = -1U
    };

    // Policy types
    enum : int {
        PERIODIC    = HIGH,
        SPORADIC    = NORMAL,
        APERIODIC   = LOW
    };

    // Policy events
    typedef int Event;
    enum {
        CREATE          = 1 << 0,
        FINISH          = 1 << 1,
        ENTER           = 1 << 2,
        LEAVE           = 1 << 3,
        JOB_RELEASE     = 1 << 4,
        JOB_FINISH      = 1 << 5
    };

    // Policy operations
    typedef int Operation;
    enum {
        COLLECT         = 1 << 16,
        CHARGE          = 1 << 17,
        AWARD           = 1 << 18,
        UPDATE          = 1 << 19
    };

    // Policy traits
    static const bool timed = false;
    static const bool dynamic = false;
    static const bool preemptive = true;
    static const unsigned int QUEUES = 1;
    struct EnergyAwarenessStatistics {  
        Tick job_release;
        Tick thread_last_dispatch;

        Tick current_execution_time = 0L;
        Tick total_execution_time = 0L;
        unsigned long long executions = 0ULL;

        Tick job_utilization;
    };

    typedef EnergyAwarenessStatistics Statistics;

protected:
    Scheduling_Criterion_Common() {}

public:
    Microsecond period() { return 0;}
    Microsecond deadline() { return 0; }
    Microsecond capacity() { return 0; }

    bool periodic() { return false; }

    volatile Statistics & statistics() { return _statistics; }
    void updateFrequency(){};
    unsigned int queue() const { return 0; }

protected:
    void handle(Event event) {}
    void queue(unsigned int q) {}
    void update() {}

    static void init() {}

protected:
    Statistics _statistics;
};

// Priority (static and dynamic)
class Priority: public Scheduling_Criterion_Common
{
public:
    template <typename ... Tn>
    Priority(int p = NORMAL, Tn & ... an): _priority(p) {}

    operator const volatile int() const volatile { return _priority; }

protected:
    volatile int _priority;
};

// Round-Robin
class RR: public Priority
{
public:
    static const bool timed = true;
    static const bool dynamic = false;
    static const bool preemptive = true;

public:
    template <typename ... Tn>
    RR(int p = NORMAL, Tn & ... an): Priority(p) {}
};

// First-Come, First-Served (FIFO)
class FCFS: public Priority
{
public:
    static const bool timed = false;
    static const bool dynamic = false;
    static const bool preemptive = false;

public:
    template <typename ... Tn>
    FCFS(int p = NORMAL, Tn & ... an);
};


// Multicore Algorithms
class Variable_Queue_Scheduler
{
protected:
    Variable_Queue_Scheduler(unsigned int queue): _queue(queue) {};

    const volatile unsigned int & queue() const volatile { return _queue; }
    void queue(unsigned int q) { _queue = q; }

protected:
    volatile unsigned int _queue;
    static volatile unsigned int _next_queue;
};

// Global Round-Robin
class GRR: public RR
{
public:
    static const unsigned int HEADS = Traits<Machine>::CPUS;

public:
    template <typename ... Tn>
    GRR(int p = NORMAL, Tn & ... an): RR(p) {}

    static unsigned int current_head() { return CPU::id(); }
};

// Fixed CPU (fully partitioned)
class Fixed_CPU: public Priority, public Variable_Queue_Scheduler
{
public:
    static const bool timed = true;
    static const bool dynamic = false;
    static const bool preemptive = true;

    static const unsigned int QUEUES = Traits<Machine>::CPUS;

public:
    template <typename ... Tn>
    Fixed_CPU(int p = NORMAL, unsigned int cpu = ANY, Tn & ... an)
    : Priority(p), Variable_Queue_Scheduler(((_priority == IDLE) || (_priority == MAIN)) ? CPU::id() : (cpu != ANY) ? cpu : ++_next_queue %= CPU::cores()) {}

    using Variable_Queue_Scheduler::queue;
    static unsigned int current_queue() { return CPU::id(); }
};

// CPU Affinity
class CPU_Affinity: public Priority, public Variable_Queue_Scheduler
{
public:
    static const bool timed = true;
    static const bool dynamic = false;
    static const bool preemptive = true;
    static const bool heuristic = true;
    static const unsigned int QUEUES = Traits<Machine>::CPUS;

public:
    template <typename ... Tn>
    CPU_Affinity(int p = NORMAL, unsigned int cpu = ANY, Tn & ... an)
    : Priority(p), Variable_Queue_Scheduler(((_priority == IDLE) || (_priority == MAIN)) ? CPU::id() : (cpu != ANY) ? cpu : ++_next_queue %= CPU::cores()) {}

    bool charge(bool end = false);
    bool award(bool end = false);

    using Variable_Queue_Scheduler::queue;
    static unsigned int current_queue() { return CPU::id(); }
};

// Real-time Algorithms
class RT_Common: public Priority
{
    friend class FCFS;
    friend class Thread;                // for handle() and queue()
    friend class Periodic_Thread;       // for handle() and queue()
    friend class RT_Thread;             // for handle() and queue()

public:
    static const bool timed = true;
    static const bool preemptive = true;

protected:
    RT_Common(int i): Priority(i), _period(0), _deadline(0), _capacity(0) {} // aperiodic
    RT_Common(int i, Microsecond p, Microsecond d, Microsecond c): Priority(i), _period(ticks(p)), _deadline(ticks(d ? d : p)), _capacity(ticks(c)) {}

public:
    Microsecond period() { return time(_period); }
    Microsecond deadline() { return time(_deadline); }
    Microsecond capacity() { return time(_capacity); }

    bool periodic() { return (_priority >= PERIODIC) && (_priority <= SPORADIC); }

    volatile Statistics & statistics() { return _statistics; }

protected:
    Tick ticks(Microsecond time);
    Microsecond time(Tick ticks);

    void handle(Event event);

    static Tick elapsed();

protected:
    Tick _period;
    Tick _deadline;
    Tick _capacity;
    Statistics _statistics;
};

// Rate Monotonic
class RM:public RT_Common
{
public:
    static const bool dynamic = false;

public:
    RM(int p = APERIODIC): RT_Common(p) {}
    RM(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN, unsigned int cpu = ANY): RT_Common(int(ticks(p)), p, d, c) {}
};

// Deadline Monotonic
class DM: public RT_Common
{
public:
    static const bool dynamic = false;

public:
    DM(int p = APERIODIC): RT_Common(p) {}
    DM(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN, unsigned int cpu = ANY): RT_Common(int(ticks(d ? d : p)), p, d, c) {}
};

// Laxity Monotonic
class LM: public RT_Common
{
public:
    static const bool dynamic = false;

public:
    LM(int p = APERIODIC): RT_Common(p) {}
    LM(Microsecond p, Microsecond d, Microsecond c, unsigned int cpu = ANY): RT_Common(int(ticks((d ? d : p) - c)), p, d, c) {}
};

// Earliest Deadline First
class EDF: public RT_Common
{
public:
    static const bool dynamic = true;

public:
    EDF(int p = APERIODIC): RT_Common(p) {}
    EDF(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN, unsigned int cpu = ANY);

    void handle(Event event);
};


class EDFEnergyAwareness : public EDF {

public:

    EDFEnergyAwareness(int p = APERIODIC) : EDF(p) {}
    EDFEnergyAwareness(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN) : EDF(p, d, c) {}

    void handle(Event event);
    void updateFrequency();
    unsigned long long calculateFrequency(unsigned long long frequency);
};


class GEDFEnergyAwareness : public EDFEnergyAwareness {

public:
    static const unsigned int HEADS = Traits<Machine>::CPUS;
public:

    GEDFEnergyAwareness(int p = APERIODIC) : EDFEnergyAwareness(p) {}
    GEDFEnergyAwareness(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN) : EDFEnergyAwareness(p, d, c) {}
    void updateFrequency();
    static unsigned int current_head() { return CPU::id(); }
};

class EDFEnergyAwarenessAffinity: public EDFEnergyAwareness, public Variable_Queue_Scheduler
{
public:
    static const bool dynamic = true;
    static const unsigned int QUEUES = Traits<Machine>::CPUS;

public:

    EDFEnergyAwarenessAffinity(int p = APERIODIC)
    : EDFEnergyAwareness(p), Variable_Queue_Scheduler(((_priority == IDLE) || (_priority == MAIN) ) ? CPU::id() : define_best_queue()) {}

    EDFEnergyAwarenessAffinity(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN) : EDFEnergyAwareness(p, d, c), Variable_Queue_Scheduler(((_priority == IDLE) || (_priority == MAIN)) ? CPU::id(): define_best_queue()) {}

    using Variable_Queue_Scheduler::queue;
    static unsigned int current_queue() { return CPU::id(); }

    unsigned long define_best_queue();
};

// Least Laxity First
class LLF: public RT_Common
{
public:
    static const bool dynamic = true;

public:
    LLF(int p = APERIODIC): RT_Common(p) {}
    LLF(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN, unsigned int cpu = ANY);

    void handle(Event event);
};

__END_SYS

__BEGIN_UTIL

// Scheduling Queues
template<typename T>
class Scheduling_Queue<T, GRR>:
public Multihead_Scheduling_List<T> {};

template<typename T>
class Scheduling_Queue<T, Fixed_CPU>:
public Scheduling_Multilist<T> {};

template<typename T>
class Scheduling_Queue<T, CPU_Affinity>:
public Scheduling_Multilist<T> {};

template<typename T>
class Scheduling_Queue<T, EDFEnergyAwarenessAffinity>:
public Scheduling_Multilist<T> {};

template<typename T>
class Scheduling_Queue<T, GEDFEnergyAwareness>:
public Multihead_Scheduling_List<T> {};

__END_UTIL

#endif
