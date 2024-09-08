+++
title = 'Allocating zeroed memory may lead to lazy allocation'
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
