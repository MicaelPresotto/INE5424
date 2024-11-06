#include <process.h>
#include <real-time.h>
#include <synchronizer.h>
#include <machine.h>
#include <time.h>

using namespace EPOS;
OStream cout;
Simple_Spin s;

#define ITERATIONS  2
#define N_THREADS   5

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

void function_sqrt() {
    for (int i = 0; i <= 1e7; i++) {
        auto sqr = Math::sqrt(i);
        auto gcd = Math::gcd(i, 193);
        sqr += gcd;
    }
    log_stopped(1);
}

void function_delay() {
    Delay d(5e3);
    log_stopped(2);
}


void function_pass() {
    for (int i = 0; i < 2e7; i++) {
        i = i;
    }
    log_stopped(3);
}


void function_log() {
    for (int i = 0; i < 2e7; i++) {
        auto r = Math::fast_log2(i);
        r += 1;
    }
    log_stopped(4);
}

void function_cos() {
    for (int i = 0; i < 2e7; i++) {
        auto r = Math::cos(i);
        r += 1;
    }
    log_stopped(5);
}

int pt_loop(void(callback)()) {
    do {
        callback();
    } while (Periodic_Thread::wait_next());
    return 0;
}

int main() {

    int i;
    Periodic_Thread* pthreads[N_THREADS];
    
    banner();

    pthreads[0] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &pt_loop, function_sqrt);
    pthreads[1] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &pt_loop, function_delay);
    pthreads[2] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &pt_loop, function_pass);
    pthreads[3] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &pt_loop, function_log);
    pthreads[4] = new Periodic_Thread(RTConf(9e5, Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS), &pt_loop, function_cos);

    for (i = 0; i < N_THREADS; i++) pthreads[i]->join();
    print_line();
    for (i = 0; i < N_THREADS; i++) cout << "|           Deadlines perdidas p" << i+1 << ": " << pthreads[i]->get_deadline_losses() << "            |" << " " << pthreads[i] << endl;
    print_line();
    for (i = 0; i < N_THREADS; i++) delete pthreads[i];

    return 0;
}
