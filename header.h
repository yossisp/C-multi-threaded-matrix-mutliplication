/*
// Created by Yossi Spektor
*/

#ifndef MY_HEADER
# define MY_HEADER


typedef struct matrix_t {
    int ** resultMatrix;
    int resultMatrixRows;
    int resultMatrixCols;
    int ** A; /* matrix A */
    int Arows;
    int Acols;
    int ** B; /* matrix B */
    int Brows;
    int Bcols;
    int maxThreads;
    int activeThreads;
} matrix_t;

typedef struct coordinate_t {
    int row;
    int col;
} coordinate_t;

enum DataType {INTEGER, COORDINATE_T};

#define TRUE  1
#define FALSE 0
#define ZERO  0
#define MAX_THREADS  5
#define DEFAULT_CELL_VALUE  -1
#define UPPPER_BOUND_MATRIX_RANGE  20
#define LEGAL_ARGS_LEN  4
#define NOT_NUMBER 0
#define END_OF_STRING_CHAR '\0'

/* --- prototypes --- */
int * parseCommandLineParams(int argc, char ** argv);
void ** init2dArray(int rows, int cols, enum DataType type);
void fillMatricesWithRandomNumbers();
void matrix_t_init(int * matrixDims);
void printMatrix(int ** matrix, int rows, int cols);
void initResultMatrixToDefaults();
int * getMatrixBColumnAs1dArray(int column);
int multiplyVectors(int * a, int * b, int vecLen);
void mallocError();
void checkIfNeedToWait();
void * WorkerThread(void * arg);
void spawnThreads(coordinate_t **coord, pthread_t *threadIds);
void joinThreads(pthread_t * threadIds);
void printResults();
int isValidNumber(char * str);
void destroyMatrix_t(matrix_t * m);
void cleanupAllData(coordinate_t ** coordinates, pthread_t * threadIds);

#endif
