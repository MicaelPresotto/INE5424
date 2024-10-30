// EPOS CPU Scheduler Component Implementation

#include <process.h>
#include <time.h>

__BEGIN_SYS

volatile unsigned int Variable_Queue_Scheduler::_next_queue;

inline RT_Common::Tick RT_Common::elapsed() { return Alarm::elapsed(); }

RT_Common::Tick RT_Common::ticks(Microsecond time) {
    return Timer_Common::ticks(time, Alarm::timer()->frequency());
}

Microsecond RT_Common::time(Tick ticks) {
    return Timer_Common::time(ticks, Alarm::timer()->frequency());
}

// The following Scheduling Criteria depend on Alarm, which is not available at scheduler.h
template <typename ... Tn>
FCFS::FCFS(int p, Tn & ... an): Priority((p == IDLE) ? IDLE : Alarm::elapsed()) {}

// Since the definition above is only known to this unit, forcing its instantiation here so it gets emitted in scheduler.o for subsequent linking with other units is necessary.
template FCFS::FCFS<>(int p);

void EDFEnergyAwareness::handle(Event event) {
    db<Thread>(TRC) << "RT::handle(this=" << this << ",e=";
    if(event & CREATE) {
        db<Thread>(TRC) << "CREATE";
    }
    if(event & FINISH) {
        db<Thread>(TRC) << "FINISH";
    }
    if(event & ENTER) {
        db<Thread>(TRC) << "ENTER";
    }
    if(event & LEAVE) {
        db<Thread>(TRC) << "LEAVE";
    }
    if(periodic() && (event & JOB_RELEASE)) {
        db<Thread>(TRC) << "RELEASE";
        _priority = elapsed() + _deadline;
        _statistics.thread_last_dispatch = elapsed();
    }
    if(periodic() && (event & JOB_FINISH)) {
        db<Thread>(TRC) << "WAIT";
    }
    db<Thread>(TRC) << ") => {i=" << _priority << ",p=" << _period << ",d=" << _deadline << ",c=" << _capacity << "}" << endl;
}

void EDFEnergyAwareness::updateFrequency() {
    unsigned long long thread_last_dispatch = time(_statistics.thread_last_dispatch);
    unsigned long long job_release = time(_statistics.job_release);
    unsigned long long deadline = period();
    unsigned long long percentage = 0;
    unsigned long long elapsed_time = job_release - thread_last_dispatch;

    if (elapsed_time && deadline) percentage = (100ULL*elapsed_time) / deadline;

    unsigned long new_freq = calculateFrequency(percentage);
    CPU::clock(new_freq);
}

unsigned long long EDFEnergyAwareness::calculateFrequency(unsigned long long percentage) {
    unsigned long long factor;

    if (percentage <= 10) factor = 198ULL;
    else if (percentage <= 20) factor = 357ULL;
    else if (percentage <= 30) factor = 485ULL;
    else if (percentage <= 40) factor = 587ULL;
    else if (percentage <= 50) factor = 669ULL;
    else if (percentage <= 60) factor = 735ULL;
    else if (percentage <= 70) factor = 787ULL;
    else if (percentage <= 80) factor = 829ULL;
    else factor = 1000ULL;

    return CPU::min_clock() + (((CPU::max_clock() - CPU::min_clock()) * factor)/1000ULL);
}

EDF::EDF(Microsecond p, Microsecond d, Microsecond c, unsigned int cpu): RT_Common(int(elapsed() + ticks(d)), p, d, c) {}

__END_SYS
