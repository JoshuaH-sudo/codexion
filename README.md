*This project has been created as part of the 42 curriculum by jhoban.*

# Codexion

> Master the race for resources before the deadline masters you

---

## Description

**Codexion** is a concurrency simulation written in C, modelled on the classic Dining Philosophers problem. In this variant, coders sit in a circular co-working hub around a shared Quantum Compiler. Each coder must repeatedly **compile**, **debug**, and **refactor**. Compiling requires holding two USB dongles simultaneously (one from each side). Since dongles are shared between adjacent coders, acquiring them without causing deadlocks, starvation, or burnout is the core challenge.

**Goals:**
- Implement POSIX thread-based concurrency (`pthread_create`, `pthread_mutex_t`, `pthread_cond_t`).
- Prevent deadlocks, starvation, and race conditions.
- Enforce dongle cooldown periods after each release.
- Support two scheduling policies: **FIFO** (arrival order) and **EDF** (Earliest Deadline First).
- Detect and report coder burnout within 10 ms of the actual event.
- Stop cleanly when all coders reach the required compile count or when a burnout occurs.

**Key concepts:** thread lifecycle management, mutex-protected shared state, condition variable wait queues, priority queues (min-heap), real-time monitoring, log serialization.

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

All arguments are **mandatory**. Invalid inputs (negative numbers, non-integers, unknown scheduler) are rejected.

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
# 5 coders, 800ms burnout, 200ms compile, 200ms debug, 200ms refactor
# 3 compiles required, 0ms cooldown, FIFO scheduling
./codexion 5 800 200 200 200 3 0 fifo

# 4 coders, EDF scheduling with 50ms cooldown
./codexion 4 600 150 150 150 5 50 edf

# 1 coder (only one dongle on the table)
./codexion 1 500 200 100 100 2 0 fifo
```

### Expected Output Format

```
0 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
201 1 is debugging
401 1 is refactoring
402 2 has taken a dongle
403 2 has taken a dongle
403 2 is compiling
603 2 is debugging
803 2 is refactoring
1204 3 burned out
```

Each line: `timestamp_in_ms  coder_number  state_message`

---

## Blocking Cases Handled

### Deadlock Prevention (Coffman's Conditions)

The classic deadlock scenario in this problem occurs when every coder holds one dongle and waits for the other — a circular wait. This is broken by requiring a coder to **atomically acquire both dongles** before compiling. The dongle arbitration layer (priority queue + condition variables) ensures a coder only "wins" both dongles at once; it never partially holds one while blocking on the other from a global perspective.

Coffman's four conditions are addressed:
- **Mutual exclusion**: Each dongle is protected by a `pthread_mutex_t`; only one coder holds it at a time. This is required and intentional.
- **Hold and wait**: Eliminated — a coder registers intent to acquire both dongles and waits until both are simultaneously available, rather than grabbing one and then blocking on the second.
- **No preemption**: Dongles are voluntarily released after compiling. Burnout detection by the monitor thread triggers a graceful simulation stop rather than forceful preemption.
- **Circular wait**: Prevented by the global two-dongle atomic grant policy; no partial holds exist in the wait state.

### Starvation Prevention

Under **FIFO** scheduling, requests are granted strictly in arrival order, guaranteeing eventual service for every waiting coder.

Under **EDF** scheduling, the coder with the earliest burnout deadline (`last_compile_start + time_to_burnout`) is prioritized. This is inherently starvation-resistant for feasible parameter sets because a coder approaching burnout gets the highest priority. A tie-breaker (coder ID) ensures a fully deterministic policy when deadlines are equal.

### Dongle Cooldown Handling

After a coder releases a dongle, that dongle is marked unavailable until `dongle_cooldown` milliseconds have elapsed. The dongle's release timestamp is recorded; before granting a dongle to the next waiter, the scheduler checks whether `current_time >= release_time + cooldown`. If not, the condition variable wait uses `pthread_cond_timedwait` with a timeout calculated to the exact moment the cooldown expires.

### Precise Burnout Detection

A dedicated **monitor thread** runs independently of all coder threads. It periodically checks the elapsed time since each coder's last compile start. When `current_time - last_compile_start >= time_to_burnout`, the monitor sets a shared `simulation_over` flag (protected by a mutex), logs the burnout message, and broadcasts to all waiting threads to unblock and exit. The polling interval is kept ≤ 1 ms to ensure the 10 ms reporting requirement is met.

### Log Serialization

All output (`printf`/`write`) is guarded by a dedicated **print mutex**. Before printing any state message, a coder or the monitor acquires `log_mutex`, writes the full line, then releases it. This prevents interleaved output even under heavy concurrency.

---

## Thread Synchronization Mechanisms

### `pthread_mutex_t`

Every dongle has an associated `pthread_mutex_t` (`dongle_mutex`) that serializes access to the dongle's state struct (availability flag, release timestamp, wait queue). A separate `log_mutex` serializes all output. A `sim_mutex` protects the global `simulation_over` flag and compile-count checks.

```c
pthread_mutex_lock(&dongle->mutex);
// inspect / modify dongle state safely
pthread_mutex_unlock(&dongle->mutex);
```

### `pthread_cond_t`

Each dongle has a `pthread_cond_t` (`dongle_cond`) used as a wait queue. When a coder requests a dongle that is unavailable or not next in the priority queue, it calls `pthread_cond_wait` (or `pthread_cond_timedwait` when a cooldown timeout is needed). When a dongle is released, `pthread_cond_broadcast` wakes all waiters; each re-evaluates the scheduling condition.

```c
// Waiting for dongle availability
pthread_mutex_lock(&dongle->mutex);
while (!dongle_available_for(dongle, coder))
    pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &deadline);
