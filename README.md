#  Multi-Container Runtime Project

---

## Team Members

| Name | SRN |
|------|-----|
| Ramith M R | PES1UG24CS367 |
| Rakshan R | PES1UG24CS365 |

**Course:** Operating Systems
**Date:** 19th April 2026

---

##  1. Introduction

This project implements a lightweight Linux container runtime in C with:

- A long-running **supervisor** that manages multiple containers
- A **kernel-space memory monitor** (LKM) that enforces soft and hard memory limits
- Multi-container lifecycle management with PID, UTS, and Mount namespace isolation
- Concurrent logging via a bounded-buffer producer-consumer pipeline
- A CLI interface using UNIX domain sockets for control commands
- Scheduling experiments using Linux CFS (Completely Fair Scheduler)

---

## ⚙️ 2. Build, Load, and Run Instructions

### 2.1 Prerequisites

```bash
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r)
```
---

### 2.2 Preflight Check

```bash
chmod +x environment-check.sh
sudo ./environment-check.sh
```
---

### 2.3 Prepare Alpine Root Filesystem

```bash
mkdir rootfs-base
wget https://dl-cdn.alpinelinux.org/alpine/v3.20/releases/x86_64/alpine-minirootfs-3.20.3-x86_64.tar.gz
sudo tar -xzf alpine-minirootfs-3.20.3-x86_64.tar.gz -C rootfs-base
```
---

### 2.4 Build the Project

```bash
make clean
make all
```
---

### 2.5 Compile Workload Binaries

```bash
gcc -O0 -static -o workload_cpu workload_cpu.c
gcc -O0 -static -o workload_io  workload_io.c
gcc -O0 -static -o workload_mem workload_mem.c
```
---

### 2.6 Create Container Root Filesystems

```bash
sudo cp -a ./rootfs-base ./rootfs-alpha
sudo cp -a ./rootfs-base ./rootfs-beta
sudo cp -a ./rootfs-base ./rootfs-mem
sudo cp -a ./rootfs-base ./rootfs-cpu
sudo cp -a ./rootfs-base ./rootfs-io

# Copy workload binaries into their respective containers
sudo cp workload_cpu workload_io workload_mem ./rootfs-alpha/
sudo cp workload_cpu workload_io ./rootfs-beta/
sudo cp workload_cpu ./rootfs-cpu/
sudo cp workload_io  ./rootfs-io/
sudo cp workload_mem ./rootfs-mem/
```
---

### 2.7 Load Kernel Module

```bash
sudo insmod monitor.ko

# Verify it loaded correctly
ls -l /dev/container_monitor
dmesg | tail -5
```
---

### 2.8 Start Supervisor — Terminal 1

```bash
sudo ./engine supervisor ./rootfs-base
```

> Leave this terminal running throughout the entire session.

---

### 2.9 Basic CLI Commands — Terminal 2

**Start two containers:**
```bash
sudo ./engine start demo1 ./rootfs-alpha /bin/sleep 60
sudo ./engine start demo2 ./rootfs-beta  /bin/sleep 60
```

**List all running containers:**
```bash
sudo ./engine ps
```


**View logs of a container:**
```bash
sudo ./engine logs demo1
```

**Stop a container:**
```bash
sudo ./engine stop demo1
sudo ./engine stop demo2
```

---

### 2.10 Memory Limit Demo (Screenshots 5 & 6)

```bash
# Start container with tight memory limits
sudo ./engine start memtest ./rootfs-mem /workload_mem --soft-mib 10 --hard-mib 20

# Watch kernel logs live in Terminal 3
sudo dmesg -w

# After kill event, confirm container state changed
sudo ./engine ps
dmesg | grep -i "kill\|limit\|warn"
```

> **Screenshot 5** — `dmesg` showing soft-limit warning
> **Screenshot 6** — `dmesg` showing hard-limit kill + `engine ps` showing state as `killed`

---

### 2.11 Scheduling Experiment (Screenshot 7)

