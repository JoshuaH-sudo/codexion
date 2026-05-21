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
# Balanced baseline: minimal burnout ~= 600 ms, using 1000 ms -> expected PASS
./codexion 5 1000 200 200 200 3 0 fifo

# EDF + cooldown: minimal burnout ~= 450 ms, using 750 ms -> expected PASS
./codexion 4 750 150 150 150 5 50 edf

# Single-coder edge case (left dongle == right dongle)
# always burns out because one coder can never acquire two distinct dongles.
./codexion 1 500 200 100 100 2 0 fifo

# Contention-heavy fifo: minimal burnout ~= 750 ms, using 700 ms -> expected BURNOUT
./codexion 5 700 200 200 200 3 50 fifo

# EDF stress case: minimal burnout ~= 750 ms, using 1200 ms -> expected PASS
./codexion 5 1200 200 200 200 10 50 edf

# Fast cycle with low cooldown: minimal burnout ~= 360 ms, using 600 ms -> expected PASS
./codexion 4 600 120 120 120 3 1 fifo
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

Arbitration happens **at the dongle level**. When multiple coders compete for the same dongle, the dongle's own priority queue decides who proceeds next — no global compile-slot gate exists.

Each dongle owns:
- a `t_heap queue` — min-heap of pending requests sorted by the active policy.
- `sched_mutex` / `sched_cond` — protect the queue and coordinate waiters.
- `held` flag — marks the dongle as currently held by a coder.
- `next_seq_no` — per-dongle arrival counter used for FIFO ordering.

Policy is selected once at startup and applies to every dongle's queue:

- **FIFO mode** — requests ordered by `seq_no` (arrival order per dongle).
- **EDF mode** — requests ordered by `deadline_ms = last_compile_time + time_to_burnout`; ties broken by `seq_no`.

A coder gains access to a dongle when all three conditions hold simultaneously:
1. Its job is at the head of the dongle's queue.
2. The dongle is not currently `held`.
3. The dongle's cooldown window has expired.

`dongle_wait_access` uses `pthread_cond_timedwait` to sleep for exactly the remaining cooldown duration, avoiding busy-waiting while still waking up promptly when the cooldown lapses.

### Dongle Locking and Cooldown

Coders lock dongles in deterministic index order (`min(id), max(id)`) to break circular wait.

- Each dongle is protected by its own `sched_mutex` (part of the per-dongle scheduler).
- Cooldown is enforced inside `dongle_wait_access` via `pthread_cond_timedwait` for the exact remaining cooldown; no polling is used.
- A single-coder case (`left == right`) is handled by the coder spinning on `coder_should_stop` and never starting a compile cycle.
- Since compile requires two distinct dongles, a single coder can never start compile and will always burn out after `time_to_burnout`.

### Burnout Detection

A dedicated monitor thread polls every 1 ms and ends the simulation when either condition is true:

- any coder exceeds `time_to_burnout`
- all coders reached `number_of_compiles_required`

The stop flag is protected by `state_mutex`, and all per-dongle waiters are released by broadcasting on every dongle's `sched_cond`.

### Logging

All output is serialized under `log_mutex`. After stop, non-burnout logs are suppressed to keep shutdown output coherent.

---

## Blocking Cases Handled

### Single-Coder Behavior

With `number_of_coders = 1`, there is only one dongle in the system.

- The coder can acquire that single dongle.
- The coder cannot acquire a second distinct dongle required for compile.
- No compile cycle can begin, so `last_compile_time` is never refreshed by a compile start.
- The monitor eventually detects elapsed time >= `time_to_burnout` and logs `burned out`.

This burnout is expected and considered correct behavior for the one-coder topology.

### Deadlock Prevention and Coffman's Conditions

The implementation prevents circular-wait deadlock by enforcing deterministic
dongle lock order (`min(id), max(id)`) for every coder.

- **Mutual exclusion**: each dongle is guarded by a mutex.
- **Hold and wait**: coders can still wait for a second dongle, but ordering
    prevents circular dependency.
- **No preemption**: mutex ownership is released explicitly after compile.
- **Circular wait**: broken by global lock ordering rule.

### Starvation Prevention

- In **FIFO**, scheduler order is by `seq_no` per dongle, so admission is first-come,
    first-served for each contested resource.
- In **EDF**, order is by deadline (`last_compile_time + time_to_burnout`); the coder
    closest to burning out is granted access first, directly preventing the starvation
    that would occur under FIFO when a slow coder holds dongles needed by a nearly-burned-out peer.

### Cooldown Handling

Each dongle stores `last_used_time` (updated by `dongle_release_access`). Before granting access, `dongle_wait_access` computes the remaining cooldown and calls `pthread_cond_timedwait` to sleep for exactly that duration — no polling, no fixed sleep interval. Stop conditions are checked on every wake-up.

### Precise Burnout Detection

The monitor thread checks all coders every 1 ms and compares elapsed time since
`last_compile_time` with `time_to_burnout`. On burnout, it sets the simulation stop
state and logs `burned out`. `context_set_over` then broadcasts on every dongle's
`sched_cond` so all waiting coders wake up and exit.

### Log Serialization

All logging passes through `log_mutex` to prevent interleaving. After stop,
non-burnout messages are filtered so shutdown output remains consistent.

---

## Synchronization Summary

- `state_mutex`: protects `simulation_over`, `last_compile_time`, and compile counters.
- `dongle[i].sched_mutex` + `dongle[i].sched_cond`: protects each dongle's scheduler queue, `held` flag, and cooldown timestamp; coordinates per-dongle turn-taking.
- `log_mutex`: prevents interleaved log lines.

---

## Thread Synchronization Mechanisms

### `pthread_mutex_t`

The project uses multiple mutexes for disjoint shared-state domains:

- `state_mutex` protects `simulation_over`, compile counters, and
    `last_compile_time` reads/writes shared between coder threads and monitor.
- per-dongle `sched_mutex` protects the dongle's priority queue, `held` flag,
    `next_seq_no`, and `last_used_time`. This unifies what was previously separate
    scheduler and dongle mutexes into a single per-resource lock.
- `log_mutex` serializes all output.

Race prevention example: monitor reads `last_compile_time` under `state_mutex`,
and coders write it under the same mutex (`mark_compile_start`).

### `pthread_cond_t`

Each dongle has its own `sched_cond` for per-resource coordination:

- coders enqueue a scheduler job and wait while not at heap top, the dongle is held,
    or the cooldown has not yet elapsed,
- `dongle_release_access` broadcasts on `sched_cond` to wake all waiters for that dongle,
- `pthread_cond_timedwait` is used during cooldown waits so threads sleep for exactly
    the remaining cooldown, then re-evaluate,
- stop path (`context_set_over`) broadcasts on every dongle's `sched_cond` to unblock all
    waiting coders simultaneously.

This eliminates busy-waiting on both turn-taking and cooldown waits.

### Custom Event Implementation

The combination of `simulation_over` (shared stop flag) and per-dongle `sched_cond`
broadcasts functions as a custom event system:

- **Event source**: monitor detects burnout or all-coders-done.
- **Event publication**: `context_set_over()` sets stop flag and broadcasts on every
    dongle's `sched_cond`.
- **Event consumers**: coder wait loops in `dongle_wait_access` exit safely when stop is
    observed on every wake-up.

This pattern provides deterministic, thread-safe stop propagation to all coder threads
regardless of which dongle they are currently waiting on.

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
