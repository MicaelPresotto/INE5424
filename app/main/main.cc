#include <process.h>
#include <real-time.h>
#include <synchronizer.h>
#include <machine.h>
#include <time.h>

using namespace EPOS;
OStream cout;


void foo1() {
    cout << Thread::self() << endl;
    cout << "Mensagem do foo1" << endl;
}


void foo2() {
    cout << Thread::self() << endl;
    cout << "Mensagem do foo2" << endl;
}


int main() {
    RT_Thread* pt1 = new RT_Thread(&foo1, 10e5);
    RT_Thread* pt2 = new RT_Thread(&foo2, 5e5);
    
    cout << "Deadline pt1: " << pt1->criterion().deadline() << endl;
    cout << "Deadline pt2: " << pt2->criterion().deadline() << endl;
    Delay d(10e6);

    delete pt1;
    delete pt2;
    
    return 0;
}
