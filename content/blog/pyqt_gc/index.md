+++
title = "Incompatibility of Qt with Python's garbage collector"
date = 2024-09-23T07:40:17+02:00
type = "post"
draft = false
+++

When using Qt in Python (either via [Pyqt](https://wiki.python.org/moin/PyQt) or [PySide](https://doc.qt.io/qtforpython-6/index.html)), there is the potential for memory corruption and segfaults every time Python's cyclic garbage collector deletes a QObject. The first mention of this problem I could find is [this mailing list from 2011](https://www.riverbankcomputing.com/pipermail/pyqt/2011-August/030376.html). As far as I can tell, this issue still persists in 2024 with current versions of Qt and Python. In fact, it seems to be a fundamental problem that can't really be fixed. Nevertheless, mitigations are possible.

Note: This post relates to *CPython* only. I don't have experience with other Python implementations, but suspect they suffer from the same issue as long as they also employ synchronous garbage collection of reference cycles.

## Deleting QObjects

Qt requires QObjects to be deleted in the thread that "owns" them (which is usually the creating thread, but can be another one when the object's thread affinity was changed using [QObject::moveToThread()](https://doc.qt.io/qt-6/qobject.html#moveToThread)). The documentation mentions this specifically [here](https://doc.qt.io/qt-6/threads-qobject.html#per-thread-event-loop):

```
Calling delete on a QObject from a thread other than the one that owns the object (or accessing the object in other ways) is unsafe, unless you guarantee that the object isn't processing events at that moment. Use QObject::deleteLater() instead, and a DeferredDelete event will be posted, which the event loop of the object's thread will eventually pick up. By default, the thread that owns a QObject is the thread that creates the QObject, but not after QObject::moveToThread() has been called.
```

So usually, all is well as long as object deletions are deferred using `QObject::deleteLater()`, in which case the owning thread is signaled to delete the object as soon as its event loop comes across the scheduled deletion event.

## Python's cyclic Garbage Collector

Unfortunately, Python is not aware of this requirement of QObjects and might delete them from an arbitrary thread. The only prerequisite for this is that the objects are part of a reference cycle, which can't be cleaned up by CPython's reference counting scheme.

Usually, CPython cleans up objects by keeping track of the number of references of any existing object. As soon as this count drops to zero, the object is deleted.

A reference cycle basically means that multiple objects are referencing each other. In this case, even if there are no outside references to elements in the cycle, the reference count for any of the elements will never reach zero, and they will never be deleted by the reference counter.

A simple example of a single-element ref cycle would be:

```python
l = []
l.append(l)
```

where the first element in `l` references the list itself.

There are tools in Python to prevent creation of reference cycles, e.g. the [weakref](https://docs.python.org/3.14/library/weakref.html) module, but in my experience it is almost impossible to completely avoid them in any non-trivial Python program. Even if your application code itself does not produce any cycles, many libraries do cause them quite often.

The reason why this is generally ok is Python's secondary garbage collection scheme, which detects reference cycles and deletes them following a heuristic. The internal design of Python's cyclic garbage collector is out-of-scope for this post, but you can read up on it [here](https://devguide.python.org/internals/garbage-collector/index.html#collecting-the-oldest-generation).

The GC performs implicit collections synchronously. This means that whenever the Python runtime detects that a GC execution is warranted, the application code in whatever thread is currently executing is halted and the GC runs in that thread. After the GC collection finishes, the application resumes.

Now, when one happens to have any `QObject`s in a reference cycle in Python, the GC will indiscriminately delete those along with all other objects in the cycle. If the GC executes in a thread that is not the one owning the `QObject`, this violates Qt's contract on deleting the object and your application might crash or exhibit unintended behavior due to memory corruption. This might happen very rarely and with weird, seemingly unrelated stack traces, which makes issues like this hard to debug.

## Workaround: Forcing GC runs in the GUI thread

In the mailing list I linked at the beginning, one user presents a workaround for this issue, which involves [explicitly calling the GC from the application's GUI thread](https://www.riverbankcomputing.com/pipermail/pyqt/2011-August/030378.html). The implicit gc is deactivated.

This works as long as all `QObject`s in an application are owned by the GUI thread, which is likely when Qt is used exclusively for the GUI portion of your program. In fact, it seems to be kind of an open secret among Qt-on-Python developers that a workaround like the linked one is necessary. For example, the widely-used scientific plotting library `pyqtgraph` contains [an implementation](https://github.com/pyqtgraph/pyqtgraph/blob/master/pyqtgraph/util/garbage_collector.py) of it, which is however not utilized in the project itself or advertised in any way.

I do wonder why this behavior and the workaround are not documented more clearly by either the PySide or PyQt projects themselves. In my opinion, this information is critical for anyone that wants to develop a non-trivial Python application which uses Qt as its frontend.
