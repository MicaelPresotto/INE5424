// EPOS CPU Scheduler Component Implementation

#include <process.h>
#include <time.h>

#define ULL unsigned long long

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
    }
    if(event & ENTER) {
        db<Thread>(TRC) << "ENTER";
    }
    if(event & LEAVE) {
        db<Thread>(TRC) << "LEAVE";
        _statistics.current_execution_time += (elapsed() - _statistics.thread_last_dispatch) * CPU::get_clock_percentage();
        _statistics.branch_instructions_retired = PMU::read(6);
        _statistics.cache_hits = PMU::read(5);
        _statistics.cache_misses = PMU::read(4);
        _statistics.branch_mispredictions = PMU::read(3);
        _statistics.instructions_retired = PMU::read(2);
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

    if (Thread::self()->criterion() != IDLE && Thread::self()->criterion().periodic()) {
        total_time += Thread::self()->get_remaining_time();
        if (current_time + (total_time / CPU::get_clock_percentage()) > Thread::self()->criterion()) {
            // db<EDFEnergyAwareness>(DEV) << Thread::self()->get_name() << " perde deadline" << endl;
            return true;
        }
        // db<EDFEnergyAwareness>(DEV) << Thread::self()->get_name() << " nao perde deadline" << endl;
    }

    for(auto it = Thread::get_scheduler().begin(); it != Thread::get_scheduler().end() && iterations; ++it, --iterations) {
        auto current_thread = (*it).object();

        if (current_thread->criterion() == IDLE || !current_thread->criterion().periodic()) continue;

        Tick remaining_time = current_thread->get_remaining_time();
        total_time += remaining_time;

        Tick deadline = current_thread->criterion();

        if (current_time + (total_time / CPU::get_clock_percentage()) > deadline) {
            // db<EDFEnergyAwareness>(DEV) << Thread::self()->get_name() << " perde deadline" << endl;
            return true;
        }
        // db<EDFEnergyAwareness>(DEV) << Thread::self()->get_name() << " nao perde deadline" << endl;
    }
    return false;
}

