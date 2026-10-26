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


/* 검증                                                                */
/* ------------------------------------------------------------------ */

int isSorted(const Data data[], int n) {
    for (int i = 1; i < n; i++)
        if (data[i - 1].key1 > data[i].key1) return 0;
    return 1;
}

/* Key1이 같은 인접 쌍 중 originalIndex 순서가 뒤집힌 개수 (0이면 안정) */
long long stabilityViolations(const Data data[], int n) {
    long long v = 0;
    for (int i = 1; i < n; i++)
        if (data[i - 1].key1 == data[i].key1 &&
            data[i - 1].originalIndex > data[i].originalIndex) v++;
    return v;
}

/* 데이터가 유실/변형되지 않았는지 확인하는 체크섬 */
long long checksum(const Data data[], int n) {
    long long sum = 0;
    for (int i = 0; i < n; i++)
        sum += (long long)data[i].key1 + 31LL * data[i].key2
             + 131LL * data[i].originalIndex;
    return sum;
}

int isValidResult(const Data result[], const Data original[], int n) {
    return isSorted(result, n) && checksum(result, n) == checksum(original, n);
}
