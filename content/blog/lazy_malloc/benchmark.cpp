#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <Windows.h>
#include <Psapi.h>

double getWallTime_s() {
    LARGE_INTEGER time, freq;
    if (!QueryPerformanceFrequency(&freq) ||
        !QueryPerformanceCounter(&time)){
        return 0;
    }
    return (double)time.QuadPart / freq.QuadPart;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return -1;
    }

    char* mode = argv[1];

    PROCESS_MEMORY_COUNTERS c0, c1, c2;

    int numElems = 10000000;
    int *mem = (int*)malloc(numElems * sizeof(int));

    GetProcessMemoryInfo(GetCurrentProcess(), &c0, sizeof(c0));

    if (!strcmp(mode, "lazy")) {
    }
    else if (!strcmp(mode, "prefetch")) {
        WIN32_MEMORY_RANGE_ENTRY memRange;
        memRange.VirtualAddress = mem;
        memRange.NumberOfBytes = numElems * sizeof(int);
        PrefetchVirtualMemory(GetCurrentProcess(), 1, &memRange, 0);
    }
    else if (!strcmp(mode, "init")) {
        memset(mem, INT_MIN, numElems * sizeof(int));
    }
    else {
        return -1;
    }

    GetProcessMemoryInfo(GetCurrentProcess(), &c1, sizeof(c1));

    double start = getWallTime_s();

    volatile int sum = 0; // Make sure this loop is not optimized away
    for (int i = 0; i < numElems; i += 1000) {
        mem[i] = rand();
        sum += mem[i];
    }

    double end = getWallTime_s();

    GetProcessMemoryInfo(GetCurrentProcess(), &c2, sizeof(c2));

    free(mem);

    printf("Time: %f s\n", end - start);
    printf("Pagefaults (Prefetch): %d\n", c1.PageFaultCount - c0.PageFaultCount);
    printf("Pagefaults (Hot): %d\n", c2.PageFaultCount - c1.PageFaultCount);

    return 0;
}
