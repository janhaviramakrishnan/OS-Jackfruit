# OS Jackfruit Container Runtime

### 1. Team Information

- **Ipsa Mishra** – PES1UG24CS191  
- **Janhavi Ramakrishnan** – PES1UG24CS198  

### 2. Build, Load, and Run Instructions

**Build The Project**
```bash 
make
```

**Load Kernel Module**
```bash
sudo insmod monitor.ko
```

**Start Supervisor**
```bash
sudo ./engine supervisor ./rootfs-base
```

**Prepare Container Root Filesystems**
```bash
cp -a ./rootfs-base ./rootfs-alpha
cp -a ./rootfs-base ./rootfs-beta
```

**Launch Containers**
```bash
sudo ./engine start alpha ./rootfs-alpha /bin/sh --soft-mib 48 --hard-mib 80
sudo ./engine start beta ./rootfs-beta /bin/sh --soft-mib 64 --hard-mib 96
```

**CLI Operations**
```bash
sudo ./engine ps
sudo ./engine logs alpha
```

**Run Workloads**
Memory Test
```bash
./memory_hog
```
Run CPU-intensive workloads:
```bash
yes > /dev/null &
yes > /dev/null &
```
Change priority of one process:
```bash
renice +10 <PID>
```
Observe CPU allocation differences using:
```bash
top -o %CPU
```

**Stop Containers**
```bash
sudo ./engine stop alpha
sudo ./engine stop beta
```

**Inspect Kernel Logs**
```bash
dmesg | tail
```

**Cleanup**
```bash
pkill yes
pkill cpu_hog
pkill memory_hog
```

### 3. Demo with Screenshots 

**1. Multi-container supervision**
<img width="940" height="285" alt="image" src="https://github.com/user-attachments/assets/338e8af3-d01b-42bd-b27e-7c5226dcd9b3" />

Two containers (alpha and beta) successfully running under a single supervisor process, demonstrating multi-container supervision.

**2. Metadata Tracking**
<img width="940" height="109" alt="image" src="https://github.com/user-attachments/assets/d0682c22-51cd-4cf1-ad1c-8ee3caefe4cb" />

Metadata tracking showing active containers and their corresponding process IDs using the ps command

**3. Bounded-buffer Logging**
<img width="940" height="254" alt="image" src="https://github.com/user-attachments/assets/faae66a8-ad4c-46e2-bffd-45a08913774b" />

Log file contents captured through a producer-consumer style logging pipeline, demonstrating bounded-buffer logging behaviour

**4. CLI and IPC**
<img width="940" height="429" alt="image" src="https://github.com/user-attachments/assets/ba9e24cb-e08c-40e3-8412-3aad9d00a149" />

A CLI command issued to the container runtime and the supervisor responding, demonstrating inter-process communication between components

**5. Soft-limit Warning**
<img width="940" height="74" alt="image" src="https://github.com/user-attachments/assets/0420fa4a-8943-4b46-9d66-55a3f82c9150" />

Kernel log showing a soft-limit warning when container memory usage exceeds the defined threshold

**6. Hard-limit Enforcement**
<img width="940" height="97" alt="image" src="https://github.com/user-attachments/assets/17df7a3e-c9ab-456b-88f5-cc2e2bc1c2a6" />

Kernel log showing hard-limit enforcement where the container process is terminated after exceeding the maximum memory limit

**7. Scheduling Experiment**
<img width="940" height="588" alt="image" src="https://github.com/user-attachments/assets/91f7dbe8-00c3-4d77-a23c-61b1f3b8605e" />

CPU-bound processes pinned to a single core demonstrate scheduling behavior where processes with lower priority (higher nice value) receive significantly less CPU time compared to higher priority processes

**8. Clean Teardown**
<img width="940" height="588" alt="image" src="https://github.com/user-attachments/assets/87775bef-8d34-46fd-9030-e1f65e2bb3b1" />

Processes are terminated successfully using pkill, and system verification confirms that no residual CPU-intensive processes remain


## 4. Engineering Analysis

**1. Process Isolation and Namespaces**
Containers are implemented as isolated processes. Each container runs with its own root filesystem and execution context, preventing interference with other containers or the host.

This demonstrates how operating systems enforce isolation between processes and protect system resources.

**2. Memory Monitoring and Resource Limits**
A kernel module monitors memory usage (RSS) of container processes using a periodic timer.

Two limits are enforced:
- **Soft Limit**: triggers a warning
- **Hard Limit**: terminates the process

This illustrates kernel-level resource monitoring and the difference between advisory and enforced limits.

**3. Kernel–User Space Interaction (IOCTL)**
Communication between user space and kernel space is implemented using `ioctl`.

The runtime sends container details (PID, limits, ID) to the kernel module, which performs monitoring and enforcement.

This reflects how user programs request services while the kernel controls resources.

**4. Inter-Process Communication and Logging Pipeline**
A bounded-buffer producer–consumer model is used for logging.

- Producers generate logs  
- A consumer writes logs to file  

This demonstrates IPC, synchronization using mutexes, and safe data sharing without race conditions.

**5. CPU Scheduling and Process Priorities**
CPU scheduling is influenced by process priority (nice values).

In the experiment:
- Multiple CPU-bound processes were run
- Priorities were adjusted using `renice`
- CPU allocation differences were observed

Higher priority processes received more CPU time, demonstrating scheduler behavior under contention.

### Summary

This project demonstrates key OS concepts:
- Process isolation
- Resource monitoring and limits
- Kernel–user communication
- Synchronization via IPC
- Priority-based scheduling

### 5. Understand the Boilerplate

The `boilerplate/` folder contains starter files:

| File                   | Purpose                                             |
| ---------------------- | --------------------------------------------------- |
| `engine.c`             | User-space runtime and supervisor skeleton          |
| `monitor.c`            | Kernel module skeleton                              |
| `monitor_ioctl.h`      | Shared ioctl command definitions                    |
| `Makefile`             | Build targets for both user-space and kernel module |
| `cpu_hog.c`            | CPU-bound test workload                             |
| `io_pulse.c`           | I/O-bound test workload                             |
| `memory_hog.c`         | Memory-consuming test workload                      |
| `environment-check.sh` | VM environment preflight check                      |

Use these as your starting point. You are free to restructure the repository however you want — the submission requirements are listed in the project guide.

### 6. Build and Verify

```bash
cd boilerplate
make
```

If this compiles without errors, your environment is ready.

### 7. GitHub Actions Smoke Check

Your fork will inherit a minimal GitHub Actions workflow from this repository.

That workflow only performs CI-safe checks:

- `make -C boilerplate ci`
- user-space binary compilation (`engine`, `memory_hog`, `cpu_hog`, `io_pulse`)
- `./boilerplate/engine` with no arguments must print usage and exit with a non-zero status

The CI-safe build command is:

```bash
make -C boilerplate ci
```

This smoke check does not test kernel-module loading, supervisor runtime behavior, or container execution.

---

## What to Do Next

Read [`project-guide.md`](project-guide.md) end to end. It contains:

- The six implementation tasks (multi-container runtime, CLI, logging, kernel monitor, scheduling experiments, cleanup)
- The engineering analysis you must write
- The exact submission requirements, including what your `README.md` must contain (screenshots, analysis, design decisions)

Your fork's `README.md` should be replaced with your own project documentation as described in the submission package section of the project guide. (As in get rid of all the above content and replace with your README.md)
