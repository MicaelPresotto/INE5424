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

void EDFEnergyAwareness::handle(Event event, Thread *current) {
    db<Thread>(TRC) << "RT::handle(this=" << this << ",e=";
    if(event & UPDATE) {
        db<Thread>(TRC) << "UPDATE";
    }
    if(event & CREATE) {
        db<Thread>(TRC) << "CREATE";
    }
    if(event & FINISH) {
        db<Thread>(TRC) << "FINISH";
        // _statistics.total_execution_time += _statistics.current_execution_time;
        // _statistics.current_execution_time = 0;
    }
    if(event & ENTER) {
        db<Thread>(TRC) << "ENTER";
    }
    if(event & LEAVE) {
        db<Thread>(TRC) << "LEAVE";
        _statistics.current_execution_time += (elapsed() - _statistics.thread_last_dispatch) * CPU::get_clock_percentage();
    }
    if(periodic() && (event & JOB_RELEASE)) {
        db<Thread>(TRC) << "RELEASE";
        _priority = elapsed() + _deadline;
        _statistics.job_release = elapsed();
    }
    if(periodic() && (event & JOB_FINISH)) {
        db<Thread>(TRC) << "WAIT";

        if (_statistics.executions == 0)
            _statistics.total_execution_time = _statistics.current_execution_time;
        else
            _statistics.total_execution_time += _statistics.current_execution_time;        

        _statistics.executions += 1ULL;
        _statistics.avg_execution_time = _statistics.total_execution_time / _statistics.executions;
        _statistics.current_execution_time = 0L;
    }
    
    db<Thread>(TRC) << ") => {i=" << _priority << ",p=" << _period << ",d=" << _deadline << ",c=" << _capacity << "}" << endl;
    db<Thread>(TRC) << " | Tempo de execução total: " << _statistics.total_execution_time << " | " << current << " / " << Thread::self() << endl;
    db<Thread>(TRC) << "Tempo de execução atual: " << _statistics.current_execution_time << " | " << current << " / " << Thread::self() << endl;
    db<Thread>(TRC) << "Tempo medio de execução: " << _statistics.avg_execution_time << " | " << current << " / " << Thread::self() << endl;
    db<Thread>(TRC) << "Execuçoes: " << _statistics.executions << " | " << current << " / " << Thread::self() << endl << endl;
}

bool EDFEnergyAwareness::checkDeadlineLoss(Tick current_time) {
    Tick total_time = 0;
    int iterations = EDFEnergyAwareness::threads_ahead;

    for(auto it = Thread::get_scheduler().begin(); it != Thread::get_scheduler().end() && iterations; ++it, --iterations) {
        auto current_thread = (*it).object();

        if (current_thread->criterion() == IDLE || !current_thread->criterion().periodic()) continue;

        Tick remaining_time = current_thread->get_remaining_time();
        total_time += remaining_time;

        Tick deadline = current_thread->criterion();

        if (current_time + (total_time / CPU::get_clock_percentage()) > deadline) {
            return true;
        }
    }
    return false;
}

int EDFEnergyAwareness::findNextStep(Tick current_time, bool is_deadline_lost) {
    int current_step = CPU::get_clock_step();
    int new_step = -1;
    int direction = is_deadline_lost ? 1 : -1;
    int limit_step = is_deadline_lost ? 14 : 0;
    
    db<EDFEnergyAwareness>(DEV) << "current_step: " << current_step << " is_deadline_lost: " << is_deadline_lost << endl;

    for (int next_step = current_step + direction; next_step != limit_step; next_step += direction) {
        Tick total_time = 0;
        int iterations = EDFEnergyAwareness::threads_ahead;
        bool stop = false;
        long diff = next_step - current_step;

        db<EDFEnergyAwareness>(DEV) << "diff: " << diff << " iterations: " << iterations << endl;

        for (auto it = Thread::get_scheduler().begin(); it != Thread::get_scheduler().end() && iterations; ++it, --iterations) {
            auto current_thread = (*it).object();
            if (current_thread->criterion() == IDLE || !current_thread->criterion().periodic()) continue;

            unsigned long long new_execution_time = (10000ULL - diff * 625ULL) * current_thread->get_remaining_time() / 100ULL;
            total_time += new_execution_time;
            Tick current_deadline = current_thread->criterion();

            db<EDFEnergyAwareness>(DEV) << "current_time: " << current_time << " new_execution: " << new_execution_time / CPU::get_clock_percentage() << " total_time: " << total_time / CPU::get_clock_percentage() << " current_deadline: " << current_deadline << endl;

            if (current_time + (total_time / CPU::get_clock_percentage()) > current_deadline) {
                stop = !is_deadline_lost;
                break;
            }
        }

        if (stop) {
            new_step = is_deadline_lost ? next_step : next_step + 1;
            break;
        }
    }

    return (new_step == -1) ? (is_deadline_lost ? 13: 1) : new_step;
}

void EDFEnergyAwareness::applyNewFrequency(int new_step) {
    Hertz new_freq = CPU::get_frequency_by_step(new_step);
    CPU::clock(new_freq);
    db<EDFEnergyAwareness>(DEV) << "new_step: " << new_step << " new_freq: " << new_freq << endl;
}

int CPU::last_update[Traits<Machine>::CPUS] = {0};

void EDFEnergyAwareness::updateFrequency() {
    CPU::last_update[CPU::id()]++;
    db<CPU>(TRC) << "LAST UPDATE [" << CPU::id() << "] = " << CPU::last_update[CPU::id()] << " | THREAD = " << Thread::self() <<  endl;
    if (CPU::last_update[CPU::id()] < 2) return;
    CPU::last_update[CPU::id()] = 0;

    const Tick current_time = elapsed();

    bool is_deadline_loss = checkDeadlineLoss(current_time);
    int new_step = findNextStep(current_time, is_deadline_loss);
    applyNewFrequency(new_step);
}

unsigned long EDFEnergyAwarenessAffinity::define_best_queue(){
    // a unica funcao q ta funcioando, advinha qm fez
    unsigned long smallest_queue = 0UL;
    unsigned long min_avg_thread_time = 0UL;
    bool first = true;

    for(unsigned long nqueue = 0UL; nqueue < CPU::cores(); nqueue++){
        unsigned long avg_queue_thread_time = 0UL;
        for(auto it = Thread::get_scheduler().begin(nqueue); it != Thread::get_scheduler().end(); ++it){ 
            auto current_element = *it;
            if (current_element.object()->criterion() != IDLE) avg_queue_thread_time += current_element.object()->criterion().statistics().avg_execution_time;
        }
        if(first || avg_queue_thread_time < min_avg_thread_time) {
            smallest_queue = nqueue;
            min_avg_thread_time = avg_queue_thread_time;
            first = false;
        }
        db<EDFEnergyAwarenessAffinity>(DEV) << "define_best_queue: " << "CPU [" << nqueue << "] = " << avg_queue_thread_time << " / " << Thread::get_scheduler().size(nqueue) << endl;
    }
    db<EDFEnergyAwarenessAffinity>(DEV) << "define_best_queue: " << "Chosen CPU [" << smallest_queue << "]" << endl;
    return smallest_queue;
}

EDF::EDF(Microsecond p, Microsecond d, Microsecond c, unsigned int cpu): RT_Common(int(elapsed() + ticks(d)), p, d, c) {}

__END_SYS