int EDFEnergyAwareness::findNextStep(Tick current_time, bool is_deadline_lost) {
    int current_step = CPU::get_clock_step();
    int new_step = -1;
    int direction = is_deadline_lost ? 1 : -1;
    int limit_step = is_deadline_lost ? 14 : 0;

    // db<EDFEnergyAwareness>(DEV) << "current_step: " << current_step << " is_deadline_lost: " << is_deadline_lost << " queue size: " << Thread::get_scheduler().size() << endl;

    for (int next_step = current_step + direction; next_step != limit_step; next_step += direction) {
        Tick total_time = 0UL;
        Tick current_deadline = Thread::get_scheduler().chosen()->criterion();
        int iterations = EDFEnergyAwareness::threads_ahead;
        bool stop = is_deadline_lost;
        long diff = next_step - current_step;
        ULL percentage_on_step = CPU::get_percentage_by_step(CPU::get_clock_step() + diff);

        if (Thread::get_scheduler().chosen()->criterion() != IDLE && Thread::get_scheduler().chosen()->criterion().periodic()) {
            total_time = Thread::get_scheduler().chosen()->get_remaining_time();
            if (current_time + (Tick)(total_time / percentage_on_step) > current_deadline) {
                if (is_deadline_lost) continue;
                new_step = next_step + 1;
                break;
            }
        }

        // db<EDFEnergyAwareness>(DEV) << "diff: " << diff << " iterations: " << iterations << endl;

        for (auto it = Thread::get_scheduler().begin(); it != Thread::get_scheduler().end() && iterations; ++it, --iterations) {
            auto current_thread = (*it).object();
            if (current_thread->criterion() == IDLE || !current_thread->criterion().periodic()) continue;
            total_time += current_thread->get_remaining_time();
            current_deadline = current_thread->criterion();
            // db<EDFEnergyAwareness>(DEV) << "elapsed: " << current_time << " total_time: " << total_time / percentage_on_step << " deadline: " << current_deadline << endl;

            if (current_time + (Tick)(total_time / percentage_on_step) > current_deadline) {
                stop = !stop;
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
    if (new_step > 13) new_step = 13;
    if (new_step < 0) new_step = 0;
    int current_step = CPU::get_clock_step();
    if (new_step == current_step) return;
    if ((current_step - new_step) > 3) new_step = current_step - 3;
    Hertz new_freq = CPU::get_frequency_by_step(new_step);
    CPU::clock(new_freq);
    db<EDFEnergyAwareness>(DEV) << elapsed();
    db<EDFEnergyAwareness>(DEV) << " {" << Thread::self()->get_name();
    db<EDFEnergyAwareness>(DEV) << " rt=" << Thread::self()->get_remaining_time() / CPU::get_clock_percentage();
    db<EDFEnergyAwareness>(DEV) << " d=" << Thread::self()->criterion();

    for(auto it = Thread::get_scheduler().begin(); it != Thread::get_scheduler().end(); ++it) {
        db<EDFEnergyAwareness>(DEV) << ", " << (*it).object()->get_name();
        db<EDFEnergyAwareness>(DEV) << " rt=" << (*it).object()->get_remaining_time() / CPU::get_clock_percentage();
        db<EDFEnergyAwareness>(DEV) << " d=" << (*it).object()->criterion();
    }

    db<EDFEnergyAwareness>(DEV) << "} ";
    db<EDFEnergyAwareness>(DEV) << "step " << current_step << " -> " << new_step << (current_step > new_step ? " - " : " + ") << endl;
}

int CPU::last_update[Traits<Machine>::CPUS] = {0};
const int rate_limit_us = 10000; //10ms

EDFEnergyAwareness::PMUStatistics EDFEnergyAwareness::get_pmu_statistics(int core) {
    Scheduling_Multilist<Thread, EDFEnergyAwarenessAffinity> scheduler = Thread::get_scheduler();
    Thread* current_thread = scheduler.chosen(core)->object();
    // Pega os dados de instructions_retired, cache misses/hits e branch mispredictions da atual cpu
    ULL instructions_retired = current_thread->statistics().instructions_retired;
    ULL cache_hits = current_thread->statistics().cache_hits;
    ULL cache_misses = current_thread->statistics().cache_misses;
    ULL branch_mispredictions = current_thread->statistics().branch_mispredictions;
    ULL branch_instructions_retired = current_thread->statistics().branch_instructions_retired;

    for(auto it = scheduler.begin(core); it != scheduler.end(); ++it){ 
        current_thread = (*it).object();
        if (current_thread->criterion() == IDLE || current_thread->criterion() == MAIN) continue;
        instructions_retired += current_thread->statistics().instructions_retired;
        cache_hits += current_thread->statistics().cache_hits;
        cache_misses += current_thread->statistics().cache_misses;
        branch_mispredictions += current_thread->statistics().branch_mispredictions;
        branch_instructions_retired += current_thread->statistics().branch_instructions_retired;
    }

    db<EDFEnergyAwareness>(TRC) << "Branch Mispredictions " << branch_mispredictions;
    db<EDFEnergyAwareness>(TRC) << " | Branch Instructions Retired " << branch_instructions_retired;
    db<EDFEnergyAwareness>(TRC) << " | Instructions Retired " << instructions_retired;
    db<EDFEnergyAwareness>(TRC) << " | Cache Misses " << cache_misses;
    db<EDFEnergyAwareness>(TRC) << " | Cache Hits " << cache_hits;
    db<EDFEnergyAwareness>(TRC) << " | " << scheduler.size() + 1 << endl;
    
    EDFEnergyAwareness::PMUStatistics pmu_statistics;
    pmu_statistics.define_statistics(cache_misses, cache_hits, branch_mispredictions, branch_instructions_retired);

    db<EDFEnergyAwareness>(TRC) << "Cache misses rate: " << pmu_statistics.cache_misses_rate << endl;
    db<EDFEnergyAwareness>(TRC) << "Branch Mispredictions Rate : " << pmu_statistics.branch_mispredictions_rate << endl;

    return pmu_statistics;
}

int EDFEnergyAwareness::evaluate_performance_metrics() {

    EDFEnergyAwareness::PMUStatistics stats = get_pmu_statistics(CPU::id());

    if (stats.cache_misses_rate > 10 && stats.branch_mispredictions_rate > 2) return -1;
    if (stats.cache_misses_rate > 10 && stats.branch_mispredictions_rate < 2) return 0;
    if (stats.cache_misses_rate < 10 && stats.branch_mispredictions_rate > 2) return 0;
    return 1;
}

void EDFEnergyAwareness::updateFrequency() {
    const Tick current_time = elapsed();
    const Microsecond current_time_us = time(current_time);

    if (current_time_us - CPU::last_update[CPU::id()] < rate_limit_us) return;
    CPU::last_update[CPU::id()] = current_time_us;

    int new_step_pmu = evaluate_performance_metrics();
    bool is_deadline_loss = checkDeadlineLoss(current_time);
    int new_step_soft = findNextStep(current_time, is_deadline_loss);

    int new_step = new_step_soft + new_step_pmu;

    applyNewFrequency(new_step);
}

unsigned long EDFEnergyAwarenessAffinity::get_avg_core_time(int core) {
    Scheduling_Multilist<Thread, EDFEnergyAwarenessAffinity> scheduler = Thread::get_scheduler();
    Thread* current_thread = scheduler.chosen(core)->object();
    unsigned long avg_core_time = 0UL;
    unsigned long last_deadline = 0UL;
    if (current_thread) {
        avg_core_time = current_thread->get_remaining_time();
        last_deadline = current_thread->criterion();
    }

    for(auto it = scheduler.begin(core); it != scheduler.end(); ++it){ 
        current_thread = (*it).object();
        if (current_thread->criterion() != IDLE) {
            avg_core_time += current_thread->criterion().statistics().avg_execution_time;
            last_deadline = current_thread->criterion();
        }
    }
    if (!last_deadline) return 0;
    return (100UL * avg_core_time) / last_deadline;
}

long EDFEnergyAwarenessAffinity::define_best_queue() {

    unsigned long best_core = 0UL;
    unsigned long min_avg_core_performance = 0UL;
    bool first = true;
    unsigned long avg_core_performance;

    for(unsigned long core = 0UL; core < CPU::cores(); core++){
        avg_core_performance = evaluate_core_performance(core);  
        if(first || avg_core_performance < min_avg_core_performance) {
            best_core = core;
            min_avg_core_performance = avg_core_performance;
            first = false;
        }
    }
    return best_core;
}


int last_migration[Traits<Machine>::CPUS] = {0};
const int migration_limit_us = 100000; // 100ms

int EDFEnergyAwarenessAffinity::find_best_cpu_to_migrate() {
    const Microsecond current_time_us = time(elapsed());

    if (current_time_us - last_migration[CPU::id()] < migration_limit_us) return -1;

    int best_core = define_best_queue();

    if (periodic() && _queue != (unsigned int) best_core) {
        last_migration[CPU::id()] = current_time_us;
        return best_core;
    }
    return -1;
}

unsigned long EDFEnergyAwarenessAffinity::evaluate_core_performance(int core) {
    EDFEnergyAwarenessAffinity::PMUStatistics pmu_stats = get_pmu_statistics(core);
    unsigned long core_time = get_avg_core_time(core);
    return evaluate_performance(pmu_stats, core_time);
}

unsigned long EDFEnergyAwarenessAffinity::evaluate_performance(EDFEnergyAwarenessAffinity::PMUStatistics pmu_stats, unsigned long time_) {
    return (50ULL * pmu_stats.cache_misses_rate) + (30ULL * pmu_stats.branch_mispredictions_rate) + (20ULL * time_);
}

EDF::EDF(Microsecond p, Microsecond d, Microsecond c, unsigned int cpu): RT_Common(int(elapsed() + ticks(d)), p, d, c) {}

__END_SYS
