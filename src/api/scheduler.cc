// EPOS CPU Scheduler Component Implementation

#include <process.h>
#include <time.h>

__BEGIN_SYS

inline RT_Common::Tick RT_Common::elapsed() { return Alarm::elapsed(); }

RT_Common::Tick RT_Common::ticks(Microsecond time) {
    return Timer_Common::ticks(time, Alarm::timer()->frequency());
}

Microsecond RT_Common::time(Tick ticks) {
    return Timer_Common::time(ticks, Alarm::timer()->frequency());
}

void RT_Common::handle(Event event) {
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
        _statistics.jobs_released++;
        _priority = elapsed() + _deadline;
        _statistics.job_release = elapsed();
    }
    if(periodic() && (event & JOB_FINISH)) {
        // if (_statistics.jobs_finished < _statistics.jobs_released)
        _statistics.jobs_finished++;
        db<Thread>(TRC) << "WAIT";
    }

    db<Thread>(TRC) << ") => {i=" << _priority << ",p=" << _period << ",d=" << _deadline << ",c=" << _capacity << "}" << endl;
    db<Thread>(TRC) << this << " " << _statistics.jobs_released << " " << _statistics.jobs_finished << " " << _statistics.jobs_released - _statistics.jobs_finished << endl;
}

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
        _statistics.jobs_released++;
        _priority = elapsed() + _deadline;
        _statistics.job_release = elapsed();
    }
    if(periodic() && (event & JOB_FINISH)) {
        // if (_statistics.jobs_finished < _statistics.jobs_released)
        _statistics.jobs_finished++;
        db<Thread>(TRC) << "WAIT";
    }

    db<Thread>(TRC) << ") => {i=" << _priority << ",p=" << _period << ",d=" << _deadline << ",c=" << _capacity << "}" << endl;
    db<Thread>(TRC) << this << " " << _statistics.jobs_released << " " << _statistics.jobs_finished << " " << _statistics.jobs_released - _statistics.jobs_finished << endl;
}

void EDFEnergyAwareness::updateFrequency() {
    unsigned long long job_start = time(_statistics.job_start);
    unsigned long long job_release = time(_statistics.job_release);
    unsigned long long deadline = period();
    double percentage = 0;
    unsigned long long elapsed_time = job_start - job_release;
    
    if (elapsed_time && deadline) percentage = (100.0f*(double)elapsed_time) / (double)deadline;

    unsigned long new_freq = calculateFrequency(percentage);
    CPU::clock(new_freq);
    // db<Thread>(TRC) << "Job start = " << time(_statistics.job_start) << ", Release = " << time(_statistics.job_release) << endl;
    // db<Thread>(TRC) << "Elapsed Time é " << elapsed_time << endl;
    // db<Thread>(TRC) << "Deadline é " << deadline << endl;
    // db<Thread>(TRC) << "Passou " << percentage << "% do começo" << endl;
    // // db<Thread>(TRC) << "Mudando a frequência para 1GHz" << endl;
    // // db<Thread>(TRC) << "Quantidade de threads rodando no momento = " << _thread_count << endl;
    // db<Thread>(TRC) << "Frequência esperada " << new_freq << endl;
    // db<Thread>(TRC) << "Frequência atual    " << CPU::clock() << endl;
    // db<Thread>(TRC) << "Max Frequência " << CPU::max_clock() << endl;
    // db<Thread>(TRC) << "Min Frequência " << CPU::min_clock() << endl << endl;
}

long long EDFEnergyAwareness::calculateFrequency(double percentage) {
    const double exponential_factor = 0.978161032; // Fator exponencial para ajuste da frequência
    long long ret = CPU::min_clock() + (( CPU::max_clock() - CPU::min_clock()) * (1.0 - Math::pow(exponential_factor, percentage)));
    // Define a frequência máxima se exceder 85% da frequência máxima
    if (ret >= 0.85 * CPU::max_clock()) return  CPU::max_clock();
    return ret;
}


template <typename ... Tn>
FCFS::FCFS(int p, Tn & ... an): Priority((p == IDLE) ? IDLE : RT_Common::elapsed()) {}


EDF::EDF(Microsecond p, Microsecond d, Microsecond c): RT_Common(int(elapsed() + ticks(d)), p, d, c) {}

void EDF::handle(Event event) {
    RT_Common::handle(event);

    // Update the priority of the thread at job releases, before _alarm->v(), so it enters the queue in the right order (called from Periodic_Thread::Xxx_Handler)
    if(periodic() && (event & JOB_RELEASE))
        _priority = elapsed() + _deadline;
}


LLF::LLF(Microsecond p, Microsecond d, Microsecond c): RT_Common(int(elapsed() + ticks((d ? d : p) - c)), p, d, c) {}

void LLF::handle(Event event) {
    if(periodic() && ((event & UPDATE) | (event & JOB_RELEASE) | (event & JOB_FINISH))) {
        _priority = elapsed() + _deadline - _capacity + _statistics.job_utilization;
    }
    RT_Common::handle(event);

    // Update the priority of the thread at job releases, before _alarm->v(), so it enters the queue in the right order (called from Periodic_Thread::Xxx_Handler)
//    if((_priority >= PERIODIC) && (_priority < APERIODIC) && ((event & JOB_FINISH) || (event & UPDATE_ALL)))
}

// Since the definition of FCFS above is only known to this unit, forcing its instantiation here so it gets emitted in scheduler.o for subsequent linking with other units is necessary.
template FCFS::FCFS<>(int p);

__END_SYS
