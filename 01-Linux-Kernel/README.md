# 1. Linux Kernel

## 1.1. Introduction to the Linux Kernel

### 1.1.1. History and Origin
The Linux kernel was created in 1991 by Linus Torvalds as a personal project while he was a student at the University of Helsinki. Initially designed as a free alternative to MINIX, it quickly evolved with contributions from developers worldwide. Today, Linux powers millions of devices ranging from smartphones to supercomputers, and is the foundation for Android, Chrome OS, and most cloud infrastructure.

### 1.1.2. Role and Function
The kernel is the core component of a Linux operating system, serving as the interface between hardware and user applications. The kernel's primary responsibilities include:
- Managing hardware resources (CPU, memory, I/O devices)
- Providing a standardized interface for applications to access hardware
- Handling concurrent access to hardware resources
- Implementing security and process isolation

### 1.1.3. Versioning and Development Cycle
Linux follows a time-based release cycle:
- New versions are released approximately every 8-10 weeks
- Version numbering follows the x.y.z format:
  - x: major version (currently 6.x)
  - y: minor version (contains new features)
  - z: patch version (bug fixes only)
- Long Term Support (LTS) versions are maintained for several years with security fixes

## 1.2. Kernel Architecture

### 1.2.1. Monolithic Design
Linux uses a monolithic kernel architecture where the entire kernel runs in a privileged kernel space. Despite its monolithic nature, Linux incorporates modularity through loadable kernel modules. Compared to a microkernel design, this architecture offers:
- Higher performance (direct function calls vs. message passing)
- Simpler development for certain functionality
- Better debugging capabilities for many scenarios

### 1.2.2. Kernel Space vs. User Space
Linux divides memory into two distinct regions:
- **Kernel space**: Where the kernel executes with full privileges, direct hardware access
- **User space**: Where user applications run with limited privileges

User applications interact with the kernel through system calls. This separation ensures system stability and security by preventing applications from directly manipulating hardware or interfering with other processes.

### 1.2.3. Core Subsystems

#### 1.2.3.1. Process Scheduler
The scheduler determines which process runs, when, and for how long. Linux uses the Completely Fair Scheduler (CFS) as its default scheduler, which aims to maximize CPU utilization while ensuring good interactive performance.

Processes have different priority levels:
- Real-time processes (highest priority)
- Normal user processes (can be adjusted with nice values)

#### 1.2.3.2. Memory Management
The memory management subsystem handles:
- Physical and virtual memory allocation
- Memory protection between processes
- Swapping to disk when physical memory is full
- File-backed and anonymous memory mappings

Linux employs mechanisms like the buddy allocator and slab allocator to efficiently manage kernel memory.

#### 1.2.3.3. Virtual File System (VFS)
The Virtual File System provides a common interface to different filesystems, abstracting the details of how various filesystem types store data. VFS allows Linux to support numerous filesystems:
- Native filesystems (ext4, XFS, Btrfs)
- Network filesystems (NFS, SMB)
- Special filesystems (procfs, sysfs, debugfs)
- Foreign filesystems (FAT, NTFS)

#### 1.2.3.4. Network Stack
The Linux networking subsystem implements network protocols and provides APIs for applications:
- Socket interfaces for applications
- Protocol implementations (TCP/IP, UDP, etc.)
- Network device drivers
- Packet filtering and routing capabilities

#### 1.2.3.5. Device Drivers
Drivers allow the kernel to interact with hardware devices, categorized into:
- Block device drivers (for storage devices)
- Character device drivers (for serial ports, keyboards, etc.)
- Network device drivers
- Sound drivers
- Graphics drivers
- Bus drivers (USB, PCI, etc.)

## 1.3. Kernel Source Organization

