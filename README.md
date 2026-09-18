*This project has been created as part of the 42 curriculum by lcristur.*

# 📚 CODEXION

## 📝 Description

**CODEXION** simulates coders working in a shared co-working hub who must share a limited number of USB dongles to compile "quantum code". Each coder cycles endlessly through three phases — compiling, debugging, and refactoring — and needs both a left and a right dongle held simultaneously to compile. The project models a variant of the classic Dining Philosophers problem using POSIX threads, mutexes, and condition variables, with a custom fair scheduler (FIFO or EDF) arbitrating access to dongles, and a strict deadline: any coder who fails to start compiling within `time_to_burnout` milliseconds of their last compile burns out, stopping the simulation. 

## ⚙️ Instructions

### 📥 Installation && Compilation

To use this function in your project, clone the repository and compile it.

* **To compile the functions:**

```bash
make
```

* **To remove object files:**
```bash
make clean
```

* **To remove all generated files (objects and library):**
```bash
make fclean
```

* **To make a full fclean and recompilation :**
```bash
make re
```

### 🚀 Usage

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

All arguments are mandatory, in this exact order. `scheduler` must be exactly `fifo` or `edf`. Example:

    ./codexion 4 800 200 100 100 5 50 edf
 
## 📚 Resources

### References
* **42 Peer-Learning** - Collaborated with colleagues.
* POSIX Threads Programming tutorial (Lawrence Livermore National Laboratory)
* man pages: pthread_create(3), pthread_mutex_lock(3), pthread_cond_wait(3), gettimeofday(2)
* Dijkstra's Dining Philosophers problem, as the conceptual basis for the deadlock/starvation scenario modeled here
* Binary heap / priority queue data structure (standard CS reference, e.g. CLRS)
* Earliest Deadline First (EDF) scheduling, as a real-time scheduling policy

### AI Usage

AI (Claude) was used as a design-discussion and code-review partner throughout this project: to reason through the data structures and thread architecture before writing any code, to work through the deadlock-prevention strategy (consistent lock ordering) with worked examples, and to review code written independently — catching concrete bugs such as an incorrect `'\0'` comparison, a missing return path in a validation function, and Norm violations (function count per file, function line limits). All code was written and understood by the author; no code was generated wholesale by AI and copy-pasted.

## Blocking cases handled

- **Deadlock prevention**: a coder needing both dongles always locks them in a fixed, absolute order — by dongle `id`, lower first — rather than by their relative "left"/"right" label. Since two neighboring coders always agree on this order for the dongle they share, no circular wait (Coffman's fourth condition) can ever form, regardless of scheduling.
- **Starvation prevention**: all pending compile requests go through a single global priority queue (binary heap), ordered strictly by arrival time (`fifo`) or burnout deadline (`edf`). A coder is served according to this order once its dongles are free, guaranteeing liveness for feasible parameters.
- **Cooldown handling**: each dongle records the timestamp of its last release; it is treated as unavailable until `dongle_cooldown` milliseconds have elapsed since that release, checked before granting it again.
- **Precise burnout detection**: a dedicated monitor thread continuously checks every coder's `last_compile_start` against `time_to_burnout` and logs the burnout within the required precision window, then stops the simulation.
- **Log serialization**: a single mutex guards every `printf` call, so state messages from different threads are never interleaved on the same line.

## Thread synchronization mechanisms

- `pthread_mutex_t` protects each dongle's state (`in_use`, `released_at`), each coder's timing state (`last_compile_start`, `compiles_done`), the shared request queue, the log output, and the simulation's stop flag — each guarding exactly the data it's responsible for.
- `pthread_cond_t` on the request queue lets a coder block efficiently (`pthread_cond_wait`) while waiting for its turn and for its dongles to become available, instead of busy-waiting. It is woken with `pthread_cond_broadcast` whenever a dongle is released or the simulation is asked to stop.
- All locks are acquired in a single, consistent global order — the queue lock first, then dongle locks (lower `id` first) — which prevents any possible deadlock between threads holding multiple locks at once.
- Race condition example prevented: two neighboring coders both trying to start compiling at the same instant could, without the id-based lock order, each successfully lock one dongle and deadlock waiting for the other. The ordering rule guarantees one of them always wins both locks first, letting the simulation make progress.
- Thread-safe communication between coders and the monitor: a coder's `last_compile_start` and `compiles_done` are only ever read or written while holding that coder's own mutex, so the monitor thread never observes a value mid-update.

## 🧪 Testing examples

Practical commands mapped to the evaluation's testing tiers, so the same checks can be reproduced live.

### Easy — feasible parameters, nobody should burn out

```bash
./codexion 4 800 200 100 100 5 50 fifo
./codexion 4 800 200 100 100 5 50 edf
./codexion 10 1000 100 100 100 5 60 edf
```

What to check: the program runs to completion and returns; no `"burned out"` line appears; every coder id reaches `number_of_compiles_required` compiles. Quick check:

```bash
./codexion 4 800 200 100 100 5 50 edf | grep -c "burned out"   # expect 0
./codexion 4 800 200 100 100 5 50 edf | grep -c "is compiling" # expect 4 * 5 = 20
```

### Less easy — burnout edge cases

```bash
./codexion 2 300 200 100 100 5 50 fifo   # infeasible: cycle (compile+debug+refactor) exceeds time_to_burnout
./codexion 4 300 200 100 100 5 50 edf    # partial burnout: only the coder(s) that cannot make it in time burn out
```

### Medium

```bash
# cooldown behavior: a dongle must stay unavailable for dongle_cooldown ms after release
./codexion 2 5000 200 100 100 5 500 fifo

# scheduler differences: same parameters, only the policy changes
./codexion 5 1000 200 150 100 8 80 fifo
./codexion 5 1000 200 150 100 8 80 edf

# refactoring timing + log serialization under load
./codexion 50 2000 100 100 100 5 40 edf | wc -l
```
