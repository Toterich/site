+++
title = 'Lazy allocation and forcing early pagefaults'
date = 2024-09-20T15:49:16+02:00
type = "post"
draft = false
+++

## TLDR

* On Linux and Windows, allocating heap memory with `malloc()` adds the requested memory range to the process' page table, not necessarily backing it by physical memory yet
* Physical memory is only paged in on first access
* An OOM condition can occur on memory access even if the allocation succeeded
* In performance-critical applications, pagefaults can add a significant overhead to the hot path
* Eager mapping can be forced either by OS-specific syscalls or simply initializing the allocated memory to a sentinel value

## Introduction

When an application requests some amount of memory from the Operating System (e.g. by calling `malloc()` in a C-program), it receives a pointer to some address, with the understanding that on that address lies a continuous range of memory of the requested size for the current process to do with as it pleases. Internals of how the OS, in conjunction with primary (RAM) and secondary (disk) storage, handles the memory allocation, are abstracted away and usually not of interest for the application programmer.

In some situations however, this abstraction starts to leak and it pays to dive a bit deeper into how the system actually manages memory. The first important part to consider is that the address range a process received via `malloc()` does not denote an actual region of memory anywhere on a physical device. Instead, it points to *Virtual Memory*, an abstraction that is local to each process. The OS takes care of backing this abstract virtual memory of a process by an actual storage device. However, it doesn't necessarily do so immediately, but in many cases only when the allocated memory is actually used by the process. When a process accesses some of its virtual memory that is not yet backed by physical memory, a *page fault* occurs which triggers the OS to clear some space on primary storage for the process to use.

While this process is usually opaque to the user process, some issues can arise from it in certain applications.

## Potential issues with lazy allocation

In general, there are good reasons why both Linux and Windows don't immediately back any allocated virtual address space by physical pages in RAM, but default to lazy, "on-demand" paging instead. For instance, it allows the operating system to allocate more memory than is physically available in the system. As memory pages that are not currently needed are written out to secondary storage, the limit here is the size of your swap file. It also helps avoiding fragmentation the virtual address space; a range of memory can be contiguous in the view of a process, even though it is backed by multiple physical pages all over the system's RAM.

Nevertheless, there are applications where one might prefer all allocated memory to be backed by physical RAM immediately, for the following reasons.

### Running out of memory unexpectedly

Lazy mapping means that your program might experience Out-of-memory issues even if the single allocation you performed at program start succeeded. The OS is perfectly fine with allocating more virtual address space than there is available physical memory, in fact being able to do this is one of the motivations of having the abstraction layer of virtual memory in the first place.

