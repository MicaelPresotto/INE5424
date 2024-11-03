#include <process.h>
#include <real-time.h>
#include <synchronizer.h>
#include <machine.h>
#include <time.h>

using namespace EPOS;
OStream cout;
Simple_Spin s;

#define ITERATIONS  500
#define N_THREADS   9

void log_stopped(int t) {
    s.acquire();
    cout << "|        Thread " << t << " has finished executing.       |" << endl;
    s.release();
}

void print_line() {
    cout << "+-----------------------------------------------+" << endl;
}

void banner() {
    print_line();
    cout << "|  _______        _             _____  _  _     |"    << endl;
    cout << "| |__   __|      | |           |  __ \\| || |    |"   << endl;
    cout << "|    | | ___  ___| |_ ___      | |__) | || |_   |"    << endl;
    cout << "|    | |/ _ \\/ __| __/ _ \\     |  ___/|__   _|  |"  << endl;
    cout << "|    | |  __/\\__ \\ ||  __/     | |       | |    |"  << endl;
    cout << "|    |_|\\___||___/\\__\\___|     |_|       |_|    |" << endl;
    print_line();
}

int function_sqrt() {
    for (int i = 0; i <= 1e7; i++) {
        auto sqr = Math::sqrt(i);
        auto gcd = Math::gcd(i, 193);
        sqr += gcd;
    }
    log_stopped(1);
    return 0;
}

int function_delay() {
    Delay d(5e3);
    log_stopped(2);
    return 0;
}

int function_pass() {
    for (int i = 0; i < 2e7; i++) {
        i = i;
    }
    log_stopped(3);
    return 0;
}

int function_log2() {
    for (int i = 0; i < 2e7; i++) {
        auto r = Math::fast_log2(i);
        r += 1;
    }
    log_stopped(4);
    return 0;
}

int function_cos() {
    for (int i = 0; i < 2e7; i++) {
        auto r = Math::cos(i);
        r += 1;
    }
    log_stopped(5);
    return 0;
}

int function_sin() {
    for (int i = 0; i < 2e7; i++) {
        auto r = Math::sin(i);
        r += 1;
    }
    log_stopped(6);
    return 0;
}

int function_pow() {
    for (int i = 0; i < 2e7; i++) {
        auto r = Math::pow(i, 2);
        r += 1;
    }
    log_stopped(7);
    return 0;
}

int function_bab_sqrt() {
    for (int i = 0; i < 2e7; i++) {
        auto r = Math::babylonian_sqrt(i);
        r += 1;
    }
    log_stopped(8);
    return 0;
}

int function_lcm() {
    for (int i = 0; i < 2e7; i++) {
        auto r = Math::lcm(i, 631);
        r += 1;
    }
    log_stopped(9);
    return 0;
}

int main() {

    int i;
    Periodic_Thread* pthreads[N_THREADS];
    
    banner();

    pthreads[0] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &function_sqrt);
    pthreads[1] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &function_delay);
    pthreads[2] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &function_pass);
    pthreads[3] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &function_log2);
    pthreads[4] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &function_cos);
    pthreads[5] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &function_sin);
    pthreads[6] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &function_pow);
    pthreads[7] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &function_bab_sqrt);
    pthreads[8] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &function_lcm);

    for (i = 0; i < N_THREADS; i++) pthreads[i]->join();
    print_line();
    for (i = 0; i < N_THREADS; i++) cout << "|            Deadlines perdidas " << i+1 << ": " << pthreads[i]->get_deadline_losses() << "            |" << endl;
    print_line();
    for (i = 0; i < N_THREADS; i++) delete pthreads[i];

    return 0;
}
