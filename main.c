#include <stdio.h>
#include <stdlib.h>
#include <ntsid.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include "header.h"

/* --- globals --- */
matrix_t * M;
pthread_mutex_t mutex;
pthread_cond_t condition;

int main(int argc, char ** argv) {
    pthread_t * threadIds;
    coordinate_t ** coordinates; /* contains rows and cols to be passed to threads */
    int * matrixDimensions;
    M = (matrix_t *) malloc(sizeof(matrix_t));
    if(!M) {
        mallocError();
    }

    /* --- init functions --- */
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condition, NULL);
    matrixDimensions = parseCommandLineParams(argc, argv);
    matrix_t_init(matrixDimensions);
    coordinates = (coordinate_t **) init2dArray(M->resultMatrixRows, M->resultMatrixCols, COORDINATE_T);
    threadIds = (pthread_t *) malloc(sizeof(pthread_t) * M->resultMatrixRows * M->resultMatrixCols);
    if(!threadIds) {
        mallocError();
    }

    printf("Current MAX_THREADS allotment = %d\n", MAX_THREADS);
    fillMatricesWithRandomNumbers();
    initResultMatrixToDefaults();
    spawnThreads(coordinates, threadIds);
    joinThreads(threadIds);

    printResults();

    cleanupAllData(coordinates, threadIds);

    return 0;
}


int * parseCommandLineParams(int argc, char ** argv) {
    int * result;
    int i;
    result = (int *) malloc(sizeof(int) * (LEGAL_ARGS_LEN - 1));
    if(!result) {
        mallocError();
    }
    /* if the user entered too many or too little arguments */
    if(argc != LEGAL_ARGS_LEN) {
        printf("Illegal parameter length.\nExiting the app.\n");
        exit(EXIT_FAILURE);
    }
    for(i = 1; i < LEGAL_ARGS_LEN; i++) {
        if(!isValidNumber(argv[i])) {
            printf("Illegal argument: arguments must be numbers\nExiting the app\n");
            exit(0);
        }
        result[i - 1] = atoi(argv[i]);
    }

    return result;
}

/*
    performs dot product on two vectors represented as one-dimensional arrays
     */
int multiplyVectors(int * a, int * b, int vecLen) {
    int i;
    int result = 0, count = 0;
    for(i = 0; i <= vecLen; i++) {
        result += a[i] * b[i];
        count++;
    }
    return result;
}

/*
receives a 2d array and returns a 1d array containing the values of the column requested.
When matrix multiplication is performed we're interested to present the columns of matrixB
as 1d array hence the name of the method.
*/
int * getMatrixBColumnAs1dArray(int column) {
    int i;
    int ** matrixB = M->B;
    int * resultArray = (int *) malloc(sizeof(int) * M->Brows);
    if(!resultArray) {
        mallocError();
    }
    for(i = 0; i < M->Brows; i++) {
        resultArray[i] = matrixB[i][column];
    }
    return resultArray;
}

void matrix_t_init(int * matrixDims) {
    M->Arows = matrixDims[0];
    M->Acols = matrixDims[1];
    M->Brows = M->Acols;
    M->Bcols = matrixDims[2];
    M->resultMatrixRows = M->Arows;
    M->resultMatrixCols = M->Bcols;

    M->maxThreads = MAX_THREADS;
    M->activeThreads = ZERO;

    M->A = (int **) init2dArray(M->Arows, M->Acols, INTEGER);
    M->B = (int **) init2dArray(M->Acols, M->Bcols, INTEGER);
    M->resultMatrix = (int **) init2dArray(M->Arows, M->Bcols, INTEGER);
    free(matrixDims);
}

/*
    fills matrixA and matrixB with random numbers within the valid range
     */
void fillMatricesWithRandomNumbers() {
    int i, j;
    srand(time(NULL));

    /* -------fill matrixA-------- */
    for(i = 0; i < M->Arows; i++) {
        for(j = 0; j < M->Acols; j++) {
            M->A[i][j] = rand() % UPPPER_BOUND_MATRIX_RANGE;
        }
    }

    /* -------fill matrixB-------- */
    for(i = 0; i < M->Acols; i++) {
        for(j = 0; j < M->Bcols; j++) {
            M->B[i][j] = rand() % UPPPER_BOUND_MATRIX_RANGE;
        }
    }
}

/*
 * prints matrix in Wolfram Alpha-friendly format so that the matrix multiplication result
 * can be quickly checked there
 */
