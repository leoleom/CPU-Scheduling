# CPU Scheduling Simulator

## Group Members

- **Leona Mae Blancaflor**
- **Chrystie Rae A. Sajorne**

---

## Overview

A comprehensive CPU scheduling simulator built in C that implements and compares multiple CPU scheduling algorithms. The simulator generates Gantt charts, calculates performance metrics, and provides detailed analysis of scheduling behavior.

---

## Compilation Instructions

### Prerequisites
- GCC compiler (C99 standard)
- Make build tool
- POSIX-compliant system

### Building the Project

```bash
cd CPU-Scheduling
make clean    # Remove previous builds
make          # Compile the project
```

The compiled executable `schedsim` will be created in the project root directory.

---

## Usage Instructions

### Basic Syntax
```bash
./schedsim --algorithm=<ALGORITHM> [--input=<FILE> | --processes=<PROCESS_LIST>] [--quantum=<VALUE>] [--compare]
```

### Command-Line Arguments

| Argument | Required | Description |
|----------|----------|-------------|
| `--algorithm=<ALG>` | Yes | Scheduling algorithm: `FCFS`, `SJF`, `STCF`, `RR`, or `MLFQ` |
| `--input=<FILE>` | Conditional | Input file with process definitions (if not using `--processes`) |
| `--processes=<LIST>` | Conditional | Comma-separated process list in format: `PID:AT:BT` (e.g., `A:0:8,B:1:4`) |
| `--quantum=<TIME>` | No | Time quantum for RR/MLFQ algorithms (default: 10) |
| `--compare` | No | Run comparison mode across all algorithms |

### Input File Format

Each line defines a process with three space-separated values:
```
<PROCESS_ID> <ARRIVAL_TIME> <BURST_TIME>
```

**Example file (`tests/quiz4.txt`):**
```
A 0 240
B 10 180
C 20 150
D 25 80
E 30 130
```

---

## Implemented Features & Algorithms

### CPU Scheduling Algorithms

1. **FCFS (First Come First Served)**
   - Non-preemptive scheduling
   - Processes executed in arrival order
   - Fair but may cause long wait times for later processes

2. **SJF (Shortest Job First)**
   - Non-preemptive scheduling
   - Shortest burst time processes scheduled first
   - Minimizes average waiting time
   - Requires knowledge of burst times in advance

3. **STCF (Shortest Time to Completion First)**
   - Preemptive variant of SJF
   - Scheduler checks for shorter processes at each time unit
   - Optimal for minimizing average wait time
   - Also called Shortest Remaining Time (SRT)

4. **RR (Round Robin)**
   - Preemptive scheduling
   - Each process gets a fixed time quantum
   - Processes returning to ready queue when quantum expires
   - Better response time and fairness
   - Configurable quantum (default: 10 time units)

5. **MLFQ (Multi-Level Feedback Queue)**
   - Preemptive scheduling with priority levels
   - Multiple queues with different priorities
   - Processes move between queues based on behavior
   - Balances responsiveness and throughput
   - Configurable quantum per queue level

### Additional Features

- **Gantt Chart Generation**: Visual representation of process execution timeline
- **Performance Metrics**:
  - Waiting Time (WT): Time spent waiting in ready queue
  - Turnaround Time (TAT): Total time from arrival to completion
  - Response Time (RT): Time from arrival to first execution
  - Average metrics across all processes
- **Process Management**: Tracks process state, arrival, and burst times
- **Event-Driven Simulation**: Efficient event scheduling using min heap
- **Comparison Mode**: Run all algorithms on same input for performance comparison
- **Debug Output**: Detailed logging of process loading and scheduling steps

---

## Example Usage Commands

### 1. FCFS Algorithm with Input File
```bash
./schedsim --algorithm=FCFS --input=tests/quiz4.txt
```

**Expected Output Structure:**
```
[DEBUG] Loaded 5 processes from file 'tests/quiz4.txt'
[DEBUG] Process A: AT=0, BT=240
[DEBUG] Process B: AT=10, BT=180
[DEBUG] Process C: AT=20, BT=150
[DEBUG] Process D: AT=25, BT=80
[DEBUG] Process E: AT=30, BT=130
[DEBUG] Scheduler initialized

========== FCFS Scheduling ==========
Gantt Chart:
| A (0-240) | B (240-420) | C (420-570) | D (570-650) | E (650-780) |

Metrics:
Process  Arrival  Burst  Start  End  Waiting  Turnaround
A        0        240    0      240  0        240
B        10       180    240    420  230      410
C        20       150    420    570  400      550
D        25       80     570    650  545      625
E        30       130    650    780  620      750

Average Waiting Time: 359.0
Average Turnaround Time: 515.0
```

