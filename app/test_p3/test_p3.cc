#include <process.h>
#include <real-time.h>
#include <synchronizer.h>
#include <machine.h>
#include <time.h>

using namespace EPOS;
OStream cout;

void function_sqrt() {
    for (int i = 0; i <= 1e8; i++) {
        auto sqr = Math::sqrt(i);
        auto gcd = Math::gcd(i, 193);
        sqr += gcd;
    }
}

void function_delay() {
    Delay d(5e3);
}


void function_pass() {
    for (int i = 0; i < 2e7; i++) {
        i = i;
    }
}


void function_log() {
    for (int i = 0; i < 2e7; i++) {
        auto r = Math::fast_log2(i);
        r += 1;
    }
}

void function_cos() {
    for (int i = 0; i < 2e7; i++) {
        auto r = Math::cos(i);
        r += 1;
    }
}


int main() {
    RT_Thread* pt1 = new RT_Thread(&function_sqrt, 10e5);
    RT_Thread* pt2 = new RT_Thread(&function_delay, 7e5);
    RT_Thread* pt3 = new RT_Thread(&function_pass, 3e5);
    RT_Thread* pt4 = new RT_Thread(&function_log, 2e5);
    RT_Thread* pt5 = new RT_Thread(&function_cos, 1e5);

    Delay d(1e7);

    cout << "Deadlines perdidas p1: " << pt1->get_deadline_losses() << endl;
    cout << "Deadlines perdidas p2: " << pt2->get_deadline_losses() << endl;
    cout << "Deadlines perdidas p3: " << pt3->get_deadline_losses() << endl;
    cout << "Deadlines perdidas p4: " << pt4->get_deadline_losses() << endl;
    cout << "Deadlines perdidas p5: " << pt5->get_deadline_losses() << endl;

    delete pt1;
    delete pt2;
    delete pt3;
    delete pt4;
    delete pt5;

    return 0;
}
