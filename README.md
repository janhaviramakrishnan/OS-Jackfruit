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
<img width="1280" height="800" alt="ss3_new" src="https://github.com/user-attachments/assets/834367b9-acb2-4fe3-908d-804d3880367b" />

Terminal 1 shows the bounded-buffer pipeline in operation — the logger thread starts, producer threads insert log chunks (buffer count rising), and the consumer thread drains them to persistent log files (count falling back to 0)

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

## 5. Design Decisions and Tradeoffs

**1. Namespace Isolation**

- **Design Choice**: Used process-level isolation with separate root filesystems (`chroot`) for each container  
- **Tradeoff**: Weaker isolation compared to full namespace + cgroup-based containers  
- **Justification**: Simpler to implement while still demonstrating core isolation concepts effectively  

**2. Supervisor Architecture**

- **Design Choice**: Centralized supervisor to manage container lifecycle and coordination  
- **Tradeoff**: Single point of failure  
- **Justification**: Simplifies control, debugging, and communication between components  

**3. IPC and Logging Pipeline**

- **Design Choice**: Bounded-buffer producer–consumer model with mutex-based synchronization  
- **Tradeoff**: Added synchronization overhead  
- **Justification**: Ensures safe and ordered logging without race conditions or data loss  

**4. Kernel Monitor**

- **Design Choice**: Periodic polling of process memory using a kernel timer  
- **Tradeoff**: Slight delay in detecting limit violations compared to event-driven monitoring  
- **Justification**: Easier to implement and sufficient for demonstrating monitoring and enforcement  

**5. Scheduling Experiments**

- **Design Choice**: Used CPU-bound workloads with `nice/renice` to observe scheduling behavior  
- **Tradeoff**: Results depend on system conditions (e.g., number of CPU cores)  
- **Justification**: Provides a simple and clear way to demonstrate priority-based CPU allocation  

### Summary

The design prioritizes simplicity and clarity over full production-level features, enabling effective demonstration of key operating system concepts.

## 6. Scheduler Experiment Results

### Observed Output

<img width="1280" height="800" alt="ss_last" src="https://github.com/user-attachments/assets/709dc98b-454a-4316-ba58-59cfeb9c1578" />

### Measured Data

| PID  | Nice (NI) | CPU Usage (%) |
|------|----------|---------------|
| 2511 | 0        | ~32%          |
| 2509 | 0        | ~32%          |
| 2510 | 0        | ~32%          |
| 2512 | 10       | ~3%           |


### Comparison

- Processes with **NI = 0** receive significantly higher CPU time  
- The process with **NI = 10** receives much less CPU time  


### Analysis

This experiment demonstrates that the Linux scheduler:
- Allocates CPU based on process priority (nice value)  
- Favors higher priority processes under contention  
- Reduces CPU share for lower priority processes  

Since all processes are pinned to the same CPU core, the scheduling effect becomes clearly visible.

### Conclusion

The results confirm that Linux uses priority-based scheduling, where lower-priority processes receive reduced CPU time compared to higher-priority processes.
