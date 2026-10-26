#include <stdio.h>
#include <stdlib.h>

#define DATA_COUNT 10000
#define TRIALS 100
#define KEY_MAX 1000000
#define SEED 20250101u
#define PARTIAL_SWAP_DIV 20
#define STAB_SMALL_COUNT 1000
#define STAB_SMALL_RANGE 10

typedef struct {
    int key1;
    int key2;
    int originalIndex;
} Data;

typedef struct {
    long long comparisons;
    long long movements;
} Stats;

typedef enum {
    ALGO_SELECTION = 0,
    ALGO_BUBBLE,
    ALGO_BUBBLE_EARLY,
    ALGO_INSERTION_GT,
    ALGO_INSERTION_GE,
    ALGO_COUNT
} Algo;

extern const char *ALGO_NAMES[ALGO_COUNT];

int randomInt(int maxInclusive);
void generateData(Data data[], int n);
void copyData(Data dst[], const Data src[], int n);
void resetOriginalIndex(Data data[], int n);
void makeSorted(Data dst[], const Data base[], int n);
void makeReverse(Data dst[], const Data sorted[], int n);
void makePartial(Data dst[], const Data sorted[], int n, int swaps);

Stats selectionSort(Data data[], int n);
Stats bubbleSort(Data data[], int n, int earlyStop);
Stats insertionSort(Data data[], int n, int useGE);
Stats runAlgo(Algo a, Data data[], int n);

int isSorted(const Data data[], int n);
long long stabilityViolations(const Data data[], int n);
long long checksum(const Data data[], int n);
int isValidResult(const Data result[], const Data original[], int n);

void printSeparator(void);
void printSequence(const Data data[], int n);
void runInputCase(const char *title, const char *note,
                  const Data input[], Data work[], int n);


/* 난수 / 데이터 생성                                                  */
/* ------------------------------------------------------------------ */

/* 플랫폼(Windows/macOS/Linux)에 관계없이 같은 결과가 나오도록 직접 구현한 난수 생성기 (xorshift64*) */
static unsigned long long rngState = 0x9E3779B97F4A7C15ULL ^ (unsigned long long)SEED;

static unsigned long long nextRand(void) {
    rngState ^= rngState >> 12;
    rngState ^= rngState << 25;
    rngState ^= rngState >> 27;
    return rngState * 2685821657736338717ULL;
}

/* 0 ~ maxInclusive 범위의 정수 */
int randomInt(int maxInclusive) {
    return (int)((nextRand() >> 33) % ((unsigned long long)maxInclusive + 1ULL));
}

void generateData(Data data[], int n) {
    for (int i = 0; i < n; i++) {
        data[i].key1 = randomInt(KEY_MAX);
        data[i].key2 = randomInt(KEY_MAX);
        data[i].originalIndex = i;
    }
}

void copyData(Data dst[], const Data src[], int n) {
    for (int i = 0; i < n; i++) dst[i] = src[i];
}

void resetOriginalIndex(Data data[], int n) {
    for (int i = 0; i < n; i++) data[i].originalIndex = i;
}

/* 사전 정렬용 비교 함수 (Key1, 같으면 원래 순서). 시험 입력 준비 전용이며 횟수에 포함되지 않음 */
static int cmpKeyThenIndex(const void *a, const void *b) {
    const Data *x = (const Data *)a;
    const Data *y = (const Data *)b;
    if (x->key1 != y->key1) return (x->key1 < y->key1) ? -1 : 1;
    if (x->originalIndex != y->originalIndex)
        return (x->originalIndex < y->originalIndex) ? -1 : 1;
    return 0;
}

/* 동일한 원본(base)으로부터 세 가지 입력 상태를 만든다 */
void makeSorted(Data dst[], const Data base[], int n) {
    copyData(dst, base, n);
    qsort(dst, (size_t)n, sizeof(Data), cmpKeyThenIndex);
    resetOriginalIndex(dst, n);
}

void makeReverse(Data dst[], const Data sorted[], int n) {
    for (int i = 0; i < n; i++) dst[i] = sorted[n - 1 - i];
    resetOriginalIndex(dst, n);
}

void makePartial(Data dst[], const Data sorted[], int n, int swaps) {
    copyData(dst, sorted, n);
    for (int i = 0; i < swaps; i++) {
        int a = randomInt(n - 1);
        int b = randomInt(n - 1);
        Data t = dst[a];
        dst[a] = dst[b];
        dst[b] = t;
    }
    resetOriginalIndex(dst, n);
}