### 2. SJF Algorithm
```bash
./schedsim --algorithm=SJF --input=tests/quiz4.txt
```

### 3. Round Robin with Custom Quantum
```bash
./schedsim --algorithm=RR --input=tests/quiz4.txt --quantum=20
```

### 4. MLFQ Algorithm
```bash
./schedsim --algorithm=MLFQ --input=tests/quiz4.txt --quantum=8
```

### 5. Compare All Algorithms
```bash
./schedsim --compare --input=tests/quiz4.txt
```

**Expected Output:**
```
Running comparison across all scheduling algorithms...

Algorithm    Avg Wait Time    Avg Turnaround    Avg Response
FCFS         359.0            515.0             0.0
SJF          290.0            446.0             0.0
STCF         290.0            446.0             0.0
RR           260.0            416.0             40.0
MLFQ         255.0            411.0             35.0
```

### 6. Using Process Command-Line Argument
```bash
./schedsim --algorithm=FCFS --processes=A:0:8,B:1:4,C:2:2
```

---

## Known Limitations & Assumptions

### Assumptions

1. **Deterministic Arrival & Burst Times**: All process arrival times and burst times are known in advance (no dynamic arrival)
2. **Single CPU Core**: Simulates single-processor scheduling; no multi-core support
3. **No I/O Operations**: Processes don't block for I/O; all time is CPU time
4. **No Priority Levels** (except MLFQ): FCFS, SJF, STCF, RR treat all processes equally (MLFQ has feedback priority)
5. **Preemption without Context Switch Cost**: Assumes context switching is instantaneous with zero overhead
6. **Positive Integer Values**: All arrival times, burst times, and quantum values must be positive integers
7. **Unique Process IDs**: Process identifiers should be unique within a workload

### Limitations

1. **Memory Constraints**: Maximum process capacity depends on allocated memory; no dynamic expansion tested at extreme scales
2. **No Process Priority Input**: Cannot specify explicit priority levels from input file (MLFQ priority is dynamic only)
3. **Fixed MLFQ Configuration**: Number of queue levels and queue-specific quanta are hardcoded
4. **No Aging Mechanism**: Processes can starve in lower priority queues (no anti-starvation aging)
5. **Gantt Chart Text Output**: No graphical visualization; only text-based chart output
6. **Limited Error Handling**: Malformed input files may cause undefined behavior rather than graceful error messages
7. **No Preemption Overhead**: Real systems have context switch cost; this simulator assumes zero cost
8. **Input File Size**: No tested upper limit on number of processes; extremely large files not recommended
9. **Floating Point Metrics**: Average calculations use floating-point, potential for precision issues

### Known Issues

- MLFQ behavior may vary based on queue configuration (not fully documented)
- Comparison mode assumes all algorithms complete; may hang on certain edge cases
- Debug output is verbose and cannot be disabled without code modification

---

## Project Structure

```
CPU-Scheduling/
├── src/                    # Implementation files
│   ├── main.c             # Entry point with argument parsing
│   ├── scheduler.c        # Core scheduling engine
│   ├── fcfs.c            # FCFS algorithm implementation
│   ├── sjf.c             # SJF algorithm implementation
│   ├── stcf.c            # STCF algorithm implementation
│   ├── rr.c              # Round Robin algorithm implementation
│   ├── mlfq.c            # MLFQ algorithm implementation
│   ├── process.c         # Process management
│   ├── heap.c            # Min heap data structure
│   ├── gantt.c           # Gantt chart generation
│   ├── metrics.c         # Performance metrics calculation
│   ├── compare.c         # Algorithm comparison mode
│   └── utils.c           # Utility functions
├── include/               # Header files
│   ├── scheduler.h       # Scheduler interface
│   ├── process.h         # Process structures
│   ├── gantt.h           # Gantt chart interface
│   ├── metrics.h         # Metrics calculation interface
│   ├── heap.h            # Heap interface
│   └── ...
├── tests/                 # Test input files
│   ├── quiz4.txt         # Sample workload
│   ├── workload.txt      # Additional test cases
│   └── test_suite.sh     # Test script
├── docs/                 # Documentation
│   └── mlfq_design.md    # MLFQ design documentation
├── Makefile              # Build configuration
└── README.md             # This file
```

---

## Cleaning Up

To remove compiled files and binaries:
```bash
make clean
```

---

## License

This project was developed for CMSC 125 (Operating Systems) coursework.