```bash
# Launch both containers simultaneously
sudo ./engine start cpu-cont ./rootfs-cpu /workload_cpu
sudo ./engine start io-cont  ./rootfs-io  /workload_io

# Watch CPU usage live in Terminal 3
top -d 1
# Press P inside top to sort by CPU%
# workload_cpu → ~99% CPU
# workload_io  → <5%  CPU
```

> **Screenshot 7** — `top` showing both processes with visible CPU% difference

---

### 2.12 Teardown (Screenshot 8)

```bash
# Stop all containers
sudo ./engine stop cpu-cont  2>/dev/null
sudo ./engine stop io-cont   2>/dev/null
sudo ./engine stop memtest   2>/dev/null

# Verify no zombie processes remain
ps aux | grep -E "zombie|defunct|workload"

# Stop supervisor in Terminal 1
# Press Ctrl + C

# Unload kernel module
sudo rmmod monitor

# Confirm clean unload
dmesg | tail -10
```

> **Screenshot 8** — Clean `ps aux`, supervisor exit message, and `dmesg` showing module unloaded

---

## 📸 3. Demo Screenshots

### Screenshot 1: Multi-Container Supervision
<img width="806" height="567" alt="Phase1_multicontainer_screenshot" src="https://github.com/user-attachments/assets/8f90c110-7117-441d-8488-79bde1e65178" />


*Two containers running simultaneously under one supervisor process.*

---

### Screenshot 2: Metadata Tracking
<img width="805" height="252" alt="Phase1_Metadata_tracking" src="https://github.com/user-attachments/assets/98782414-d6af-4142-83d1-56b11327d833" />


*Output of `engine ps` showing container metadata — PID, state.*

---

### Screenshot 3: Bounded-Buffer Logging
<img width="811" height="440" alt="phase1_logging_file" src="https://github.com/user-attachments/assets/46203c81-951d-4fd3-940d-ede927a2b2b9" />


*Log file contents captured through the producer-consumer logging pipeline.*

---

### Screenshot 4: CLI and IPC
<img width="940" height="652" alt="image" src="https://github.com/user-attachments/assets/b331f1de-ca4a-4a3d-802f-2f4cc1d166a2" />

after geting log file off the scoket and closing the container
<img width="675" height="519" alt="image" src="https://github.com/user-attachments/assets/37e6ea32-2805-442f-b248-2fb1a80b9515" />


*CLI command being issued and supervisor responding via UNIX domain socket.*

---

### Screenshot 5: Soft-Limit Warning
<img width="1063" height="321" alt="Phase1_soft_limit" src="https://github.com/user-attachments/assets/ffe13857-ff96-48dc-bcb3-2eb7a32c1867" />

*`dmesg` output showing kernel warning when container exceeds soft memory limit.*

---

### Screenshot 6: Hard-Limit Enforcement
<img width="1516" height="288" alt="phase1_hard_limit" src="https://github.com/user-attachments/assets/d5be98c6-4e9f-4df2-806a-b00103d58a12" />


*`dmesg` output showing container killed after exceeding hard memory limit. `engine ps` shows state as `killed`.*

---

### Screenshot 7: Scheduling Experiment
<img width="860" height="450" alt="phase1_sdcheduling_ss" src="https://github.com/user-attachments/assets/ae2755e2-3649-49f5-8e5c-75332aa0e7fb" />


*`top` showing CPU-bound workload at ~99% CPU and I/O-bound workload at <5% CPU running simultaneously.*

---

### Screenshot 8: Clean Teardown
<img width="1211" height="271" alt="phase_zombies_killed" src="https://github.com/user-attachments/assets/794c9a44-0776-44f8-a3ca-90992840fa65" />


*`ps aux` showing no zombies, supervisor exit, and kernel module unloaded cleanly.*

---

## 🧠 4. Engineering Analysis

### 4.1 Isolation Mechanisms

The core goal of a container is to make a process think it is alone on the machine, even though it shares the same kernel as everything else. Linux namespaces are the kernel feature that makes this possible — each namespace creates a "bubble" where processes have their own isolated view of the system.

