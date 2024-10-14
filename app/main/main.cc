#include <process.h>
#include <real-time.h>
#include <synchronizer.h>
#include <machine.h>
#include <time.h>

using namespace EPOS;
OStream cout;

void foo1() {
    for (int i = 0; i <= 1e8; i++) {
        Math::sqrt(i); Math::gcd(i, 193);
        // if (!(i % 10000000))
        //     cout << "i: " << i << endl;
    }
}

void foo2() {
    Delay d(5e3);
    // cout << "A thread 2 está executando, " << endl;
}


void foo3() {
    for (int i = 0; i < 2e7; i++) {
        // cout << "i: " << i << endl;
    }
}


void foo4() {
    for (int i = 0; i < 2e7; i++) {
        Math::fast_log2(i);
        // cout << "log: " << r << endl;
    }
}

void foo5() {
    for (int i = 0; i < 2e7; i++) {
        Math::cos(i);
        // cout << "log: " << r << endl;
    }
}

int foo6() {
    for (int i = 0; i < 1e9; i++) {
        // cout << "fasdfsdf asdf" << endl;
    }
    return 0;
}


int main() {    
    RT_Thread* pt1 = new RT_Thread(&foo1, 10e5);
    RT_Thread* pt2 = new RT_Thread(&foo2, 7e5);
    RT_Thread* pt3 = new RT_Thread(&foo3, 3e5);
    RT_Thread* pt4 = new RT_Thread(&foo4, 2e5);
    RT_Thread* pt5 = new RT_Thread(&foo5, 1e5);
    // Thread* pt6 = new Thread(&foo6);

    Delay d(1e7);

    int temp = pt1->criterion().statistics().jobs_released - pt1->criterion().statistics().jobs_finished;
    cout << "Deadlines perdidas p1: " << temp << endl;
    temp = pt2->criterion().statistics().jobs_released - pt2->criterion().statistics().jobs_finished;
    cout << "Deadlines perdidas p2: " << temp << endl;
    temp = pt3->criterion().statistics().jobs_released - pt3->criterion().statistics().jobs_finished;
    cout << "Deadlines perdidas p3: " << temp << endl;
    temp = pt4->criterion().statistics().jobs_released - pt4->criterion().statistics().jobs_finished;
    cout << "Deadlines perdidas p4: " << temp << endl;
    temp = pt5->criterion().statistics().jobs_released - pt5->criterion().statistics().jobs_finished;
    cout << "Deadlines perdidas p5: " << temp << endl;

    delete pt1;
    delete pt2;
    delete pt3;
    delete pt4;
    delete pt5;

    return 0;
}
