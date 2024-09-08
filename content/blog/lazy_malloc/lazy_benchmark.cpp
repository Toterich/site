#include <cstdlib>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
# include <Windows.h>
double getWallTime_s() {
    LARGE_INTEGER time, freq;
    if (!QueryPerformanceFrequency(&freq) ||
        !QueryPerformanceCounter(&time)){
        return 0;
    }
    return (double)time.QuadPart / freq.QuadPart;
}

void Prefetch(void* address, int size) {
    WIN32_MEMORY_RANGE_ENTRY memRange;
    memRange.VirtualAddress = address;
    memRange.NumberOfBytes = size * sizeof(int);
    PrefetchVirtualMemory(GetCurrentProcess(), 1, &memRange, 0);
}

#else
# include <time.h>
# include <sys/time.h>
# include <sys/mman.h>
double getWallTime_s() {
    timeval time;
    if (gettimeofday(&time, nullptr)) {
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void Prefetch(void* address, int size) {
    printf("Memory Prefetch is only implemented on Windows.\n");
}
#endif

int main(int argc, char* argv[]) {
    int numElems = 10000000;

    if (argc < 2) {
        return -1;
    }

    char* mode = argv[1];

    int *mem = (int*)malloc(numElems * sizeof(int));
    if (!strcmp(mode, "lazy")) {
    }
    else if (!strcmp(mode, "prefetch")) {
        Prefetch(mem, numElems * sizeof(int));
    }
    else if (!strcmp(mode, "initialize")) {
        memset(mem, 0, numElems * sizeof(int));
    }
    else {
        return -1;
    }
    
    double start = getWallTime_s();

    for (int i = 0; i < numElems; i++) {
        mem[i] = i;
    }

    double end = getWallTime_s();

    free(mem);

    printf("Time: %f s\n", end - start);

    return 0;
}