**PID Namespace** — Inside the container, the first process believes its PID is `1`, as if it is the only process alive. On the host, it has a completely different PID. This prevents a container from seeing or killing processes outside its bubble.

**UTS Namespace** — Each container gets its own hostname. Container `alpha` can identify itself as `alpha` while container `beta` identifies as `beta`, even though they run on the same physical machine.

**Mount Namespace** — Each container gets its own filesystem view via `chroot`. When the container does `ls /`, it sees only its assigned `rootfs-alpha/` directory. We also mount `/proc` inside each container so tools like `ps` work correctly inside the isolated environment.

**What is still shared:** Our runtime does not isolate the network stack, system clock, or CPU/memory — these are shared with the host. Full isolation (as in Docker) would require network namespaces and cgroups, which adds significant complexity and was outside the scope of this project.

---

### 4.2 Supervisor and Process Lifecycle

In Linux, when a child process exits it does not fully disappear — it becomes a **zombie** and waits for its parent to collect its exit status via `waitpid()`. If the parent exits first or never calls `wait()`, the zombie lingers and leaks system resources indefinitely.

Our supervisor solves this by staying alive as the permanent parent of all containers. The lifecycle works as follows:

1. CLI sends a `start` command over the UNIX socket
2. Supervisor calls `clone()` with namespace flags (`CLONE_NEWPID`, `CLONE_NEWUTS`, `CLONE_NEWNS`) to create a new isolated child process
3. The child sets up namespaces, `chroot`s into its rootfs, mounts `/proc`, and executes the target command
4. When the container exits, the kernel sends `SIGCHLD` to the supervisor
5. The supervisor's signal handler calls `waitpid()`, reaps the zombie, and updates the container's metadata (state → `stopped`, exit code recorded)

We use `clone()` instead of `fork()` because `clone()` accepts namespace flags at process creation time, giving us isolation from the very first instruction of the child.

---

### 4.3 IPC, Threads, and Synchronization

Our project uses two completely separate IPC paths — one for logs, one for control commands.

**Path A — Logging (Pipes + Bounded Buffer):**
Each container's stdout and stderr are connected to the supervisor via a pipe. A producer thread reads from the pipe and writes into a bounded circular buffer. A consumer thread reads from that buffer and writes to a log file. This is the classic producer-consumer problem — both threads accessing shared memory simultaneously creates a race condition. We protect against this using a **mutex** (only one thread touches the buffer at a time) and **condition variables** (producer waits when the buffer is full; consumer waits when it is empty).

**Path B — Control (UNIX Domain Sockets):**
When you type `engine ps` or `engine stop alpha`, a short-lived CLI process connects to the long-running supervisor via a UNIX domain socket (`/tmp/supervisor.sock`). The CLI sends a command string, the supervisor reads it, acts on it, sends a response back, and the CLI exits. UNIX sockets were chosen over FIFOs because they support bidirectional communication and connection-oriented semantics, making request-response patterns cleaner to implement.

---

### 4.4 Memory Management and Enforcement

