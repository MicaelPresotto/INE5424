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
        _statistics.current_execution_time += (elapsed() - _statistics.thread_last_dispatch) * (CPU::clock() * 100ULL / CPU::max_clock());
    }
    if(periodic() && (event & JOB_RELEASE)) {
        db<Thread>(TRC) << "RELEASE";
        _priority = elapsed() + _deadline;
        _statistics.job_release = elapsed();
    }
    if(periodic() && (event & JOB_FINISH)) {
        db<Thread>(TRC) << "WAIT";
        _statistics.total_execution_time += _statistics.current_execution_time;
        _statistics.current_execution_time = 0L;
        _statistics.executions += 1ULL;
        _statistics.avg_execution_time = _statistics.total_execution_time / _statistics.executions;
    }
    
    db<Thread>(TRC) << ") => {i=" << _priority << ",p=" << _period << ",d=" << _deadline << ",c=" << _capacity << "}" << endl;
    db<Thread>(TRC) << " | Tempo de execução total: " << _statistics.total_execution_time << " | " << current << " / " << Thread::self() << endl;
    db<Thread>(TRC) << "Tempo de execução atual: " << _statistics.current_execution_time << " | " << current << " / " << Thread::self() << endl;
    db<Thread>(TRC) << "Tempo medio de execução: " << _statistics.avg_execution_time << " | " << current << " / " << Thread::self() << endl;
    db<Thread>(TRC) << "Execuçoes: " << _statistics.executions << " | " << current << " / " << Thread::self() << endl << endl;
}

int CPU::last_update[Traits<Machine>::CPUS] = {0};

void EDFEnergyAwareness::updateFrequency() {
    CPU::last_update[CPU::id()]++;
    db<CPU>(TRC) << "LAST UPDATE [" << CPU::id() << "] = " << CPU::last_update[CPU::id()] << " | THREAD = " << Thread::self() <<  endl;
    if (CPU::last_update[CPU::id()] < 2) return;
    
    CPU::last_update[CPU::id()] = 0;

    unsigned long long thread_last_dispatch = time(_statistics.thread_last_dispatch);
    unsigned long long job_release = time(_statistics.job_release);
    unsigned long long deadline = period();
    unsigned long long percentage = 0;
    unsigned long long elapsed_time = thread_last_dispatch - job_release;

    if (elapsed_time && deadline) percentage = (100ULL*elapsed_time) / deadline;

    unsigned long new_freq = calculateFrequency(percentage);
    CPU::clock(new_freq);

    db<CPU>(TRC) << "UPDATE FREQ [" << CPU::id() << "] -> " << new_freq  << "(" << (new_freq * 100ULL) / CPU::max_clock() << "%) | " << percentage << " %" << endl;
}

unsigned long long EDFEnergyAwareness::calculateFrequency(unsigned long long percentage) {
    unsigned long long factor;

    if (percentage <= 10) factor = 0;
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

void EDFEnergyAwarenessAffinity::updateFrequency() {
    // Botar lista que faz o dispatch a cada X interacoes
    int iterations = 3;
    Tick current_time = elapsed();
    Tick total_time = 0;
    bool is_deadline_lost = false;
    unsigned int nqueue = CPU::id();

    // Omitindo a thread que está executando
    for(auto it = Thread::get_scheduler().begin(nqueue); it != Thread::get_scheduler().end() && iterations; ++it, --iterations){
        auto current_element = (*it).object();

        if (current_element->criterion() == IDLE || !current_element->criterion().periodic())
            continue;

        Tick remaining_time = current_element->get_remaining_time();
        int current_deadline = int(current_element->criterion());
        total_time += remaining_time;
        unsigned long long total_time_cpu_percentage = CPU::get_cpu_percentage(total_time);

        db<CPU>(DEV) << " Current time: " << current_time << " Total time: " << total_time_cpu_percentage << " Current deadline: " << current_deadline << endl;

        if (current_time + total_time_cpu_percentage > (unsigned long long)current_deadline) {
            db<CPU>(DEV) << "Fudeo" << endl;
            is_deadline_lost = true;
            break;
        }
    }

    long current_step = CPU::get_clock_step();

    db<CPU>(DEV) << "Current step: " << current_step << endl;
    int new_step = -1;

    if (is_deadline_lost) {
        for (long next_step = current_step + 1; next_step < 14; next_step++) {
            total_time = 0;
            for(auto it = Thread::get_scheduler().begin(nqueue); it != Thread::get_scheduler().end() && iterations; ++it, iterations--){
                auto current_element = (*it).object();

                if (current_element->criterion() == IDLE || !current_element->criterion().periodic())
                    continue;

                long diff = next_step - current_step;
                unsigned long long new_execution_time = current_element->criterion().statistics().avg_execution_time * (10000ULL - diff * 625ULL) / 100ULL;

                db<CPU>(DEV) << "New execution time: " << new_execution_time << endl;

                int current_deadline = int(current_element->criterion());
                total_time += new_execution_time;
                unsigned long long total_time_cpu_percentage = CPU::get_cpu_percentage(total_time);

                if (current_time + total_time_cpu_percentage > (unsigned long long)current_deadline) {
                    db<CPU>(DEV) << "Current time: " << current_time << " Total time: " << total_time_cpu_percentage << " Current deadline: " << current_deadline << endl;
                    break;
                }

                if (it + 1 == Thread::get_scheduler().end() || iterations == 1) {
                    new_step = next_step;
                }

            }

            if (new_step != -1) {
                break;
            }
        }
    } else {
        new_step = current_step;
        for (long next_step = current_step - 1; 0 < next_step; next_step--) {
            total_time = 0;
            for(auto it = Thread::get_scheduler().begin(nqueue); it != Thread::get_scheduler().end() && iterations; ++it, iterations--){
                auto current_element = (*it).object();

                if (current_element->criterion() == IDLE || !current_element->criterion().periodic())
                    continue;

                long diff = current_step - next_step;
                unsigned long long new_execution_time = current_element->criterion().statistics().avg_execution_time * (10000ULL + diff * 625ULL) / 100ULL;

                db<CPU>(DEV) << "New execution time: " << new_execution_time << endl;

                int current_deadline = int(current_element->criterion());
                total_time += new_execution_time;
                unsigned long long total_time_cpu_percentage = CPU::get_cpu_percentage(total_time);

                if (current_time + total_time_cpu_percentage > (unsigned long long)current_deadline) {
                    db<CPU>(DEV) << "Current time: " << current_time << " Total time: " << total_time_cpu_percentage << " Current deadline: " << current_deadline << endl;
                    break;
                }

                if (it + 1 == Thread::get_scheduler().end() || iterations == 1) {
                    new_step = next_step;
                }
            }
        }
    }
    db<CPU>(DEV) << "New step: " << new_step << endl;
    if (new_step == -1 || new_step == 13) {  // faz o L
        CPU::clock(CPU::max_clock());
    } else {
        Hertz new_freq = CPU::get_actual_frequency(new_step);
        CPU::clock(new_freq);
    }
}

unsigned long EDFEnergyAwarenessAffinity::define_best_queue(){
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
        db<EDFEnergyAwarenessAffinity>(DEV) << "CPU [" << nqueue << "] = " << avg_queue_thread_time << " / " << Thread::get_scheduler().size(nqueue) << endl;
    }
    db<EDFEnergyAwarenessAffinity>(DEV) << "Chosen CPU [" << smallest_queue << "]" << endl;
    return smallest_queue;
}

EDF::EDF(Microsecond p, Microsecond d, Microsecond c, unsigned int cpu): RT_Common(int(elapsed() + ticks(d)), p, d, c) {}

__END_SYS
