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


/* 정렬 알고리즘                                                       */
/* ------------------------------------------------------------------ */

Stats selectionSort(Data data[], int n) {
    Stats s = {0, 0};
    for (int i = 0; i < n - 1; i++) {
        int minIndex = i;
        for (int j = i + 1; j < n; j++) {
            s.comparisons++;
            if (data[j].key1 < data[minIndex].key1) minIndex = j;
        }
        if (minIndex != i) {
            Data temp = data[i];            s.movements++;
            data[i] = data[minIndex];       s.movements++;
            data[minIndex] = temp;          s.movements++;
        }
    }
    return s;
}

/* earlyStop = 0: 모든 순회 수행, 1: 교환이 없으면 종료 */
Stats bubbleSort(Data data[], int n, int earlyStop) {
    Stats s = {0, 0};
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            s.comparisons++;
            if (data[j].key1 > data[j + 1].key1) {
                Data temp = data[j];        s.movements++;
                data[j] = data[j + 1];      s.movements++;
                data[j + 1] = temp;         s.movements++;
                swapped = 1;
            }
        }
        if (earlyStop && !swapped) break;
    }
    return s;
}

/* useGE = 0: Case A (>), useGE = 1: Case B (>=) */
Stats insertionSort(Data data[], int n, int useGE) {
    Stats s = {0, 0};
    for (int i = 1; i < n; i++) {
        Data current = data[i];             s.movements++;
        int j = i - 1;
        while (j >= 0) {
            s.comparisons++;
            int shift = useGE ? (data[j].key1 >= current.key1)
                              : (data[j].key1 >  current.key1);
            if (!shift) break;
            data[j + 1] = data[j];          s.movements++;
            j--;
        }
        data[j + 1] = current;              s.movements++;
    }
    return s;
}

Stats runAlgo(Algo a, Data data[], int n) {
    switch (a) {
        case ALGO_SELECTION:    return selectionSort(data, n);
        case ALGO_BUBBLE:       return bubbleSort(data, n, 0);
        case ALGO_BUBBLE_EARLY: return bubbleSort(data, n, 1);
        case ALGO_INSERTION_GT: return insertionSort(data, n, 0);
        default:                return insertionSort(data, n, 1);
    }
}
