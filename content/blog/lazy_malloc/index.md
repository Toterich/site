+++
title = 'Effects of lazy allocation with malloc() and friends'
date = 2024-08-18T15:49:16+02:00
draft = true
+++

## TLDR

* On Linux and Windows, allocating heap memory with `malloc()` adds the requested memory range to the process' page table, not necessarily backing it by physical memory yet
* Physical memory is only paged in on first access
* An OOM condition can occur on memory access even if the allocation succeeded
* In performance-critical applications, lazy ...
* Eager mapping can be forced either by OS-specific syscalls or simply initializing the allocated memory to a sentinel value

## Introduction

In this post, I will present a brief overview over how `malloc()` and friends allocate memory and how this relates to the physical RAM of the host machine, followed by an assessment of potential issues for the application programmer when relying on platform defaults. I will also offer an easy-to-use, platform-agnostic technique to force eager mapping, which might help mitigating allocation-related performance issues in your program.

## Primer on virtual memory

## Potential issues with lazy allocation

In general, there are good reasons why both Linux and Windows don't immediately back any allocated virtual address space by physical pages in RAM, but default to lazy, "on-demand" paging instead. It allows the operating system to allocate more memory than is physically available in the system. As memory pages that are not currently needed are written out to secondary storage, the limit here is the size of your swap file.

Nevertheless, there are applications where one might prefer to all allocated memory to be backed by physical RAM immediately.

SEE https://stackoverflow.com/questions/57125253/why-is-iterating-though-stdvector-faster-than-iterating-though-stdarray/57130924#57130924

### Running out of memory unexpectedly

Lazy mapping means that your program might experience Out-of-memory issues even if the single allocation you performed at program start succeeded. The OS is perfectly fine with allocating more virtual address space than there is available physical memory, in fact being able to do this is one of the motivations of having the abstraction layer of virtual memory in the first place.

When you then at a later point in time start using the allocated memory and triggering page faults, at some point no physical page might be free anymore to actually map to part of the allocated virtual memory. Platforms have different ways to deal with situations like these, either by swapping some part of the physical memory to disk or killing processes in order to free up space. See e.g. (Out Of Memory Management)[https://www.kernel.org/doc/gorman/html/understand/understand016.html] for an in-depth explanation of Linux' OOM Killer. The specific OS strategies to deal with overcommitment of memory are out of scope for this blog post. The takeaway is just that preallocating all required memory for an application on startup does not necessarily protect it from experiencing an OOM condition at a later point.

### Page faults on the hot path

When a process tries to access some newly allocated memory address and that address is not backed by physical memory yet, the operating system needs to find a page in RAM that is either not allocated by any process or one that can be safely swapped to secondary storage, so that the current process can start using the page instead of the original one (*page stealing*). In the former case, the mapping from the virtual address of the process' allocated address range to the physical page needs to be stored in the process' Translation Lookaside Buffer (*TLB*). In the latter case, additionally the page needs to be written out to disk first and the mapping in the original process' TLB be deleted.
However, both of these operations are not free; they involve a context switch from user to kernel space as well as (in the second example) a slow write to secondary storage.

For this reason, in some performance critical applications, it might be desirable to move the expensive page faults outside of a program's hot path. In practice, the effect of doing so probably only brings miniscule benefits in most scenarios, as often times the hot path is executed multiple times (e.g. the update-and-render loop in any graphical application), so the page fault costs are only paid in the first execution of the loop. Still, cases in which even the very first iteration of the hot path needs to be as quick as possible are conceivable, e.g. in high frequency trading.

## Approaches to force eager mapping

To enforce physical backing of a newly allocated address range, there are two possible approaches. The first is to either use platform-specific system calls to inform the operating system that a range of addresses will be needed. The alternative is to simply write any data to the allocated memory sometime between calling `malloc()` and the first use of the address range.

### Platform dependent syscalls

On Windows, there is the (PrefetchVirtualMemory)[https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-prefetchvirtualmemory] API call, which is able to cache multiple virtual address ranges in physical RAM. Notably though, it doesn't add the memory to the process' working set, meaning it does not establish a mapping in the process' TLB. The documenatation reads:

```
The prefetched memory is not added to the target process' working set; it is cached in physical memory. When the prefetched address ranges are accessed by the target process, they will be added to the working set.
```

This means that

### Initializing memory to sentinel

## Benchmark

```
D:\projects\site\content\blog\lazy_malloc>benchmark.exe lazy
Time: 0.004556 s
Pagefaults (Prefetch): 0
Pagefaults (Hot): 9785

D:\projects\site\content\blog\lazy_malloc>benchmark.exe prefetch
Time: 0.004169 s
Pagefaults (Prefetch): 1
Pagefaults (Hot): 9784

D:\projects\site\content\blog\lazy_malloc>benchmark.exe init
Time: 0.000189 s
Pagefaults (Prefetch): 9783
Pagefaults (Hot): 2
```


* Initializing a numpy array usually allocates all the required memory at once (eagerly)
* Using numpy.zeros() behaves differently
* Because of calloc, which retrieves

The [numpy.ndarray](https://numpy.org/doc/stable/reference/generated/numpy.ndarray.html) class is one the most basic building blocks of NumPy, providing an interface to an n-dimensional array of an arbitrary datatype.

Internally, an instance of `ndarray`, maps to a contiguous area of memory in the Python process' virtual adress space. This is demonstrated in the following snippet. We first create an array of 5 elements. We then use `ctypes.string_at` to read back the internal memory at each element offset.

```python
>>> import numpy as np
>>> from ctypes import string_at
>>> arr = np.linspace(1, 5, 5, dtype=np.int64)
>>> align = arr.dtype.alignment
>>> align
8
>>> ptr = arr.__array_interface__['data'][0]
>>> string_at(ptr, align).hex()
'0100000000000000'
>>> string_at(ptr+align, align).hex()
'0200000000000000'
>>> string_at(ptr+2*align, align).hex()
'0300000000000000'
>>> string_at(ptr+3*align, align).hex()
'0400000000000000'
>>> string_at(ptr+4*align, align).hex()
'0500000000000000'
```

So at least in the case of `numpy.linspace()`, the behavior seems to be like one would initially expect: A


use_calloc in numpy source
https://github.com/numpy/numpy/blob/d60444f2383b9549e02bb61db956b91a5110ead1/numpy/_core/src/multiarray/ctors.c#L853

https://stackoverflow.com/questions/1538420/difference-between-malloc-and-calloc


https://stackoverflow.com/questions/67240022/lazy-overcommit-allocation-and-calloc


# View Pagetable for process
https://unix.stackexchange.com/questions/369185/viewing-pagetable-for-a-process

https://medium.com/@besartdollma/lazy-dynamic-memory-allocation-in-c-32bb4228108b
