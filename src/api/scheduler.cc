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

// int CPU::last_update[Traits<Machine>::CPUS] = {0};

void EDFEnergyAwarenessAffinity::updateFrequency() {
    // CPU::last_update[CPU::id()]++;
    // db<CPU>(TRC) << "LAST UPDATE [" << CPU::id() << "] = " << CPU::last_update[CPU::id()] << " | THREAD = " << Thread::self() <<  endl;
    // if (CPU::last_update[CPU::id()] < 2) return;
    
    // CPU::last_update[CPU::id()] = 0;

    const int threads_ahead = 3;
    const Tick current_time = elapsed();
    Tick total_time = 0;
    bool is_deadline_lost = false;
    int iterations = threads_ahead;
    // Omitindo a thread que está executando
    for(auto it = Thread::get_scheduler().begin(CPU::id()); it != Thread::get_scheduler().end() && iterations; ++it, --iterations){
        auto current_thread = (*it).object();

        if (current_thread->criterion() == IDLE || !current_thread->criterion().periodic()) continue;

        Tick remaining_time = current_thread->get_remaining_time();
        total_time += remaining_time;

        Tick deadline = int(current_thread->criterion());

        db<CPU>(DEV) << "Remaining time: " << remaining_time / CPU::get_clock_percentage() << " Current time: " << current_time << " Total time: " << total_time / CPU::get_clock_percentage() << " Current deadline: " << deadline << endl;

        if (current_time + (total_time / CPU::get_clock_percentage()) > deadline) {
            db<CPU>(DEV) << "VAI PERDER DEADLINE" << endl;
            is_deadline_lost = true;
            break;
        }
    }

    long current_step = CPU::get_clock_step();
    db<CPU>(DEV) << "Is deadline lost: " << is_deadline_lost << endl;
    db<CPU>(DEV) << "Current step: " << current_step << endl;
    // no test_p3 parece que ele sempre fica com o step em 13 e nunca abaixa, o deadline lost sempre eh falso e ele nunca consegue abaixar um step
    int new_step = -1;

    if (is_deadline_lost) {
        for (long next_step = current_step + 1; next_step < 14; next_step++) {
            total_time = 0;
            for(auto it = Thread::get_scheduler().begin(CPU::id()); it != Thread::get_scheduler().end() && iterations; ++it, iterations--){
                auto current_thread = (*it).object();

                if (current_thread->criterion() == IDLE || !current_thread->criterion().periodic()) continue;

                long diff = next_step - current_step;
                unsigned long long new_execution_time = current_thread->criterion().statistics().avg_execution_time * (10000ULL - diff * 625ULL) / 100ULL;
                total_time += new_execution_time;

                db<CPU>(DEV) << "New execution time: " << new_execution_time << endl;

                int current_deadline = int(current_thread->criterion());

                if (current_time + (total_time / CPU::get_clock_percentage()) > current_deadline) {
                     db<CPU>(DEV) << "Nao vai dar par abaixar nenhum step " << endl;
                    db<CPU>(DEV) << "Current time: " << current_time << " Total time: " << (total_time / CPU::get_clock_percentage()) << " Current deadline: " << current_deadline << endl;
                    // same shit here
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
            for(auto it = Thread::get_scheduler().begin(CPU::id()); it != Thread::get_scheduler().end() && iterations; ++it, iterations--){
                // nao esta sendo feito algumas execucoes a mais atoa com o break ali da linha 158?
                auto current_element = (*it).object();

                if (current_element->criterion() == IDLE || !current_element->criterion().periodic())
                    continue;

                long diff = current_step - next_step;
                unsigned long long new_execution_time = current_element->criterion().statistics().avg_execution_time * (10000ULL + diff * 625ULL) / 100ULL;
                total_time += new_execution_time;

                db<CPU>(DEV) << "New execution time: " << new_execution_time << endl;

                int current_deadline = int(current_element->criterion());

                if (current_time + (total_time / CPU::get_clock_percentage()) > current_deadline) {
                    db<CPU>(DEV) << "Current time: " << current_time << " Total time: " << (total_time / CPU::get_clock_percentage()) << " Current deadline: " << current_deadline << endl;
                    break;
                }

                if (it + 1 == Thread::get_scheduler().end() || iterations == 1) {
                    new_step = next_step;
                }
            }
        }
    }
    db<CPU>(DEV) << "New step: " << new_step << endl;
    if (new_step == -1 || new_step == 13) {
        CPU::clock(CPU::max_clock());
    } else {
        Hertz new_freq = CPU::get_frequency_by_step(new_step);
        CPU::clock(new_freq);
    }
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
