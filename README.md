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

Modern operating systems provide isolation between processes using mechanisms such as namespaces. In this project, each container is created as a separate process with isolated execution context.

The container runtime uses process-level isolation so that each container:
- Has its own root filesystem (via `chroot` or equivalent)
- Runs independently of other containers
- Does not interfere with host processes

This demonstrates how operating systems enforce isolation boundaries, ensuring that processes cannot access each other's resources without explicit permission.

**2. Memory Monitoring and Resource Limits**

The Linux kernel provides mechanisms to observe and control resource usage of processes. In this project, a custom kernel module periodically monitors memory usage (RSS) of registered container processes.

Two types of limits are enforced:
- **Soft Limit**: When exceeded, a warning is logged
- **Hard Limit**: When exceeded, the process is terminated

This demonstrates:
- Kernel-level visibility into process memory usage
- Enforcement of resource constraints
- The distinction between advisory (soft) and enforced (hard) limits

The use of a timer inside the kernel module shows how periodic monitoring is implemented in operating systems.

**3. Kernel–User Space Interaction (IOCTL)**

The project uses an `ioctl` interface for communication between user-space (engine CLI) and kernel-space (monitor module).

This interaction allows:
- Registering container processes for monitoring
- Passing structured data (PID, limits, container ID)
- Triggering kernel-side actions

This reflects a common OS design pattern where:
- User space requests services
- Kernel space enforces policies and accesses hardware-level data

**4. Inter-Process Communication and Logging Pipeline**

The system implements a bounded-buffer logging mechanism using producer–consumer synchronization.
- **Producers**: Container processes generating logs
- **Consumer**: Logging thread writing logs to file

This demonstrates:
- Inter-process communication (IPC)
- Synchronization using mutexes and condition variables
- Prevention of race conditions and buffer overflow

The bounded buffer ensures controlled data flow and models real-world logging systems used in distributed environments.

**5. CPU Scheduling and Process Priorities**
The Linux scheduler allocates CPU time among competing processes based on priority (nice values).

In the scheduling experiment:
- Multiple CPU-bound processes were executed
- Their priorities were modified using `renice`
- CPU usage differences were observed using `top`

Processes with:
- Lower nice value → higher priority → more CPU time  
- Higher nice value → lower priority → less CPU time  

This demonstrates:
- Fair scheduling policies
- Priority-based CPU allocation
- How the scheduler balances workload across processes

Pinning processes to a single CPU core further highlights contention and scheduling decisions.

### Summary

This project integrates multiple core operating system concepts:
- Process isolation through containerization
- Kernel-level resource monitoring and enforcement
- User–kernel communication via ioctl
- Synchronization using producer–consumer patterns
- Scheduling behavior influenced by process priority

Together, these components illustrate how modern operating systems manage processes, resources, and system stability.

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
