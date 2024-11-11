#include <machine.h>
#include <time.h>
#include <synchronizer.h>
#include <process.h>

using namespace EPOS;
OStream cout;

void print_line() {
    cout << "---------------------------------------------------------------------" << endl;
}

void test_get_frequency_by_step() {
    unsigned long long factor[] = {1875ULL, 2500ULL, 3125ULL, 3750ULL, 4375ULL, 5000ULL, 5625ULL, 6250ULL, 6875ULL, 7500ULL, 8125ULL, 8750ULL, 10000ULL};
    for (unsigned long long i = 1ULL; i < 14ULL; i++) {
        assert(CPU::get_frequency_by_step(i) == (CPU::max_clock()/10000ULL) * factor[i-1]);
        cout << "STEP " << i << " OK" << endl;
    }
}

void test_get_clock_percentage() {
    int expected[] = {18, 25, 31, 37, 43, 50, 56, 62, 68, 75, 81, 87, 100};
    for (int i = 1; i < 14; i++) {
        CPU::clock(CPU::get_frequency_by_step(i));
        assert(CPU::get_clock_percentage() == expected[i-1]);
        cout << "PERCENTAGE " << expected[i-1] << "% OK" << endl;
    }
}

void test_get_clock_step() {
    for (unsigned long long i = 1ULL; i < 14ULL; i++) {
        CPU::clock(CPU::get_frequency_by_step(i));
        assert(CPU::get_clock_step() == i);
        cout << "STEP " << i << " OK" << endl;
    }
}

void test_get_percentage_by_step() {
    unsigned long long expected[] = {18ULL, 25ULL, 31ULL, 37ULL, 43ULL, 50ULL, 56ULL, 62ULL, 68ULL, 75ULL, 81ULL, 87ULL, 100ULL};
    for (unsigned long long i = 1ULL; i < 14ULL; i++) {
        assert(CPU::get_percentage_by_step(i) == expected[i-1]);
        cout << "STEP " << i << " OK" << endl;
    }
}

void banner() {
    cout << "+-----------------------------------------------+"    << endl;
    cout << "|  _______        _             _____  _  _     |"    << endl;
    cout << "| |__   __|      | |           |  __ \\| || |    |"   << endl;
    cout << "|    | | ___  ___| |_ ___      | |__) | || |_   |"    << endl;
    cout << "|    | |/ _ \\/ __| __/ _ \\     |  ___/|__   _|  |"  << endl;
    cout << "|    | |  __/\\__ \\ ||  __/     | |       | |    |"  << endl;
    cout << "|    |_|\\___||___/\\__\\___|     |_|       |_|    |" << endl;
    cout << "+-----------------------------------------------+"    << endl;
}

int main() {

    banner();

    test_get_frequency_by_step(); print_line();
    test_get_clock_percentage(); print_line();
    test_get_clock_step(); print_line();
    test_get_percentage_by_step();

    return 0;
}