### 1.3.1. Directory Structure
The kernel source code is organized into directories by function:
- **arch/**: Architecture-specific code (x86, arm, riscv, etc.)
- **drivers/**: Device drivers
- **fs/**: Filesystem implementations
- **include/**: Header files
- **init/**: Kernel initialization code
- **ipc/**: Inter-process communication code
- **kernel/**: Core kernel code
- **lib/**: Library routines
- **mm/**: Memory management code
- **net/**: Networking code
- **sound/**: Sound subsystem

### 1.3.2. Size and Statistics
The Linux kernel is a massive software project:
- Approximately 80,000 files
- Over 35 million lines of code (as of Linux 5.18)
- 1.3GB of source code
- Contributions from thousands of developers worldwide

Code distribution (by percentage of lines):
- drivers/: 61.1%
- arch/: 11.6%
- fs/: 4.4%
- sound/: 4.1%
- net/: 3.7%
- kernel/: 1.3%
- Others: remainder

## 1.4. Kernel Programming

### 1.4.1. Language and Constraints
The Linux kernel is primarily written in C, with some Assembly for architecture-specific parts:
- No standard C library (libc) available
- No C++ usage
- Uses GCC-specific extensions
- Limited floating-point support
- Fixed-size stack (4KB or 8KB)
- No memory protection within the kernel
- Kernel-specific functions replace libc (`printf()` → `printk()`, `malloc()` → `kmalloc()`, etc.)

Recently, Rust support is being developed to improve code safety in the kernel.

### 1.4.2. Memory Allocation
The kernel provides several memory allocation APIs:

1. **Page Allocator**: Allocates memory in page units
   - `get_free_pages()`, `__get_free_pages()`
   - Allocates in powers of 2 pages

2. **Kmalloc**: General-purpose allocator
   - `kmalloc()`, `kzalloc()` (zero-initialized)
   - Guarantees physically contiguous memory
   - Size limitations (typically 4MB per allocation)

3. **Vmalloc**: For larger allocations
   - `vmalloc()`
   - Virtually contiguous but not necessarily physically contiguous
   - Slower than kmalloc but can allocate larger regions

4. **Slab Allocator**: For objects of the same size
   - Optimized for frequent allocation/deallocation
   - Reduces memory fragmentation

5. **Device-managed Allocations**:
   - `devm_kmalloc()`, `devm_kzalloc()`
   - Automatically freed when the driver is unloaded

### 1.4.3. Synchronization and Locking
Linux provides multiple synchronization mechanisms:

1. **Mutex**: For code that can sleep
   - `mutex_lock()`, `mutex_unlock()`
   - The current lock holder blocks if it tries to re-acquire
   - Safe with preemption

2. **Spinlock**: For code that cannot sleep
   - `spin_lock()`, `spin_unlock()`
   - Spins until the lock can be acquired
   - Should be used for short code segments
   - Variants: `spin_lock_irq()`, `spin_lock_irqsave()`

3. **RCU (Read-Copy-Update)**: For read-mostly data
   - No locking for readers
   - Updates by creating a copy and replacing

4. **Atomic Operations**: For simple variables
   - `atomic_inc()`, `atomic_add()`, etc.
   - No explicit locking required

5. **Completion**: For signaling events
   - `wait_for_completion()`, `complete()`

6. **Wait Queues**: For processes waiting for events
   - `wait_event()`, `wake_up()`

### 1.4.4. Error Handling
Error handling in the kernel uses negative error codes:
- `ENOMEM`: Out of memory
- `EINVAL`: Invalid argument
- `EBUSY`: Device or resource busy
- `EIO`: I/O error

Example:
```c
int result = some_function();
if (result < 0)
    return result;  // Return the error code
```

## 1.5. Kernel Modules

### 1.5.1. Module Concept
Modules are pieces of code that can be loaded into and unloaded from the running kernel:
- Allow extending functionality without rebooting
- Help keep the kernel small
- Useful for rapid driver development

### 1.5.2. Module Structure
A basic module includes:
- Initialization function (called when loaded)
- Cleanup function (called when unloaded)
- Metadata (license, author, description)

Simple example:
```c
#include <linux/module.h>
#include <linux/init.h>

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello, world!\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "Goodbye, world!\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World module");
```

### 1.5.3. Compiling Modules
Modules can be compiled:
- Out-of-tree: Separate from the kernel source
- In-tree: Integrated into the kernel build process

Simple Makefile for out-of-tree module:
```makefile
obj-m := mymodule.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
```

### 1.5.4. Module Management
Module management commands:
- `insmod`: Insert a module (requires full path)
- `rmmod`: Remove a module
- `modprobe`: Load a module and its dependencies
- `lsmod`: List loaded modules
- `modinfo`: Display module information

Modules can accept parameters when loaded:
```bash
insmod mymodule.ko param1=value1 param2=value2
```

## 1.6. Device Drivers

### 1.6.1. Driver Concept
Device drivers are code that bridges specific hardware with the rest of the kernel. Each type of hardware requires its own driver for the kernel to communicate with it.

### 1.6.2. Driver Types
Linux categorizes drivers into several types:

1. **Character Device Drivers**: 
   - Handle data as byte streams
   - Examples: serial devices, keyboards, mice

2. **Block Device Drivers**:
   - Handle data in fixed-size blocks
   - Examples: hard drives, SSDs, USB drives

3. **Network Device Drivers**:
   - Manage network communications
   - Examples: Ethernet cards, WiFi adapters

4. **USB Drivers**:
   - Manage USB devices
   - Usually consist of a core driver and device-specific drivers

5. **Platform Drivers**:
   - For integrated devices on SoCs
   - No auto-detection capability

### 1.6.3. Platform Driver Framework
Many embedded devices are not self-discoverable (unlike USB or PCI). Linux uses:
- Platform device: Describes non-discoverable hardware
- Platform driver: Kernel module to control the device
- Matching: Based on name or "compatible" property in Device Tree

Simple platform driver example:
```c
static const struct platform_device_id mydev_ids[] = {
    { "mydevice", 0 },
    { }
};
MODULE_DEVICE_TABLE(platform, mydev_ids);

static struct platform_driver mydev_driver = {
    .probe = mydev_probe,
    .remove = mydev_remove,
    .driver = {
        .name = "mydevice",
        .of_match_table = of_match_ptr(mydev_dt_match),
        .owner = THIS_MODULE,
    },
    .id_table = mydev_ids,
};

module_platform_driver(mydev_driver);
```

## 1.7. Device Tree

### 1.7.1. Introduction
Device Tree (DT) is a data structure that describes hardware to the kernel, separating hardware description from kernel code:
- Essential for platforms without full auto-discovery (many ARM systems)
- Allows a single kernel to run on multiple boards
- Describes devices, addresses, interrupts, clocks, etc.

### 1.7.2. Syntax
Device Trees are written as Device Tree Source (.dts) and compiled to Device Tree Blob (.dtb):

```dts
/* Simple Device Tree example */
/dts-v1/;
/ {
    model = "Vendor Board Model X";
    compatible = "vendor,board-model-x";
    
    cpus {
        cpu@0 {
            compatible = "arm,cortex-a53";
            reg = <0>;
        };
    };
    
    soc {
        uart@10000000 {
            compatible = "vendor,uart";
            reg = <0x10000000 0x1000>;
            interrupts = <0 20 4>;
            status = "okay";
        };
        
        i2c@10001000 {
            compatible = "vendor,i2c";
            reg = <0x10001000 0x1000>;
            clock-frequency = <400000>;
            status = "okay";
            
            eeprom@50 {
                compatible = "at24,24c256";
                reg = <0x50>;
            };
        };
    };
};
```

### 1.7.3. Key Properties
- **compatible**: Indicates driver compatibility
- **reg**: Address range of device registers
- **interrupts**: Interrupt specifications
- **status**: Enable/disable device ("okay", "disabled")
- **#address-cells**, **#size-cells**: Define address field interpretation

### 1.7.4. Inheritance and Overlays
Device Tree supports inheritance and overlays:
- .dtsi files can be included by multiple .dts files
- Device Tree overlays allow modifying parts of the tree without recompiling the whole

## 1.8. Kernel Boot Process

### 1.8.1. Boot Sequence
The Linux boot process:
1. Bootloader loads kernel into memory
2. Kernel initializes core hardware components
3. Kernel decompresses and initializes initramfs (if present)
4. Kernel mounts the root filesystem
5. Kernel starts the init process (PID 1)
6. Init starts system services

### 1.8.2. Kernel Command Line
The kernel receives parameters via command line from the bootloader:
- `root=`: Specifies root filesystem
- `console=`: Console device
- `init=`: Init program
- `mem=`: Limits RAM usage

Example:
```
console=ttyS0,115200 root=/dev/mmcblk0p2 rootwait
```

### 1.8.3. initramfs
Initial RAM Filesystem (initramfs) is a simple filesystem loaded into RAM:
- Contains modules and tools needed to mount the real root filesystem
- Useful when the kernel needs additional modules to access the root filesystem
- Unpacked and used before mounting the root filesystem

## 1.9. System Calls

### 1.9.1. System Call Mechanism
System calls are the interface between user applications and the kernel:
- Allow applications to request services from the kernel
- Involve a transition from user mode to kernel mode
- Around 400 system calls in modern Linux

Applications typically don't call system calls directly but through the C library (libc).

### 1.9.2. System Call Implementation
System call implementation includes:
- System call table defining mapping from system call numbers to handler functions
- Trap/exception mechanism to switch from user to kernel mode
- Core code ensuring safety and parameter checking

Example of a simple system call implementation:
```c
SYSCALL_DEFINE3(read, unsigned int, fd, char __user *, buf, size_t, count)
{
    struct fd f = fdget_pos(fd);
    ssize_t ret = -EBADF;

    if (f.file) {
        ret = vfs_read(f.file, buf, count, &f.file->f_pos);
        fdput_pos(f);
    }
    return ret;
}
```

### 1.9.3. User/Kernel Communication
When exchanging data between user space and kernel space:
- Never directly use user pointers in kernel code
- Use safe functions like `copy_from_user()` and `copy_to_user()`
- Check return values of these functions to handle errors

## 1.10. Virtual Filesystems and Special Devices

### 1.10.1. Virtual Filesystems
Linux provides several virtual filesystems to access kernel information:

1. **procfs** (/proc):
   - Information about processes and system parameters
   - Examples: `/proc/cpuinfo`, `/proc/meminfo`, `/proc/<pid>/`

2. **sysfs** (/sys):
   - Representation of the device tree structure
   - Access to device attributes and configuration
   - Interface to the kernel's device model

3. **debugfs** (/sys/kernel/debug):
   - Debugging information for kernel modules and drivers
   - Not recommended for production systems

### 1.10.2. Special Devices
Linux provides special devices in /dev:
- `/dev/null`: Discards all data written to it
- `/dev/zero`: Provides an endless stream of zero bytes
- `/dev/random`, `/dev/urandom`: Random numbers
- `/dev/mem`: Access to physical memory (restricted)
- `/dev/kmsg`: Kernel message buffer

## 1.11. Interrupts and Bottom-Half Processing

### 1.11.1. Interrupt Mechanism
Interrupts allow devices to signal the CPU when they need attention:
- Hardware IRQs: Signals from hardware devices
- Software IRQs: Interrupts generated by software

Handling interrupts in Linux:
```c
static irqreturn_t my_interrupt_handler(int irq, void *dev_id)
{
    /* Handle the interrupt */
    return IRQ_HANDLED;
}