**What RSS measures:**
RSS (Resident Set Size) is the amount of RAM a process is actively using right now — pages currently loaded in physical memory. It does not count memory that has been swapped to disk or memory that is mapped but not yet accessed (due to Linux's lazy allocation strategy). This makes RSS a reliable real-time indicator of actual memory pressure.

**Why two limits:**

| | Soft Limit | Hard Limit |
|---|---|---|
| Action | Log a warning | Send SIGKILL |
| Purpose | Early alert — "you are using a lot of memory" | Hard enforcement — "you have gone too far" |
| Kernel response | `dmesg` warning printed | Process immediately terminated |

The two-tier design gives operators visibility before enforcement. A legitimate workload with a temporary spike gets a warning; only a workload that keeps growing past the hard limit gets killed.

**Why kernel space:**
A user-space monitor would have to periodically read `/proc/PID/status` to check RSS. This introduces latency — by the time user space checks, the process may have already allocated far too much memory. A misbehaving process also cannot be trusted to report its own usage accurately. Our kernel module (`monitor.ko`) runs inside the kernel and directly inspects process memory structures, sending `SIGKILL` the moment the hard limit is crossed — with no user-space round trip and no way for the container to evade it.

---

### 4.5 Scheduling Behavior (CFS)

Linux uses the **Completely Fair Scheduler (CFS)**. The principle is simple — every process deserves a fair share of CPU time. CFS tracks accumulated CPU time per process and always schedules the process that has used the least so far.

**Experiment results:**

| Container | Workload Type | Completion Time | CPU Usage |
|-----------|--------------|-----------------|-----------|
| cpu-cont | CPU-bound (prime sieve) | ~0.535s | ~99% |
| io-cont | I/O-bound (file read/write) | ~40.281s | <1% |

**Why the difference:**
`workload_cpu` never sleeps — it is always in a runnable state, so CFS gives it CPU continuously and it finishes fast. `workload_io` spends most of its time blocked waiting for disk reads and writes — it voluntarily gives up the CPU every time it issues an I/O call, so CFS cannot schedule it even if it wanted to. This demonstrates a fundamental CFS property: **fairness is about CPU time, not wall clock time**. The I/O-bound process is not being starved — it is simply not ready to run most of the time.

The `--nice` flag in our CLI maps directly to Linux `nice` values, which adjust a process's weight in the CFS scheduler. A lower nice value (higher priority) gives a process a larger time slice; a higher nice value throttles it. This lets our runtime control relative scheduling priorities between containers without reimplementing any scheduler logic.

---

##  5. Design Decisions and Tradeoffs

| Subsystem | Design Choice | Tradeoff | Justification |
|-----------|--------------|----------|---------------|
| Namespace Isolation | PID, UTS, Mount only | No network isolation | Sufficient for OS concept demonstration; network namespaces add complexity with no added learning value here |
| Supervisor Architecture | Single long-running process with detached threads | Single point of failure | Simpler than a multi-process supervisor; adequate for the scale of this project |
| IPC — Control Channel | UNIX domain sockets | Requires socket file cleanup on crash | Bidirectional, connection-oriented; cleaner request-response than FIFOs |
| IPC — Logging | Pipes + bounded buffer | Pipe cleanup required on container exit | Decouples container output rate from log write rate; prevents log loss under bursts |
| Kernel Monitor | Polling every ~1 second | Up to 1s enforcement delay | Simpler than kernel hooks; delay acceptable for memory enforcement use case |

---

##  6. Scheduler Experiment Results

Both containers were launched simultaneously under the same supervisor with default scheduling priority (nice = 0).

| Container | Workload | Time | CPU% |
|-----------|----------|------|------|
| cpu-cont | Prime number sieve (compute only) | 0.535s | ~99% |
| io-cont | File read/write loop (2000 cycles × 1MB) | 40.281s | <1% |

**Analysis:**
The CPU-bound workload completed ~75× faster than the I/O-bound one, not because it received more CPU — CFS was fair — but because the I/O-bound workload spent the vast majority of its time in a blocked (sleeping) state waiting for disk. CFS can only schedule processes that are ready to run. This result confirms that Linux CFS achieves fairness in terms of CPU time distribution, but wall clock completion time is dominated by I/O wait, not scheduling policy.

---

##  7. Conclusion

This project successfully demonstrates core operating system concepts through a working implementation:

- **Process isolation** via Linux namespaces and chroot
- **Memory management** with kernel-enforced soft and hard limits
- **IPC mechanisms** — pipes for logging, UNIX sockets for control
- **CPU scheduling** behavior under Linux CFS with observable results
- **Clean resource management** — no zombies, no leaked file descriptors, no stale kernel state

The two-tier architecture (user-space supervisor + kernel-space monitor) reflects a real pattern used in production container runtimes, where enforcement belongs in the kernel for reliability and performance.
