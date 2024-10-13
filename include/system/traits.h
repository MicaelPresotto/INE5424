#ifndef __traits_types_h
#define __traits_types_h

__BEGIN_UTIL

// Utilities
template<unsigned int KEY_SIZE> class SWAES;
template<typename T, unsigned int LENGHT> class Array;
class Bitmaps;
class Ciphers;
class CRC;
class Debug;
class ELF;
class Handler;
class Lists;
class Heaps;
class Observers;
class OStream;
class Predictors;
class Queues;
class Random;
class Spin;
class SREC;
class Vectors;
template<typename> class Scheduler;

typedef unsigned long Hertz;
typedef unsigned long PPM; // parts per million
typedef unsigned long long PPB; // parts per billion

__END_UTIL

__BEGIN_SYS

// System parts
class Build;
class Boot;
class Setup;
class Init;
class Utility;

// Architecture Hardware Mediators
class CPU;
class TSC;
class MMU;
class FPU;
class PMU;

// Machine Hardware Mediators
class Machine;
class PCI;
class IC;
class Timer;
class RTC;
class UART;
class SPI;
class RS485;
class USB;
class EEPROM;
class Display;
class Serial_Display;
class Keyboard;
class Serial_Keyboard;
class Scratchpad;
class Watchdog;
class GPIO;
class I2C;
class ADC;
class FPGA;
template<unsigned int KEY_SIZE> class HWAES;
class Ethernet;
class IEEE802_15_4;
class Modem;
template<typename Family> class NIC;
class PCNet32;
class RTL8139;
class C905;
class E100;
class M95;
class GEM;
class IEEE802_15_4_NIC;
class Ethernet_NIC;

// Transducer Mediators (i.e. sensors and actuators)
class Transducers;
class Dummy_Transducer;
class Accelerometer;
class Gyroscope;
class Thermometer;
class Alternate_Thermometer;
class Hygrometer;
class Alternate_Hygrometer;
class CO2_Sensor;
class Pluviometer;
class Pressure_Sensor;
class Keypad;

// API Components
class System;
class Application;

class Thread;
class Active;
class Periodic_Thread;
class RT_Thread;
class Task;
class Priority;
class FCFS;
class RR;
class RM;
class DM;
class LM;
class EDF;
class EDFEnergyAwareness;
class LLF;
class GRR;
class Fixed_CPU;
class CPU_Affinity;
class GEDF;
class PEDF;
class CEDF;
class PRM;
class EA_PEDF;

class Address_Space;
class Segment;

class Synchronizer;
class Mutex;
class Semaphore;
class Condition;

class Time;
class Clock;
class Chronometer;
class Alarm;
class Delay;

template<typename T> class Clerk;
class Monitor;

class Network;
class ELP;
class TSTPOE;
class TSTP;
template<typename NIC, typename Network, unsigned int HTYPE> class ARP;
class IP;
class ICMP;
class UDP;
class TCP;
class DHCP;
class HTTP;
class IPC;
template<typename Channel, bool connectionless = Channel::connectionless> class Link;
template<typename Channel, bool connectionless = Channel::connectionless> class Port;

class SmartData;
template<typename Transducer, typename Network = TSTP> class Responsive_SmartData;
template<typename Transducer, typename Network = TSTP> class Interested_SmartData;

// Framework
class Framework;
template<typename Component> class Handle;
template<typename Component, bool remote> class Stub;
template<typename Component> class Proxy;
template<typename Component> class Adapter;
template<typename Component> class Scenario;
class Agent;

// Aspects
class Aspect;
template<typename Component> class Authenticated;
template<typename Component> class Shared;
template<typename Component> class Remote;

// Configuration Tokens
struct Traits_Tokens
{
    // EPOS software architecture (aka SMOD)
    enum {LIBRARY, BUILTIN, KERNEL};

    // CPU hardware architectures
    enum {AVR8, H8, ARMv4, ARMv7, ARMv8, IA32, X86_64, SPARCv8, PPC32, RV32, RV64};

