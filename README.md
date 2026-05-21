*This project has been created as part of the 42 curriculum by jhoban.*

# Codexion

> Master the race for resources before the deadline masters you

---

## Description

**Codexion** is a C concurrency simulation inspired by Dining Philosophers. Coders run in parallel and cycle through **compile**, **debug**, and **refactor**. Compiling requires two shared dongles (left and right), so synchronization must prevent deadlocks, races, and starvation while still meeting burnout timing constraints.

**Goals:**
- Implement POSIX thread-based concurrency (`pthread_create`, `pthread_mutex_t`, `pthread_cond_t`).
- Prevent deadlocks, starvation, and race conditions.
- Enforce dongle cooldown periods after each release.
- Support two scheduling policies: **FIFO** (arrival order) and **EDF** (Earliest Deadline First).
- Detect and report coder burnout within 10 ms of the actual event.
- Stop cleanly when all coders reach the required compile count or when a burnout occurs.

**Key concepts:** thread lifecycle management, mutex-protected shared state, condition-variable coordination, min-heap scheduling, real-time monitoring, log serialization.

---

## Instructions

### Compilation

```bash
make
```

This produces the `codexion` binary. Compiled with `-Wall -Wextra -Werror -pthread`.

```bash
make clean    # remove object files
make fclean   # remove objects and binary
make re       # fclean + all
```

### Execution

```
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
            time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

All arguments are **mandatory**. Invalid input (non-integer values, bad scheduler) is rejected.

`make run` uses this default argument set unless overridden:

```bash
5 800 200 200 200 3 0 fifo
```

| Argument | Type | Description |
|---|---|---|
| `number_of_coders` | int ≥ 1 | Number of coders and dongles |
| `time_to_burnout` | ms | Max wait between compile starts before burnout |
| `time_to_compile` | ms | Duration of the compile phase (holds 2 dongles) |
| `time_to_debug` | ms | Duration of the debug phase |
| `time_to_refactor` | ms | Duration of the refactor phase |
| `number_of_compiles_required` | int ≥ 1 | Simulation ends when all coders reach this count |
| `dongle_cooldown` | ms | Cooldown after a dongle is released before it can be reacquired |
| `scheduler` | string | `fifo` or `edf` |

### Usage Examples

```bash
# Balanced baseline: expected to complete without burnout
./codexion 5 800 200 200 200 3 0 fifo

# Same baseline with EDF scheduling and cooldown
./codexion 4 600 150 150 150 5 50 edf

# Single-coder edge case (left dongle == right dongle)
./codexion 1 500 200 100 100 2 0 fifo

# Contention-heavy case: likely burnout
./codexion 5 800 200 200 200 3 50 fifo

# EDF stress case for liveness behavior under feasible params
./codexion 5 1000 200 200 200 10 50 edf

