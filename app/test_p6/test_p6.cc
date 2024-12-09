#include <process.h>
#include <real-time.h>
#include <synchronizer.h>
#include <machine.h>
#include <time.h>
#include <utility/random.h>

using namespace EPOS;
OStream cout;
Simple_Spin s;

#define ITERATIONS  5
#define N_THREADS   9

void log_stopped() {
    s.acquire();
    cout << Alarm::elapsed() << " " << Thread::self()->get_name() << " has finished executing (" << Thread::self()->statistics().executions + 1 << ")." << endl;
    s.release();
}

void log_started() {
    s.acquire();
    cout << Alarm::elapsed() << " " << Thread::self()->get_name() << " has started executing (" << Thread::self()->statistics().executions + 1 << ")." << endl;
    s.release();
}

void print_line() {
    cout << "---------------------------------------------------------------------" << endl;
}

void banner() {
    cout << "+--------------------------------------------+"     << endl;
    cout << "|  _______        _             _____   __   |"     << endl;
    cout << "| |__   __|      | |           |  __ \\ / /   |"    << endl;
    cout << "|    | | ___  ___| |_ ___      | |__) / /_   |"     << endl;
    cout << "|    | |/ _ \\/ __| __/ _ \\     |  ___/ '_ \\  |"  << endl;
    cout << "|    | |  __/\\__ \\ ||  __/     | |   | (_) | |"   << endl;
    cout << "|    |_|\\___||___/\\__\\___|     |_|    \\___/  |" << endl;
    cout << "+--------------------------------------------+"     << endl;
}

void function_sqrt() {
    for (int i = 0; i <= 10000000; i++) {
        auto sqr = Math::sqrt(i);
        auto gcd = Math::gcd(i, 193);
        sqr += gcd;
    }
}

void function_matrix_multiplication() {
    const int SIZE = 256;
    static int A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE];

    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++) {
            A[i][j] = Random::random() % 100;
            B[i][j] = Random::random() % 100;
        }

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            C[i][j] = 0;
            for (int k = 0; k < SIZE; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void function_cache_miss() {
    const int SIZE = 1024 * 1024;
    static int array[SIZE];

    for (int i = 0; i < SIZE; i++) {
        array[i] = i;
    }

    for (int i = SIZE - 1; i >= 0; i -= 64) {
        array[i] += 1;
    }
}

void function_delay() {
    Delay d(5e3);
}

void function_pass() {
    for (int i = 0; i < 20000000; i++) {
        if (Random::random() % 2 % 2)
            i = i;
    }
}

void function_log2() {
    for (int i = 0; i < 20000000; i++) {
        auto r = Math::fast_log2(i);
        r += 1;
    }
}

void function_cos() {
    for (int i = 0; i < 20000000; i++) {
        auto r = Math::cos(i);
        r += 1;
    }
}

void function_sin() {
    for (int i = 0; i < 20000000; i++) {
        auto r = Math::sin(i);
        r += 1;
    }
}

void function_pow() {
    for (int i = 0; i < 20000000; i++) {
        auto r = Math::pow(i, 2);
        r += 1;
    }
}

void function_bab_sqrt() {
    for (int i = 0; i < 20000000; i++) {
        auto r = Math::babylonian_sqrt(i);
        r += 1;
    }
}

void function_lcm() {
    for (int i = 0; i < 20000000; i++) {
        auto r = Math::lcm(i, 631);
        r += 1;
    }
}

int pt_loop(void(callback)()) {
    do {
        log_started();
        callback();
        log_stopped();
    } while (Periodic_Thread::wait_next());
    return 0;
}

int main() {

    int i;
    Periodic_Thread* pthreads[N_THREADS];
    
    banner();

    int randoms[9] = {0};
    for (int i = 0; i < N_THREADS; i++) randoms[i] = Math::abs((Random::random() % 500000)) + 900000;
    for (int i = 0; i < N_THREADS; i++) cout << " Period "<< i+1 << " = " << randoms[i] << endl;

    print_line();
    
    pthreads[0] = new Periodic_Thread(RTConf(randoms[0], Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS, Periodic_Thread::READY, Traits<Application>::STACK_SIZE, "sqrt"), &pt_loop, function_sqrt);
    pthreads[1] = new Periodic_Thread(RTConf(randoms[1], Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS, Periodic_Thread::READY, Traits<Application>::STACK_SIZE, "cachemiss"), &pt_loop, function_cache_miss);
    pthreads[2] = new Periodic_Thread(RTConf(randoms[2], Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS, Periodic_Thread::READY, Traits<Application>::STACK_SIZE, "matrixmul"), &pt_loop, function_matrix_multiplication);
    pthreads[3] = new Periodic_Thread(RTConf(randoms[3], Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS, Periodic_Thread::READY, Traits<Application>::STACK_SIZE, "log2"), &pt_loop, function_log2);
    pthreads[4] = new Periodic_Thread(RTConf(randoms[4], Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS, Periodic_Thread::READY, Traits<Application>::STACK_SIZE, "cos"), &pt_loop, function_cos);
    pthreads[5] = new Periodic_Thread(RTConf(randoms[5], Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS, Periodic_Thread::READY, Traits<Application>::STACK_SIZE, "sin"), &pt_loop, function_sin);
    pthreads[6] = new Periodic_Thread(RTConf(randoms[6], Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS, Periodic_Thread::READY, Traits<Application>::STACK_SIZE, "pow"), &pt_loop, function_pow);
    pthreads[7] = new Periodic_Thread(RTConf(randoms[7], Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS, Periodic_Thread::READY, Traits<Application>::STACK_SIZE, "bsqrt"), &pt_loop, function_bab_sqrt);
    pthreads[8] = new Periodic_Thread(RTConf(randoms[8], Periodic_Thread::SAME, Periodic_Thread::UNKNOWN, Periodic_Thread::NOW, ITERATIONS, Periodic_Thread::READY, Traits<Application>::STACK_SIZE, "lcm"), &pt_loop, function_lcm);

    for (i = 0; i < N_THREADS; i++) pthreads[i]->join();
    print_line();
    for (i = 0; i < N_THREADS; i++) cout << " PT " << pthreads[i]->get_name() << " [" << pthreads[i]->criterion().queue() << "]" << ": Deadlines perdidas " << pthreads[i]->get_deadline_losses() << "; Execucoes " << pthreads[i]->statistics().executions << "; Tempo medio execucao " << pthreads[i]->statistics().avg_execution_time << endl;
    for (i = 0; i < N_THREADS; i++) delete pthreads[i];

    return 0;
}