    // Machines
    enum {eMote1, eMote2, STK500, RCX, Cortex, PC, Leon, Virtex, RISCV};

    // Machine models
    enum {Unique, Legacy_PC, eMote3, LM3S811, Zynq, Realview_PBX, Raspberry_Pi3, FZ3, SiFive_E, SiFive_U};

    // Networking models
    enum {STANDALONE = 1, PAN = 10, LAN = 100, MAN = 1000, WAN = 10000, UNROUTED = 0}; // UNROUTED means all nodes are in a single collision domain, but it doesn't assume anything about the number of nodes

    // Architecture endianness
    enum {LITTLE, BIG};

    // Serial display engines
    enum {UART, USB};

    // Life span multipliers
    enum {FOREVER = 0, SECOND = 1, MINUTE = 60, HOUR = 3600, DAY = 86400, WEEK = 604800, MONTH = 2592000, YEAR = 31536000};

    // IP configuration strategies
    enum {STATIC, MAC, INFO, RARP, DHCP};

    // SmartData predictors
    enum :unsigned char {NONE, LVP, DBP};

    // Priority inversion protocols
    enum {CEILING = NONE + 1, INHERITANCE};

    // Default aspects
    typedef ALIST<> ASPECTS;
};

// Interrupt souces names (for all machines; overridden at Traits<IC>; 0 => not used)
struct Interrupt_Source {
    static const unsigned int IRQ_DDR         = 0;
    static const unsigned int IRQ_DMA0        = 0;
    static const unsigned int IRQ_DMA1        = 0;
    static const unsigned int IRQ_ETH0        = 0;
    static const unsigned int IRQ_ETH1        = 0;
    static const unsigned int IRQ_GPIO0       = 0;
    static const unsigned int IRQ_GPIO1       = 0;
    static const unsigned int IRQ_GPIO2       = 0;
    static const unsigned int IRQ_GPIO3       = 0;
    static const unsigned int IRQ_I2C         = 0;
    static const unsigned int IRQ_L2_CACHE    = 0;
    static const unsigned int IRQ_MSI0        = 0;
    static const unsigned int IRQ_PWM0        = 0;
    static const unsigned int IRQ_PWM1        = 0;
    static const unsigned int IRQ_PWM2        = 0;
    static const unsigned int IRQ_PWM3        = 0;
    static const unsigned int IRQ_QSPI0       = 0;
    static const unsigned int IRQ_QSPI1       = 0;
    static const unsigned int IRQ_QSPI2       = 0;
    static const unsigned int IRQ_QSPI3       = 0;
    static const unsigned int IRQ_RTC         = 0;
    static const unsigned int IRQ_SPI0        = 0;
    static const unsigned int IRQ_SPI1        = 0;
    static const unsigned int IRQ_SPI2        = 0;
    static const unsigned int IRQ_SPI3        = 0;
    static const unsigned int IRQ_UART0       = 0;
    static const unsigned int IRQ_UART1       = 0;
    static const unsigned int IRQ_UART2       = 0;
    static const unsigned int IRQ_UART3       = 0;
    static const unsigned int IRQ_WDOG        = 0;
};

// Monitor events (Transducers)
enum Transducer_Event {
    CPU_TEMPERATURE,
    CPU_VOLTAGE,
};

// Monitor events (System)
enum System_Event {
    ELAPSED_TIME,
    DEADLINE_MISSES,
    CPU_EXECUTION_TIME,
    THREAD_EXECUTION_TIME,
    RUNNING_THREAD,
};

// Monitor events (PMU)
enum PMU_Event {
    CPU_CYCLES,
    UNHALTED_CYCLES,