# Fast cycle with minimal cooldown
./codexion 4 700 120 120 120 3 1 fifo
```

### Expected Output Format

```
0 1 has taken a dongle.
1 1 has taken a dongle.
1 1 is compiling with dongles.
201 1 is debugging
401 1 is refactoring
402 2 has taken a dongle.
403 2 has taken a dongle.
403 2 is compiling with dongles.
603 2 is debugging
803 2 is refactoring
1204 3 burned out
```

Each line: `timestamp_in_ms  coder_number  state_message`

---

## Runtime Design

### Scheduler Admission

Before each compile cycle, each coder requests a slot in one centralized scheduler queue.

- **FIFO mode** orders by `seq_no` (arrival order).
- **EDF mode** orders by `deadline_ms = last_compile_time + time_to_burnout`.
- EDF includes an aging safeguard: if queue drift is large enough, older requests are promoted to preserve liveness under feasible parameters.

Only the heap-top coder may proceed to dongle acquisition.

### Dongle Locking and Cooldown

After scheduler admission, the coder locks dongles in deterministic index order (`min(id), max(id)`) to break circular wait.

- Each dongle has its own mutex.
- Cooldown is enforced from `last_used_time` in 1 ms steps until the cooldown window expires.
- A single-coder case (`left == right`) is handled by taking only one dongle.

### Burnout Detection

A dedicated monitor thread polls every 1 ms and ends the simulation when either condition is true:

- any coder exceeds `time_to_burnout`
- all coders reached `number_of_compiles_required`

The stop flag is protected by `state_mutex`, and scheduler waiters are released via `scheduler_cond` broadcast.

### Logging

All output is serialized under `log_mutex`. After stop, non-burnout logs are suppressed to keep shutdown output coherent.

---

## Blocking Cases Handled

### Deadlock Prevention and Coffman's Conditions

The implementation prevents circular-wait deadlock by enforcing deterministic
dongle lock order (`min(id), max(id)`) for every coder.

- **Mutual exclusion**: each dongle is guarded by a mutex.
- **Hold and wait**: coders can still wait for a second dongle, but ordering
    prevents circular dependency.
- **No preemption**: mutex ownership is released explicitly after compile.
- **Circular wait**: broken by global lock ordering rule.

### Starvation Prevention

- In **FIFO**, scheduler order is by `seq_no`, so admission is first-come,
    first-served.
- In **EDF**, order is by deadline (`last_compile_time + time_to_burnout`).
- An EDF aging safeguard in heap comparison biases toward older queued requests
    when queue drift grows, preserving liveness under feasible timing.

### Cooldown Handling

Each dongle stores `last_used_time`. Before locking, coders wait until
`dongle_cooldown` expires. This is implemented as 1 ms cooperative polling,
re-checking stop conditions between waits.

### Precise Burnout Detection

The monitor thread checks all coders every 1 ms and compares elapsed time since
`last_compile_time` with `time_to_burnout`. On burnout, it sets simulation stop
state and logs `burned out`.

### Log Serialization

All logging passes through `log_mutex` to prevent interleaving. After stop,
non-burnout messages are filtered so shutdown output remains consistent.

---

## Synchronization Summary

- `state_mutex`: protects `simulation_over`, `last_compile_time`, and compile counters.
- `scheduler_mutex` + `scheduler_cond`: protects scheduler heap and coordinates turn-taking.
- `dongle[i].mutex`: protects each dongle lock ownership and cooldown timestamp.
- `log_mutex`: prevents interleaved log lines.

---

## Thread Synchronization Mechanisms

### `pthread_mutex_t`

The project uses multiple mutexes for disjoint shared-state domains:

- `state_mutex` protects `simulation_over`, compile counters, and
    `last_compile_time` reads/writes shared between coder threads and monitor.
- `scheduler_mutex` protects heap operations and `next_seq_no` in scheduler
    admission.
- per-dongle mutexes protect dongle ownership and cooldown timestamp updates.
- `log_mutex` serializes all output.

Race prevention example: monitor reads `last_compile_time` under `state_mutex`,
and coders write it under the same mutex (`mark_compile_start`).

### `pthread_cond_t`

`scheduler_cond` provides thread-safe communication for admission ordering:

- coders enqueue a scheduler job and wait while not at heap top,
- the thread that pops top broadcasts to wake others,
- stop path (`context_set_over`) broadcasts to unblock all waiters.

This avoids busy-wait on scheduler turn-taking and ensures prompt wake-up on
global stop.

### Custom Event Implementation

The combination of `simulation_over` (shared stop flag), `scheduler_cond`
broadcasts, and frequent stop checks functions as a custom event system:

- **Event source**: monitor detects burnout or all-coders-done.
- **Event publication**: `context_set_over()` sets stop flag and broadcasts.
- **Event consumers**: coder and scheduler wait loops exit safely when stop is
    observed.

This pattern provides deterministic, thread-safe stop propagation between coder
threads and the monitor.

---

## Resources

### Concurrency & POSIX Threads
- [POSIX Threads Programming — Blaise Barney, LLNL](https://hpc-tutorials.llnl.gov/posix/)
- [The Linux Programming Interface — Michael Kerrisk, Chapter 30–33](https://man7.org/tlpi/)
- [Dining Philosophers Problem — Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)

### Scheduling Algorithms
- [Earliest Deadline First Scheduling — Wikipedia](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)
- [Priority Queue / Binary Heap — Wikipedia](https://en.wikipedia.org/wiki/Binary_heap)

### Deadlock Theory
- [Coffman Conditions — Wikipedia](https://en.wikipedia.org/wiki/Deadlock_(computer_science))
- [Operating System Concepts — Silberschatz, Galvin, Gagne (Chapter 7: Deadlocks)](https://www.os-book.com/)
