+++
title = 'Effects of lazy allocation with malloc() and friends'
date = 2024-08-18T15:49:16+02:00
draft = true
+++

1. Memory observation with numpy.zeros() vs numpy.ones()
2. malloc() may defer allocation of physical mem pages to first write (Copy on Write)
3. This can be a problem in memory constrained environments, where we need to know up front we have enough memory available for the desired allocation, OR in runtime performance critical situations where the actual allocation should be done outside of the hot loop
3. To make sure allocation has happened, write allocated data
4. calloc() may also defer allocation, even though data gets zeroed, because the whole requested range is mapped to the same memory page before it is written
5. To make sure allocation happens eagerly, manually write data with memset
6. malloc() + memset(0) may be optimized to calloc(), observed with Gcc and clang, but not MSVC
7. To be sure, memset to some non-zero sentinel value


#################################################################

## TLDR

* On Linux and Windows, allocating heap memory with `malloc()` adds the requested memory range to the process' page table, not necessarily backing it by physical memory yet
* Physical memory is only paged in on first access
* An OOM condition can occur on memory access even if the allocation succeeded
* In performance-critical applications, lazy ...
* Eager mapping can be forced either by OS-specific syscalls or simply initializing the allocated memory to a sentinel value

## Introduction

In this post, I will present a brief overview over how `malloc()` and friends allocate memory and how this relates to the physical RAM of the host machine, followed by an assessment of potential issues for the application programmer when relying on platform defaults. I will also offer an easy-to-use, platform-agnostic technique to force eager mapping, which might help mitigating allocation-related performance issues in your program.

## Virtual vs Physical Memory

## Potential issues with lazy mapping

In general, there are good reasons why both Linux and Windows default to lazy mapping of memory pages. TODO: WHICH Nevertheless, there are applications where one might prefer all allocated memory to be backed by physical RAM immediately.

### Running out of memory unexpectedly

Lazy mapping means that your program might experience Out-of-memory issues even if the single allocation you performed at program start succeeded. The OS is perfectly fine with allocating more virtual address space than physical memory is available, in fact that is one of the motivations of having the abstraction layer of virtual memory in the first place.

When you then at a later point in time start using the allocated memory and triggering page faults, at some point no physical page might be free anymore to actually map to part of the allocated virtual memory. Platforms have different ways to deal with situations like these, either by swapping some part of the physical memory to disk or killing processes in order to free up space. See e.g. (Out Of Memory Management)[https://www.kernel.org/doc/gorman/html/understand/understand016.html] for an in-depth explanation of Linux' OOM Killer. The specific OS strategies to deal with overcommitment of memory are out of scope for this blog post. The takeaway is just that preallocating all required memory for an application on startup does not necessarily protect it from experiencing an OOM condition at a later point.

### 

## Approaches to force eager mapping

### Platform dependent syscalls

### Initializing memory to sentinel

## Benchmark


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