    INSTRUCTIONS_RETIRED,
    LOAD_INSTRUCTIONS_RETIRED,                      INTEGER_LOAD_INSTRUCTIONS_RETIRED = LOAD_INSTRUCTIONS_RETIRED,
    STORE_INSTRUCTIONS_RETIRED,                     INTEGER_STORE_INSTRUCTIONS_RETIRED = STORE_INSTRUCTIONS_RETIRED,
    INTEGER_ARITHMETIC_INSTRUCTIONS_RETIRED,
    INTEGER_MULTIPLICATION_INSTRUCTIONS_RETIRED,
    INTEGER_DIVISION_INSTRUCTIONS_RETIRED,
    FPU_INSTRUCTIONS_RETIRED,
    SIMD_INSTRUCTIONS_RETIRED,
    ATOMIC_MEMEMORY_INSTRUCTIONS_RETIRED,

    BRANCHES,
    IMMEDIATE_BRANCHES,
    CONDITIONAL_BRANCHES,
    BRANCH_MISPREDICTIONS,
    BRANCH_DIRECTION_MISPREDICTIONS,
    CONDITIONAL_BRANCH_MISPREDICTIONS,

    EXCEPTIONS,
    INTERRUPTS,

    L1_CACHE_HITS,
    L1_CACHE_MISSES,
    L1_DATA_CACHE_MISSES,
    L1_DATA_CACHE_WRITEBACKS,
    L1_INSTRUCTION_CACHE_MISSES,                    INSTRUCTION_CACHE_MISSES = L1_INSTRUCTION_CACHE_MISSES,
    L2_CACHE_HITS,
    L2_CACHE_MISSES,
    L2_DATA_CACHE_MISSES,
    L2_DATA_CACHE_WRITEBACKS,
    L3_CACHE_HITS,
    L3_CACHE_MISSES,                                LAST_LEVEL_CACHE_HITS = L3_CACHE_HITS, LAST_LEVEL_CACHE_MISSES = L3_CACHE_MISSES, CACHE_MISSES = LAST_LEVEL_CACHE_MISSES, MEMORY_ACCESSES = LAST_LEVEL_CACHE_MISSES, DATA_MEMORY_ACCESSES = LAST_LEVEL_CACHE_MISSES,

    INSTRUCTION_MEMORY_ACCESSES,
    UNCACHED_MEMORY_ACCESSES,
    UNALIGNED_MEMORY_ACCESSES,

    BUS_CYCLES,
    BUS_ACCESSES,
    TLB_MISSES,
    DATA_TLB_MISSES = TLB_MISSES,                   MMU_MISSES = TLB_MISSES,
    INSTRUCTION_TLB_MISSES,
    MEMORY_ERRORS,