/* Register the handler */
request_irq(irq_num, my_interrupt_handler, IRQF_SHARED, "my_device", dev_data);
```

### 1.11.2. Top and Bottom Halves
Linux splits interrupt handling into two parts:
- **Top half**: Runs in interrupt context, cannot sleep, must complete quickly
- **Bottom half**: Defers more detailed processing, runs after the top half completes

Bottom half mechanisms:
1. **Softirq**: Code scheduled to run after top halves complete
2. **Tasklet**: Built on softirq, simpler for drivers
3. **Work queues**: Run in process context, can sleep

### 1.11.3. Threaded IRQs
A more modern approach to handling interrupts:
```c
request_threaded_irq(irq, quick_handler, thread_fn, IRQF_ONESHOT, "name", data);
```
- `quick_handler`: Fast interrupt function (can be NULL)
- `thread_fn`: Runs in a kernel thread, can sleep

## 1.12. Scheduling and Processes

### 1.12.1. Scheduler
The Completely Fair Scheduler (CFS) is the default scheduler:
- Ensures fair CPU time distribution
- Uses a red-black tree to track runnable processes
- Assigns time slices based on nice values and task priority

Process states:
- TASK_RUNNING: Either running or ready to run
- TASK_INTERRUPTIBLE: Sleeping, can be woken by signals
- TASK_UNINTERRUPTIBLE: Sleeping, cannot be interrupted
- TASK_STOPPED: Stopped by a signal (e.g., SIGSTOP)
- TASK_ZOMBIE: Terminated but parent hasn't retrieved exit status

### 1.12.2. Process Representation
Each process is represented by a `task_struct`:
- Contains all process information
- Includes process ID, state, scheduling information, file descriptors, etc.
- Organized in a doubly-linked list

### 1.12.3. Process Creation
New processes are created through:
- `fork()`: Creates a copy of the current process
- `clone()`: Like fork but with more control over what is shared
- `execve()`: Replaces the current process image with a new one

## 1.13. Memory Management

### 1.13.1. Virtual Memory System
The Linux virtual memory system:
- Maps virtual addresses to physical addresses
- Provides memory protection between processes
- Implements demand paging and copy-on-write

Each process has its own page tables defining its address space.

### 1.13.2. Memory Layout
The typical memory layout of a process:
- Text segment (code)
- Data segment (initialized data)
- BSS segment (uninitialized data)
- Heap (dynamic memory allocation)
- Memory mappings (shared libraries, mmap)
- Stack (local variables, function calls)

### 1.13.3. Page Cache
The page cache improves performance:
- Caches file contents in memory
- Reduces disk I/O
- Managed by the kernel's reclaim algorithm
- Unified with buffer cache in modern Linux

## 1.14. I/O and Block Subsystem

### 1.14.1. I/O Scheduler
The I/O scheduler optimizes disk access:
- Merges adjacent requests
- Reorders requests to minimize seek time
- Different algorithms for different workloads:
  - CFQ (Completely Fair Queuing)
  - Deadline
  - NOOP
  - BFQ (Budget Fair Queuing)

### 1.14.2. Block Layer
The block layer:
- Manages block devices (hard drives, SSDs)
- Handles I/O request queues
- Provides interfaces for filesystems
- Implements the bio structure for I/O operations

### 1.14.3. Direct I/O and Buffered I/O
Linux supports different I/O modes:
- **Buffered I/O**: Data passes through the page cache
- **Direct I/O**: Bypasses the page cache
- **Memory-mapped I/O**: Maps files directly into process address space

## 1.15. Power Management

### 1.15.1. CPU Power States
Modern CPUs support various power states:
- C-states: CPU idle states (C0, C1, C2, etc.)
- P-states: Performance states (different frequencies)
- T-states: Throttling states (thermal management)

### 1.15.2. Kernel Power Management Interfaces
Linux provides several power management frameworks:
- **CPUFreq**: CPU frequency scaling
- **CPUIdle**: Managing CPU idle states
- **ACPI/APM**: System-wide power management
- **Runtime PM**: Device-specific power management

### 1.15.3. Device Power Management
Devices can be power-managed using:
- **System suspend**: All devices suspended during system sleep
- **Runtime PM**: Individual devices suspended when idle
- The device driver must implement power management operations (suspend, resume, etc.)

## 1.16. Security Features

### 1.16.1. Access Control
Linux implements various access control mechanisms:
- **Traditional UNIX permissions**: User, group, others
- **Capabilities**: Fine-grained privileges
- **SELinux/AppArmor**: Mandatory Access Control (MAC)
- **Seccomp**: System call filtering
- **Namespaces**: Resource isolation

### 1.16.2. Memory Protection
Linux includes several memory protection features:
- **ASLR**: Address Space Layout Randomization
- **NX bit**: Non-executable memory
- **KASLR**: Kernel Address Space Layout Randomization
- **Stack canaries**: Stack overflow protection

### 1.16.3. Seccomp
Seccomp (Secure Computing Mode) restricts the system calls a process can make:
- BPF filters define allowed system calls
- Helps contain compromised processes
- Used by container technologies

## 1.17. Debugging and Tracing

### 1.17.1. Kernel Logging
The kernel logs messages using `printk()`:
- Different log levels (EMERG, ALERT, CRIT, ERR, WARNING, etc.)
- Messages stored in a ring buffer
- Accessible via `dmesg` command or syslog

### 1.17.2. Debugging Tools
Various tools aid kernel debugging:
- **KGDB**: GDB-based kernel debugger
- **Kernel crash dumps**: Post-mortem analysis
- **Magic SysRq**: Emergency commands even when system appears hung
- **perf**: Performance analysis tools

### 1.17.3. Tracing Systems
Linux provides powerful tracing capabilities:
- **ftrace**: Function tracer
- **tracepoints**: Static trace points in code
- **kprobes/uprobes**: Dynamic tracing
- **BPF/eBPF**: Extended Berkeley Packet Filter for tracing and more

### 1.17.4. Dynamic Debugging
Dynamic debug allows enabling/disabling debug messages at runtime:
- CONFIG_DYNAMIC_DEBUG kernel option
- Control via /proc/dynamic_debug/control
- Per-file, per-module, or per-message control

## 1.18. Kernel Development Process

### 1.18.1. Development Cycle
The kernel follows a time-based release cycle:
- 2-week merge window for new features
- ~8 weeks of release candidates
- Final release and start of new cycle

### 1.18.2. Maintainer Structure
The kernel has a hierarchical maintainer structure:
- Linus Torvalds at the top
- Subsystem maintainers (networking, filesystem, etc.)
- Driver maintainers
- Architecture maintainers

### 1.18.3. Contribution Process
Contributing to the kernel requires:
- Creating patches using `git format-patch`
- Following kernel coding style
- Adding appropriate sign-offs
- Sending to appropriate mailing lists
- Responding to feedback

### 1.18.4. Code Review
All kernel code undergoes strict review:
- Maintainers review for technical correctness
- The community reviews for bugs, style issues
- Multiple review iterations may be needed
- Final acceptance by the subsystem maintainer