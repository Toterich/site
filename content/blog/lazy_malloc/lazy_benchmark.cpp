#include <Windows.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

double getWallTime_s() {
    LARGE_INTEGER time, freq;
    if (!QueryPerformanceFrequency(&freq) ||
        !QueryPerformanceCounter(&time)){
        return 0;
    }
    return (double)time.QuadPart / freq.QuadPart;
}

int main(int argc, char* argv[]) {
    int numElems = 100000;

    if (argc < 2) {
        return -1;
    }

    int mode = atoi(argv[1]);

    int *mem = (int*)malloc(numElems * sizeof(int));
    if (mode == 1) {
        memset(mem, -1, numElems * sizeof(int));
    }
    else if (mode == 2) {
        WIN32_MEMORY_RANGE_ENTRY memRange;
        memRange.VirtualAddress = mem;
        memRange.NumberOfBytes = numElems * sizeof(int);
        PrefetchVirtualMemory(GetCurrentProcess(), 1, &memRange, 0);
    }
    
    double start = getWallTime_s();

    for (int i = 0; i < numElems; i++) {
        mem[i] = i;
    }

    double end = getWallTime_s();

    free(mem);

    printf("Mode %d: %f s\n", mode, end - start);

    return 0;
}