void printMatrix(int ** matrix, int rows, int cols) {
    int row, col;
    printf("{");
    for(row = 0; row < rows; row++) {
        for(col = 0; col < cols; col++) {
            if(col == 0) {
                printf("{");
            }
            printf("%d", matrix[row][col]);
            if(col != cols - 1) {
                printf(",");
            }
        }
        printf("}");
        if(row == rows - 1) {
            printf("}");
        } else {
            printf(",\n");
        }
    }
    printf("\n");
}

void initResultMatrixToDefaults() {
    int i, j;
    for(i = 0; i < M->resultMatrixRows; i++) {
        for(j = 0; j < M->resultMatrixCols; j++) {
            M->resultMatrix[i][j] = DEFAULT_CELL_VALUE;
        }
    }
}

void mallocError() {
    printf("Error: malloc - couldn't allocate memory.\nExiting the app.\n");
    exit(0);
}

/*
 * only MAX_THREADS threads are allowed to be active at any given time. If the limit was reached we wait.
 */
void checkIfNeedToWait() {
    pthread_mutex_lock(&mutex);

    while(M->activeThreads == MAX_THREADS) {
        pthread_cond_wait(&condition, &mutex);
    }
    M->activeThreads++;
    printf("active threads = %d\n", M->activeThreads);
    pthread_mutex_unlock(&mutex);
}

void * WorkerThread(void * arg) {
    coordinate_t * coord = (coordinate_t *) arg;
    int * matrixBColumnAs1dArray = getMatrixBColumnAs1dArray(coord->col);
    M->resultMatrix[coord->row][coord->col] = multiplyVectors(M->A[coord->row],
                                                              matrixBColumnAs1dArray,
                                                              M->Bcols);
    free(matrixBColumnAs1dArray);
    sleep(1); /* simulate heavy work */
    pthread_mutex_lock(&mutex);
    M->activeThreads--;
    pthread_cond_broadcast(&condition);
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void spawnThreads(coordinate_t **coord, pthread_t * threadIds) {
    int row, col, count = 0;
    for(row = 0; row < M->resultMatrixRows; row++) {
        for(col = 0; col < M->resultMatrixCols; col++) {
            checkIfNeedToWait();
            if(pthread_create(&threadIds[count], NULL, WorkerThread, (void *) &coord[row][col])) {
                perror("Error: ");
                printf("Exiting the app\n");
                exit(EXIT_FAILURE);
            }
            count++;
        }
    }
}

void ** init2dArray(int rows, int cols, enum DataType type) {
    int i, row, col, ** arr;
    coordinate_t ** coord;
    if(type == INTEGER) {
        arr = (int **) malloc(rows * sizeof(int *));
        for (i = 0; i < rows; i++)
            arr[i] = (int *) malloc(cols * sizeof(int));
        return (void **) arr;
    } else {
        coord = (coordinate_t **) malloc(sizeof(coordinate_t *) * M->resultMatrixRows);
        if(!coord) {
            mallocError();
        }
        for(row = 0; row < M->resultMatrixRows; row++) {
            coord[row] = (coordinate_t *) malloc(sizeof(coordinate_t) * M->resultMatrixCols);
            if(!coord[row]) {
                mallocError();
            }
        }
        for(row = 0; row < M->resultMatrixRows; row++) {
            for(col = 0; col < M->resultMatrixCols; col++) {
                coord[row][col].row = row;
                coord[row][col].col = col;
            }
        }
        return (void **) coord;
    }
}

void joinThreads(pthread_t * threadIds) {
    int row, col, count = 0;
    for(row = 0; row < M->resultMatrixRows; row++) {
        for(col = 0; col < M->resultMatrixCols; col++){
            if(pthread_join(threadIds[count], NULL)) {
                perror("Error: ");
                printf("Exiting the app\n");
                exit(EXIT_FAILURE);
            }
            count++;
        }
    }
}

void printResults() {
    printf("\n");
    printf("matrix A:\n");
    printMatrix(M->A, M->Arows, M->Acols);
    printf("\n");
    printf("matrix B:\n");
    printMatrix(M->B, M->Acols, M->Bcols);
    printf("\n");
    printf("result matrix:\n");
    printMatrix(M->resultMatrix, M->resultMatrixRows, M->resultMatrixCols);
    printf("\n");
}

int isValidNumber(char * str) {
    char * c;
    c = str;
    while(*c != END_OF_STRING_CHAR) {
        if(!isdigit(*c)) {
            return FALSE;
        }
        c++;
    }
    return TRUE;
}

void destroyMatrix_t(matrix_t * m) {
    free(m->A);
    free(m->B);
    free(m->resultMatrix);
    free(m);
}

void cleanupAllData(coordinate_t ** coordinates, pthread_t * threadIds) {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);
    free(coordinates);
    destroyMatrix_t(M);
    free(threadIds);
}
