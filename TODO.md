# Codexion TODO Checklist

## Completed

- [x] Parse and validate all 8 required arguments plus scheduler string (`fifo`/`edf`)
- [x] Initialize core simulation context (`t_context`) and dynamic arrays
- [x] Create one thread per coder and join all threads in main flow
- [x] Map coders to left/right dongles in a circular ring
- [x] Protect each dongle with a mutex
- [x] Serialize logs with `log_mutex` (no interleaved lines)
- [x] Add millisecond timestamps from simulation start using `gettimeofday`
- [x] Enforce lock ordering to prevent circular-wait deadlock
- [x] Implement dongle cooldown wait before lock attempt (`dongle_cooldown`)
- [x] Add lifecycle phases and logs:
- [x] `is compiling with dongles.`
- [x] `is debugging`
- [x] `is refactoring`
- [x] Convert phase waits from ms to `usleep` microseconds (`* 1000`)
- [x] Build and norm targets working (`make re`, `make norm`)
- [x] Default `make run` args include non-zero cooldown (currently `50`)

## Still Needed

- [x] Burnout detection logic (`time_to_burnout`) while simulation is running
- [x] Stop simulation immediately when any coder burns out
- [x] Monitor thread (or equivalent) to detect burnout within required tolerance
- [x] Shared `simulation_over` state protected by mutex
- [x] Ensure coder loop exits on burnout, not only on compile-count completion
- [x] Print burnout event line in required format
- [x] Clarify and fully implement scheduler behavior differences:
- [x] FIFO policy behavior
- [x] EDF policy behavior
- [x] Remove unused scheduler/cooldown condition infrastructure if not used (or wire it fully)
- [x] Validate starvation behavior under stress cases

## Nice-to-Have Validation

- [x] Add deterministic test scenarios for 1 coder, high contention, and cooldown edge cases
- [x] Add a quick runtime check target/script for repeated smoke tests
- [x] Keep `README.md` aligned with real implementation status as features land