    STALL_CYCLES,
    STALL_CYCLES_CACHE,                             STALL_CYCLES_INSTRUCTION_CACHE = STALL_CYCLES_CACHE,
    STALL_CYCLES_DATA_CACHE,
    STALL_CYCLES_TLB,
    STALL_CYCLES_MEMORY,
    PIPELINE_SERIALIZATIONS,    // ISB
    BUS_SERIALIZATION,          // DSB = 44

#if defined(__ia32__) || defined(__riscv__) || defined(__cortex_a__)
    ARCHITECTURE_DEPENDENT_EVENT45,
    ARCHITECTURE_DEPENDENT_EVENT46,
    ARCHITECTURE_DEPENDENT_EVENT47,
    ARCHITECTURE_DEPENDENT_EVENT48,
    ARCHITECTURE_DEPENDENT_EVENT49,
    ARCHITECTURE_DEPENDENT_EVENT50,
    ARCHITECTURE_DEPENDENT_EVENT51,
    ARCHITECTURE_DEPENDENT_EVENT52,
    ARCHITECTURE_DEPENDENT_EVENT53,
    ARCHITECTURE_DEPENDENT_EVENT54,
    ARCHITECTURE_DEPENDENT_EVENT55,
#endif
#if defined(__ia32__) || defined(__cortex_a__)
    ARCHITECTURE_DEPENDENT_EVENT56,
    ARCHITECTURE_DEPENDENT_EVENT57,
    ARCHITECTURE_DEPENDENT_EVENT58,
    ARCHITECTURE_DEPENDENT_EVENT59,
    ARCHITECTURE_DEPENDENT_EVENT60,
    ARCHITECTURE_DEPENDENT_EVENT61,
    ARCHITECTURE_DEPENDENT_EVENT62,
    ARCHITECTURE_DEPENDENT_EVENT63,
    ARCHITECTURE_DEPENDENT_EVENT64,
    ARCHITECTURE_DEPENDENT_EVENT65,
    ARCHITECTURE_DEPENDENT_EVENT66,
    ARCHITECTURE_DEPENDENT_EVENT67,
    ARCHITECTURE_DEPENDENT_EVENT68,
    ARCHITECTURE_DEPENDENT_EVENT69,
#endif
#if defined(__ia32__) || defined(__cortex_a9__)
    ARCHITECTURE_DEPENDENT_EVENT70,
    ARCHITECTURE_DEPENDENT_EVENT71,
    ARCHITECTURE_DEPENDENT_EVENT72,
    ARCHITECTURE_DEPENDENT_EVENT73,
    ARCHITECTURE_DEPENDENT_EVENT74,
    ARCHITECTURE_DEPENDENT_EVENT75,
    ARCHITECTURE_DEPENDENT_EVENT76,
    ARCHITECTURE_DEPENDENT_EVENT77,
    ARCHITECTURE_DEPENDENT_EVENT78,
#endif
#if defined(__ia32__)
    ARCHITECTURE_DEPENDENT_EVENT79,
    ARCHITECTURE_DEPENDENT_EVENT80,
    ARCHITECTURE_DEPENDENT_EVENT81,
    ARCHITECTURE_DEPENDENT_EVENT82,
    ARCHITECTURE_DEPENDENT_EVENT83,
    ARCHITECTURE_DEPENDENT_EVENT84,
    ARCHITECTURE_DEPENDENT_EVENT85,
    ARCHITECTURE_DEPENDENT_EVENT86,
    ARCHITECTURE_DEPENDENT_EVENT87,
    ARCHITECTURE_DEPENDENT_EVENT88,
    ARCHITECTURE_DEPENDENT_EVENT89,
    ARCHITECTURE_DEPENDENT_EVENT90,
    ARCHITECTURE_DEPENDENT_EVENT91,
    ARCHITECTURE_DEPENDENT_EVENT92,
    ARCHITECTURE_DEPENDENT_EVENT93,
    ARCHITECTURE_DEPENDENT_EVENT94,
    ARCHITECTURE_DEPENDENT_EVENT95,
    ARCHITECTURE_DEPENDENT_EVENT96,
    ARCHITECTURE_DEPENDENT_EVENT97,
    ARCHITECTURE_DEPENDENT_EVENT98,
    ARCHITECTURE_DEPENDENT_EVENT99,
    ARCHITECTURE_DEPENDENT_EVENT100,
    ARCHITECTURE_DEPENDENT_EVENT101,
    ARCHITECTURE_DEPENDENT_EVENT102,
    ARCHITECTURE_DEPENDENT_EVENT103,
    ARCHITECTURE_DEPENDENT_EVENT104,
    ARCHITECTURE_DEPENDENT_EVENT105,
    ARCHITECTURE_DEPENDENT_EVENT106,
    ARCHITECTURE_DEPENDENT_EVENT107,
    ARCHITECTURE_DEPENDENT_EVENT108,
    ARCHITECTURE_DEPENDENT_EVENT109,
    ARCHITECTURE_DEPENDENT_EVENT110,
    ARCHITECTURE_DEPENDENT_EVENT111,
    ARCHITECTURE_DEPENDENT_EVENT112,
    ARCHITECTURE_DEPENDENT_EVENT113,
    ARCHITECTURE_DEPENDENT_EVENT114,
    ARCHITECTURE_DEPENDENT_EVENT115,
    ARCHITECTURE_DEPENDENT_EVENT116,
    ARCHITECTURE_DEPENDENT_EVENT117,
    ARCHITECTURE_DEPENDENT_EVENT118,
    ARCHITECTURE_DEPENDENT_EVENT119,
    ARCHITECTURE_DEPENDENT_EVENT120,
    ARCHITECTURE_DEPENDENT_EVENT121,
    ARCHITECTURE_DEPENDENT_EVENT122,
    ARCHITECTURE_DEPENDENT_EVENT123,
    ARCHITECTURE_DEPENDENT_EVENT124,
    ARCHITECTURE_DEPENDENT_EVENT125,
    ARCHITECTURE_DEPENDENT_EVENT126,
    ARCHITECTURE_DEPENDENT_EVENT127,
    ARCHITECTURE_DEPENDENT_EVENT128,
    ARCHITECTURE_DEPENDENT_EVENT129,
    ARCHITECTURE_DEPENDENT_EVENT130,
    ARCHITECTURE_DEPENDENT_EVENT131,
    ARCHITECTURE_DEPENDENT_EVENT132,
    ARCHITECTURE_DEPENDENT_EVENT133,
    ARCHITECTURE_DEPENDENT_EVENT134,
    ARCHITECTURE_DEPENDENT_EVENT135,
    ARCHITECTURE_DEPENDENT_EVENT136,
    ARCHITECTURE_DEPENDENT_EVENT137,
    ARCHITECTURE_DEPENDENT_EVENT138,
    ARCHITECTURE_DEPENDENT_EVENT139,
    ARCHITECTURE_DEPENDENT_EVENT140,
    ARCHITECTURE_DEPENDENT_EVENT141,
    ARCHITECTURE_DEPENDENT_EVENT142,
    ARCHITECTURE_DEPENDENT_EVENT143,
    ARCHITECTURE_DEPENDENT_EVENT144,
    ARCHITECTURE_DEPENDENT_EVENT145,
    ARCHITECTURE_DEPENDENT_EVENT146,
    ARCHITECTURE_DEPENDENT_EVENT147,
    ARCHITECTURE_DEPENDENT_EVENT148,
    ARCHITECTURE_DEPENDENT_EVENT149,
    ARCHITECTURE_DEPENDENT_EVENT150,
    ARCHITECTURE_DEPENDENT_EVENT151,
    ARCHITECTURE_DEPENDENT_EVENT152,
    ARCHITECTURE_DEPENDENT_EVENT153,
    ARCHITECTURE_DEPENDENT_EVENT154,
    ARCHITECTURE_DEPENDENT_EVENT155,
    ARCHITECTURE_DEPENDENT_EVENT156,
    ARCHITECTURE_DEPENDENT_EVENT157,
    ARCHITECTURE_DEPENDENT_EVENT158,
    ARCHITECTURE_DEPENDENT_EVENT159,
    ARCHITECTURE_DEPENDENT_EVENT160,
    ARCHITECTURE_DEPENDENT_EVENT161,
    ARCHITECTURE_DEPENDENT_EVENT162,
    ARCHITECTURE_DEPENDENT_EVENT163,
    ARCHITECTURE_DEPENDENT_EVENT164,
    ARCHITECTURE_DEPENDENT_EVENT165,
    ARCHITECTURE_DEPENDENT_EVENT166,
    ARCHITECTURE_DEPENDENT_EVENT167,
    ARCHITECTURE_DEPENDENT_EVENT168,
    ARCHITECTURE_DEPENDENT_EVENT169,
    ARCHITECTURE_DEPENDENT_EVENT170,
    ARCHITECTURE_DEPENDENT_EVENT171,
    ARCHITECTURE_DEPENDENT_EVENT172,
    ARCHITECTURE_DEPENDENT_EVENT173,
    ARCHITECTURE_DEPENDENT_EVENT174,
    ARCHITECTURE_DEPENDENT_EVENT175,
    ARCHITECTURE_DEPENDENT_EVENT176,
    ARCHITECTURE_DEPENDENT_EVENT177,
    ARCHITECTURE_DEPENDENT_EVENT178,
    ARCHITECTURE_DEPENDENT_EVENT179,
    ARCHITECTURE_DEPENDENT_EVENT180,
    ARCHITECTURE_DEPENDENT_EVENT181,
    ARCHITECTURE_DEPENDENT_EVENT182,
    ARCHITECTURE_DEPENDENT_EVENT183,
    ARCHITECTURE_DEPENDENT_EVENT184,
    ARCHITECTURE_DEPENDENT_EVENT185,
    ARCHITECTURE_DEPENDENT_EVENT186,
    ARCHITECTURE_DEPENDENT_EVENT187,
    ARCHITECTURE_DEPENDENT_EVENT188,
    ARCHITECTURE_DEPENDENT_EVENT189,
    ARCHITECTURE_DEPENDENT_EVENT190,
    ARCHITECTURE_DEPENDENT_EVENT191,
    ARCHITECTURE_DEPENDENT_EVENT192,
    ARCHITECTURE_DEPENDENT_EVENT193,
    ARCHITECTURE_DEPENDENT_EVENT194,
    ARCHITECTURE_DEPENDENT_EVENT195,
    ARCHITECTURE_DEPENDENT_EVENT196,
    ARCHITECTURE_DEPENDENT_EVENT197,
    ARCHITECTURE_DEPENDENT_EVENT198,
    ARCHITECTURE_DEPENDENT_EVENT199,
    ARCHITECTURE_DEPENDENT_EVENT200,
    ARCHITECTURE_DEPENDENT_EVENT201,
    ARCHITECTURE_DEPENDENT_EVENT202,
    ARCHITECTURE_DEPENDENT_EVENT203,
    ARCHITECTURE_DEPENDENT_EVENT204,
    ARCHITECTURE_DEPENDENT_EVENT205,
    ARCHITECTURE_DEPENDENT_EVENT206,
    ARCHITECTURE_DEPENDENT_EVENT207,
    ARCHITECTURE_DEPENDENT_EVENT208,
    ARCHITECTURE_DEPENDENT_EVENT209,
    ARCHITECTURE_DEPENDENT_EVENT210,
    ARCHITECTURE_DEPENDENT_EVENT211,
    ARCHITECTURE_DEPENDENT_EVENT212,
    ARCHITECTURE_DEPENDENT_EVENT213,
    ARCHITECTURE_DEPENDENT_EVENT214,
    ARCHITECTURE_DEPENDENT_EVENT215,
    ARCHITECTURE_DEPENDENT_EVENT216,
    ARCHITECTURE_DEPENDENT_EVENT217,
    ARCHITECTURE_DEPENDENT_EVENT218,
    ARCHITECTURE_DEPENDENT_EVENT219,
    ARCHITECTURE_DEPENDENT_EVENT220,
    ARCHITECTURE_DEPENDENT_EVENT221,
    ARCHITECTURE_DEPENDENT_EVENT222,
    ARCHITECTURE_DEPENDENT_EVENT223,
    ARCHITECTURE_DEPENDENT_EVENT224,
    ARCHITECTURE_DEPENDENT_EVENT225,
    ARCHITECTURE_DEPENDENT_EVENT226,
    ARCHITECTURE_DEPENDENT_EVENT227,
    ARCHITECTURE_DEPENDENT_EVENT228,
    ARCHITECTURE_DEPENDENT_EVENT229,
    ARCHITECTURE_DEPENDENT_EVENT230,
    ARCHITECTURE_DEPENDENT_EVENT231,
    ARCHITECTURE_DEPENDENT_EVENT232,
    ARCHITECTURE_DEPENDENT_EVENT233,
    ARCHITECTURE_DEPENDENT_EVENT234,
    ARCHITECTURE_DEPENDENT_EVENT235,
    ARCHITECTURE_DEPENDENT_EVENT236,
    ARCHITECTURE_DEPENDENT_EVENT237,
    ARCHITECTURE_DEPENDENT_EVENT238,
    ARCHITECTURE_DEPENDENT_EVENT239,
#endif
    LAST_EVENT
};

template<typename T>
struct Traits {
    // Traits for components that do not declare any
    static const bool enabled = true;
    static const bool monitored = false;
    static const bool debugged = true;
    static const bool hysterically_debugged = false;

    typedef ALIST<> ASPECTS;
};

__END_SYS

#endif
