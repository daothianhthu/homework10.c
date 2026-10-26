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


/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void) {
    Data *base    = malloc(DATA_COUNT * sizeof(Data));
    Data *work    = malloc(DATA_COUNT * sizeof(Data));
    Data *sortedD = malloc(DATA_COUNT * sizeof(Data));
    Data *reverse = malloc(DATA_COUNT * sizeof(Data));
    Data *partial = malloc(DATA_COUNT * sizeof(Data));
    if (!base || !work || !sortedD || !reverse || !partial) {
        printf("Memory allocation failed!\n");
        free(base); free(work); free(sortedD); free(reverse); free(partial);
        return 1;
    }

    printf("Data Count : %d\n\n", DATA_COUNT);

    /* ---------------- 1. 무작위 데이터 100회 반복 ---------------- */
    printf("Random Data\n");
    printf("Number of Trials : %d\n\n", TRIALS);

    long long totalComp[ALGO_COUNT] = {0};
    long long totalMove[ALGO_COUNT] = {0};
    int allCorrect[ALGO_COUNT];
    for (int a = 0; a < ALGO_COUNT; a++) allCorrect[a] = 1;

    Algo randomOrder[4] = {ALGO_SELECTION, ALGO_BUBBLE, ALGO_BUBBLE_EARLY, ALGO_INSERTION_GT};

    for (int t = 0; t < TRIALS; t++) {
        generateData(base, DATA_COUNT);              /* 한 번만 생성 → 모든 알고리즘이 동일 데이터 사용 */
        for (int k = 0; k < 4; k++) {
            Algo a = randomOrder[k];
            copyData(work, base, DATA_COUNT);
            Stats s = runAlgo(a, work, DATA_COUNT);
            totalComp[a] += s.comparisons;
            totalMove[a] += s.movements;
            if (!isValidResult(work, base, DATA_COUNT)) allCorrect[a] = 0;
        }
    }

    for (int k = 0; k < 4; k++) {
        Algo a = randomOrder[k];
        printf("%s\n", ALGO_NAMES[a]);
        printf("Total Comparisons   : %lld\n", totalComp[a]);
        printf("Average Comparisons : %.2f\n", (double)totalComp[a] / TRIALS);
        printf("Average Movements   : %.2f\n", (double)totalMove[a] / TRIALS);
        printf("All Trials Correct  : %s\n\n", allCorrect[a] ? "Yes" : "No");
    }

    /* ---------------- 2. 입력 상태별 실험 ---------------- */
    /* 동일한 원본 데이터 하나에서 sorted / reverse / partial 생성 */
    generateData(base, DATA_COUNT);
    makeSorted(sortedD, base, DATA_COUNT);
    makeReverse(reverse, sortedD, DATA_COUNT);
    makePartial(partial, sortedD, DATA_COUNT, DATA_COUNT / PARTIAL_SWAP_DIV);

    int displaced = 0;                                /* 완전 정렬 상태와 Key1이 다른 위치 수 */
    for (int i = 0; i < DATA_COUNT; i++)
        if (partial[i].key1 != sortedD[i].key1) displaced++;

    char note[200];
    snprintf(note, sizeof(note),
             "(sorted data + %d random swaps; %d of %d positions (%.2f%%) differ from sorted order)",
             DATA_COUNT / PARTIAL_SWAP_DIV, displaced, DATA_COUNT,
             100.0 * displaced / DATA_COUNT);

    printSeparator();
    runInputCase("Sorted Data", NULL, sortedD, work, DATA_COUNT);
    printSeparator();
    runInputCase("Reverse-Sorted Data", NULL, reverse, work, DATA_COUNT);
    printSeparator();
    runInputCase("Partially Sorted Data", note, partial, work, DATA_COUNT);

    /* ---------------- 3. 안정성 검증 ---------------- */
    printSeparator();
    printf("Stability Verification\n\n");

    /* 시험 데이터 1: 과제에 제시된 예시 (527이 2개) */
    Data example[4] = {
        {527, 100, 0},
        {527, 200, 1},
        {125, 300, 2},
        {892, 400, 3}
    };
    const int exampleN = 4;

    /* 시험 데이터 2: Key1이 0~9뿐인 1000개 (중복 키가 매우 많음) */
    Data *dup = malloc(STAB_SMALL_COUNT * sizeof(Data));
    Data *dupWork = malloc(STAB_SMALL_COUNT * sizeof(Data));
    if (!dup || !dupWork) {
        printf("Memory allocation failed!\n");
        free(dup); free(dupWork);
        free(base); free(work); free(sortedD); free(reverse); free(partial);
        return 1;
    }
    for (int i = 0; i < STAB_SMALL_COUNT; i++) {
        dup[i].key1 = randomInt(STAB_SMALL_RANGE - 1);
        dup[i].key2 = randomInt(KEY_MAX);
        dup[i].originalIndex = i;
    }

    long long vEx[ALGO_COUNT], vDup[ALGO_COUNT];
    Stats dupStats[ALGO_COUNT];
    Data exampleResult[ALGO_COUNT][4];
    int sortedOk[ALGO_COUNT];

    for (int a = 0; a < ALGO_COUNT; a++) {
        Data ex[4];
        copyData(ex, example, exampleN);
        runAlgo((Algo)a, ex, exampleN);
        copyData(exampleResult[a], ex, exampleN);
        vEx[a] = stabilityViolations(ex, exampleN);

        copyData(dupWork, dup, STAB_SMALL_COUNT);
        dupStats[a] = runAlgo((Algo)a, dupWork, STAB_SMALL_COUNT);
        vDup[a] = stabilityViolations(dupWork, STAB_SMALL_COUNT);

        sortedOk[a] = isValidResult(ex, example, exampleN) &&
                      isValidResult(dupWork, dup, STAB_SMALL_COUNT);
    }

    /* 최종 판정 (Bubble Sort는 일반/조기 종료 두 방식 모두 안정해야 Stable) */
    int stable[ALGO_COUNT];
    for (int a = 0; a < ALGO_COUNT; a++)
        stable[a] = (vEx[a] == 0 && vDup[a] == 0 && sortedOk[a]);

    printf("%-20s : %s\n", "Selection Sort",
           stable[ALGO_SELECTION] ? "Stable" : "Unstable");
    printf("%-20s : %s\n", "Bubble Sort",
           (stable[ALGO_BUBBLE] && stable[ALGO_BUBBLE_EARLY]) ? "Stable" : "Unstable");
    printf("%-20s : %s\n", "Insertion Sort (>)",
           stable[ALGO_INSERTION_GT] ? "Stable" : "Unstable");
    printf("%-20s : %s\n", "Insertion Sort (>=)",
           stable[ALGO_INSERTION_GE] ? "Stable" : "Unstable");

    printf("\n[Details]\n");
    printf("Test 1 (example from the assignment, Key1 = 527 appears twice)\n");
    printf("  Original                : ");
    printSequence(example, exampleN);

    const char *detailNames[ALGO_COUNT] = {
        "Selection Sort", "Bubble Sort", "Bubble Sort (Early)",
        "Insertion Sort (>)", "Insertion Sort (>=)"
    };
    for (int a = 0; a < ALGO_COUNT; a++) {
        printf("  %-23s : ", detailNames[a]);
        printSequence(exampleResult[a], exampleN);
        printf("  %-23s   order violations among equal keys = %lld\n", "", vEx[a]);
    }

    printf("\nTest 2 (%d items, Key1 in 0..%d, many duplicates)\n",
           STAB_SMALL_COUNT, STAB_SMALL_RANGE - 1);
    for (int a = 0; a < ALGO_COUNT; a++) {
        printf("  %-23s : violations = %-4lld sorted+checksum OK = %s | Comparisons = %lld, Movements = %lld\n",
               detailNames[a], vDup[a], sortedOk[a] ? "Yes" : "No",
               dupStats[a].comparisons, dupStats[a].movements);
    }

    free(dup);
    free(dupWork);
    free(base);
    free(work);
    free(sortedD);
    free(reverse);
    free(partial);
    return 0;
}
