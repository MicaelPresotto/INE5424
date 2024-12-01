// EPOS IA32 CPU Mediator Initialization

#include <architecture.h>
#include <machine/machine.h>
#include <system.h>

__BEGIN_SYS

void CPU::init()
{
    db<Init, CPU>(TRC) << "CPU::init()" << endl;

    _cpu_clock = System::info()->tm.cpu_clock;
    _cpu_current_clock[CPU::id()] = System::info()->tm.cpu_clock;
    _bus_clock = System::info()->tm.bus_clock;

    // Initialize the MMU
    if(CPU::id() == CPU::BSP) {
        if(Traits<MMU>::enabled)
            MMU::init();
        else
            db<Init, MMU>(WRN) << "MMU is disabled!" << endl;
    }

    // Initialize the PMU	
    if(Traits<PMU>::enabled) {
        PMU::init();
        //TODO: Ver se tem alguma forma de pegar esses numeros de eventos sem ser magic numbers, ja tentei PMU::, PMU_Event:: e Intel_Sandy_Bridge_PMU::
        // todos dao fail no assert do config
        PMU::config(4, 24); // L1_INSTRUCTION_CACHE_MISSES
        PMU::config(3, 15); // Branch Mispredictions
        PMU::config(2, 2); // Instructions Retired
        PMU::start(4);
        PMU::start(3);
        PMU::start(2);
        PMU::reset(4);
        PMU::reset(3);
        PMU::reset(2);
    }
        
}

void CPU::smp_barrier_init(unsigned int cores) {
    // Core activation in IA32 is handled by the APIC
    _cores = cores;
    if(multicore)
        APIC::remap();
};

__END_SYS