// dongle granted
pthread_mutex_unlock(&dongle->mutex);

// Releasing dongle
pthread_mutex_lock(&dongle->mutex);
dongle->in_use = 0;
gettimeofday(&dongle->release_time, NULL);
pthread_cond_broadcast(&dongle->cond);
pthread_mutex_unlock(&dongle->mutex);
```

### Priority Queue (Min-Heap)

Each dongle maintains an internal **min-heap** of pending requests. For FIFO, the heap key is the request arrival timestamp. For EDF, the key is `last_compile_start + time_to_burnout`. The heap is protected by the dongle's mutex. Only the coder at the top of the heap may claim the dongle.

```c
// On request: push to heap (inside dongle mutex)
heap_push(&dongle->queue, (t_request){.coder_id = id, .key = key});

// On wake: check if we are at heap top before claiming
if (heap_top(&dongle->queue).coder_id == coder->id && dongle_cooled_down(dongle))
    break; // granted
```

### Monitor Thread

The monitor thread runs a tight loop checking burnout deadlines across all coders. Communication back to coder threads uses the shared `simulation_over` flag guarded by `sim_mutex`, with a `sim_cond` condition variable that coders can block on during long sleep phases instead of busy-waiting.

```c
// Monitor sets stop flag
pthread_mutex_lock(&sim->mutex);
sim->over = 1;
pthread_cond_broadcast(&sim->cond);
pthread_mutex_unlock(&sim->mutex);

// Coder checks stop flag before each phase
pthread_mutex_lock(&sim->mutex);
if (sim->over) { pthread_mutex_unlock(&sim->mutex); return NULL; }
pthread_mutex_unlock(&sim->mutex);
```

### Race Condition Prevention

- **Double-checked state**: All coder state (compile count, last compile time) is read/written under `sim_mutex`.
- **No TOCTOU on dongle**: The dongle's availability is only acted upon while holding `dongle->mutex`; there is no window between checking and claiming.
- **Atomic compile-count completion**: The monitor checks all coders' compile counts under `sim_mutex` to avoid a race where two coders simultaneously reach the target and both trigger an exit.

---

## Resources

### Concurrency & POSIX Threads
- [POSIX Threads Programming — Blaise Barney, LLNL](https://hpc-tutorials.llnl.gov/posix/)
- [The Linux Programming Interface — Michael Kerrisk, Chapter 30–33](https://man7.org/tlpi/)
- [pthread_cond_timedwait(3) man page](https://man7.org/linux/man-pages/man3/pthread_cond_timedwait.3p.html)
- [Dining Philosophers Problem — Wikipedia](https://en.wikipedia.org/wiki/Dining_philosophers_problem)

### Scheduling Algorithms
- [Earliest Deadline First Scheduling — Wikipedia](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling)
- [Priority Queue / Binary Heap — Wikipedia](https://en.wikipedia.org/wiki/Binary_heap)

### Deadlock Theory
- [Coffman Conditions — Wikipedia](https://en.wikipedia.org/wiki/Deadlock#Coffman_conditions)
- [Operating System Concepts — Silberschatz, Galvin, Gagne (Chapter 7: Deadlocks)](https://www.os-book.com/)

### AI Usage

GitHub Copilot (Claude Sonnet 4.6) was used during this project for the following tasks:

- **README generation**: Drafting the initial structure and content of this README based on the project subject PDF.
- **Conceptual clarification**: Explaining EDF scheduling and priority queue heap invariants in the context of POSIX condition variable wait queues.
- **Debugging guidance**: Helping identify potential TOCTOU races in the dongle acquisition logic and suggesting the "check at heap top under mutex" pattern.
- **Makefile structure**: Reviewing the Makefile rules for correctness with `-pthread` and `-Werror`.

All AI-generated suggestions were reviewed, tested, and understood before inclusion. No code was copied blindly — each piece was verified against the subject requirements and tested manually.