When you then at a later point in time start using the allocated memory and triggering page faults, at some point no physical page might be free anymore to actually map to part of the allocated virtual memory. Platforms have different ways to deal with situations like these, either by swapping some part of the physical memory to disk or killing processes in order to free up space. See e.g. (Out Of Memory Management)[https://www.kernel.org/doc/gorman/html/understand/understand016.html] for an in-depth explanation of Linux' OOM Killer. The specific OS strategies to deal with overcommitment of memory are out of scope for this blog post. The takeaway is just that preallocating all required memory for an application on startup does not necessarily protect it from experiencing an OOM condition at a later point.

### Page faults on the hot path

When a process tries to access some newly allocated memory address and that address is not backed by physical memory yet, the operating system needs to find a page in RAM that is either not allocated by any process or one that can be safely swapped to secondary storage, so that the current process can start using the page instead of the original one (*page stealing*). In the former case, the mapping from the virtual address of the process' allocated address range to the physical page needs to be stored in the process' Translation Lookaside Buffer (*TLB*). In the latter case, additionally the page needs to be written out to disk first and the mapping in the original process' TLB be deleted.
However, both of these operations are not free; they involve a context switch from user to kernel space as well as (in the second example) a slow write to secondary storage.

For this reason, in some performance critical applications, it might be desirable to move the expensive page faults outside of a program's hot path. In practice, the effect of doing so probably only brings miniscule benefits in most scenarios, as often times the hot path is executed multiple times (e.g. the update-and-render loop in any graphical application), so the page fault costs are only paid in the first execution of the loop. Still, cases in which even the very first iteration of the hot path needs to be as quick as possible are conceivable, e.g. in high frequency trading.

## Approaches to force eager mapping

To enforce physical backing of a newly allocated address range, there are two possible approaches. The first is to either use platform-specific system calls to inform the operating system that a range of addresses will be needed. The alternative is to simply write any data to the allocated memory sometime between calling `malloc()` and the first use of the address range.

### Platform dependent system calls

#### Linux

On Linux, the (mmap)[https://man7.org/linux/man-pages/man2/mmap.2.html] syscall allows creating a mapping of virtual addresses. Specifically, the documentation of the `MAP_POPULATE` flag reads:

```
Populate (prefault) page tables for a mapping. For a file
mapping, this causes read-ahead on the file. This will
help to reduce blocking on page faults later.
```

, which sounds like it fixes our problem exactly.

#### Windows

Sadly, Windows does not provide an equivalent API to Linux' `mmap` with the `MAP_POPULATE` flag. There is (PrefetchVirtualMemory)[https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-prefetchvirtualmemory], which is able to cache multiple virtual address ranges in physical RAM. Notably though, it doesn't add the memory to the process' working set, meaning it does not establish a mapping between virtual and physical addresses in the process' TLB. The documentation reads:

```
The prefetched memory is not added to the target process' working set; it
is cached in physical memory. When the prefetched address ranges are
accessed by the target process, they will be added to the working set.
```

This means this function is mostly useful to fetch already written-to memory from disk that has been unpaged in the meantime, as it allows bundling of the required IO operations. Accessing the prefetched memory will still trigger a pagefault and a context switch to the kernel to update the TLB. Therefore the only performance gains we could expect from this for the usecase of prefetching newly allocated memory would come from the potentially required unpaging of some other memory in order to clear space. We will see in the [benchmark section](#benchmark) whether this caveat has an effect on the performance numbers.


### Initializing memory to sentinel

Alternatively to the system-specific facilities mentioned above, a portable way to prefetch some address range is to simply write some value to it. Then the OS has no choice but to back the virtual memory with physical pages, as the stored data needs to live somewhere. The following snippet allocates space for 42 integers and then initializes each one to a sentinel value.

```c
int *mem = (int*)malloc(42 * sizeof(int));
int const sentinel = –2147483648;
for (int i = 0; i < 42; i++) {
    mem[i] = sentinel;
}
```

NOTE: Watch out when trying to use `0` as the sentinel value (or use `calloc` instead of `malloc` to immediately zero the memory upon allocation). In this case, the OS may choose to map multiple virtual pages to the same zeroed location in physical memory, which means you still need to pay the cost of remapping pages to new regions when you actually write useful data to them. For this reason, it is more sensible for this trick to use a non-zero initialization value.

Of course, it depends on your application if there actually is an unneeded value in the datatype of your allocation that you can use for initialization here.

## Benchmark

Here is a small benchmark application that measures both the runtime and the number of pagefaults when writing random data to some memory range. Depending on the input parameter, the application initializes the memory in some way beforehand:

* `lazy`: No pre-initialization
* `prefetch`: Using the Windows-specific `PrefetchVirtualMemory` call after allocation,
* `init`: Initializing all memory to a constant value after allocation.

The benchmark code is Windows-only, only because the machine I'm currently working on runs on Windows. Investigating the behavior of `mmap` would still be interesting and I might add a Linux benchmark to this blogpost at a later date.

{{< code language="c" source="/content/blog/lazy_malloc/benchmark.c">}}

### Results

This is the output of the benchmark application. Repeating this several times showed similar results:

```
D:\projects\site\content\blog\lazy_malloc>benchmark.exe lazy
Time: 0.004556 s
Pagefaults (Prepare): 0
Pagefaults (Hot): 9785

D:\projects\site\content\blog\lazy_malloc>benchmark.exe prefetch
Time: 0.004169 s
Pagefaults (Prepare): 1
Pagefaults (Hot): 9784

D:\projects\site\content\blog\lazy_malloc>benchmark.exe init
Time: 0.000189 s
Pagefaults (Prepare): 9783
Pagefaults (Hot): 2
```

Both the `lazy` and `init` runs perform as expected. On the `lazy` run, there are no pagefaults in the "Prepare" stage (because there is none), but a high number of them during the actual benchmark. On the `init` run, all pagefaults are happening in the "Prepare" stage, and as a result the benchmark itself finishes much quicker, by a factor of ~24 for this example and on my machine (Intel I7 12700K).

The `prefetch` run shows that `PrefetchVirtualMemory` didn't save any pagefaults on the hot path, which is in line with the documentation of that function. The runtime is comparable to the `lazy` execution, so this adds to the argument that `PrefetchVirtualMemory` probably isn't the right tool to achieve eager mapping of memory on Windows. Instead, the pre-initialization method should be used.
