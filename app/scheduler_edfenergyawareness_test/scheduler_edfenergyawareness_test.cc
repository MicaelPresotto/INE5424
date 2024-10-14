#include <process.h>
#include <real-time.h>
#include <synchronizer.h>
#include <machine.h>
#include <time.h>

using namespace EPOS;
OStream cout;

// 10000000 + random()
void foo1() {
    for (int i = 0; i <= 4.36e7; i++) {
        Math::sqrt(i); Math::gcd(i, 193);
        if (!(i % 10000000))
            cout << "i = " << i << endl;
        // cout << "A thread 1 está executando, ";
    }
    // cout << (100 * CPU::clock()) << ", " << CPU::max_clock() << ", ";
    // cout << "clock da CPU = " << CPU::clock() << " (" << (100 * CPU::clock()) / CPU::max_clock() << "%), ";
    cout << endl;
}



int main() {
    cout << "Main " << Thread::self() << endl;
    
    RT_Thread* pt1 = new RT_Thread(&foo1, 10e5);
    // RT_Thread* pt2 = new RT_Thread(&foo2, 5e5);

    Delay d(1e7);

    int temp = pt1->criterion().statistics().jobs_released - pt1->criterion().statistics().jobs_finished;
    delete pt1;
    // delete pt2;
    cout << "Deadlines perdidas: " << temp << endl;

    return 0;
}
