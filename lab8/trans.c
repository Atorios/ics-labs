/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 *
 * Student Name: 黎君
 * Student ID: 516030910233
 *
 */ 
#include <stdio.h>
#include "cachelab.h"

#define MIN(x, y) ((x) > (y)? (y) : (x))

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans_f3(int M, int N, int A[N][M], int B[M][N]);
void trans_f5(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 64 && N == 64) {
        trans_f3(M, N, A, B);
    } else {
        trans_f5(M, N, A, B);
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

char f1_desc[] = "ii jj";
void trans_f1(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, ii, jj, tmp;
    int bsize = 8;
    int en = bsize * (MIN(N, M) / bsize);

    for (ii = 0; ii < en; ii += bsize) {
        for (jj = 0; jj < en; jj += bsize) {
            for (i = ii; i < ii + bsize; i++) {
                for (j = jj; j < jj + bsize; j++) {
                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
            }
        }
    }

    for (i = en; i < N; i++) {
        for (j = en; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = 0; i < en; i++) {
        for (j = en; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = en; i < N; i++) {
        for (j = 0; j < en; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

char f2_desc[] = "diag";
void trans_f2(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, ii, jj, tmp, diag;
    int bsize = 8;
    int en = bsize * (MIN(N, M) / bsize);

    for (ii = 0; ii < en; ii += bsize) {
        for (jj = 0; jj < en; jj += bsize) {
            for (i = ii; i < ii + bsize; i++) {
                for (j = jj; j < jj + bsize; j++) {
                    if (ii == jj && i == j) {
                        diag = A[i][j];
                        continue;
                    }
                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
                if (ii == jj) B[i][i] = diag;
            }
        }
    }

    for (i = en; i < N; i++) {
        for (j = en; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = 0; i < en; i++) {
        for (j = en; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = en; i < N; i++) {
        for (j = 0; j < en; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

char f3_desc[] = "4 partition";
void trans_f3(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, ii, jj, tmp, diag;
    int bsize = 8;
    int flag = (M == 64 ? 1 : 0);
    int en = bsize * (MIN(N, M) / bsize);

    for (ii = 0; ii < en; ii += bsize) {
        for (jj = 0; jj < en; jj += bsize) {
            /* left up */
            for (i = ii; i < ii + bsize / 2; i++) {
                for (j = jj; j < jj + bsize / 2; j++) {
                    if (i == j) {
                        diag = A[i][j];
                        continue;
                    }
                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
                if (ii == jj) B[i][i] = diag;
            }
            /* right up */
            for (i = ii; i < ii + bsize / 2; i++) {
                for (j = jj + bsize / 2; j < jj + bsize; j++) {
                    if (flag && ii == jj && i + 4 == j) {
                        diag = A[i][j];
                        continue;
                    }
                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
                if (ii == jj) B[i+4][i] = diag;
            }
            /* right down */
            for (i = ii + bsize / 2; i < ii + bsize; i++) {
                for (j = jj + bsize / 2; j < jj + bsize; j++) {
                    if (i == j) {
                        diag = A[i][j];
                        continue;
                    }
                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
                if (ii == jj) B[i][i] = diag;
            }
            /* left down */
            for (i = ii + bsize / 2; i < ii + bsize; i++) {
                for (j = jj; j < jj + bsize / 2; j++) {
                    if (flag && ii == jj && i - 4 == j) {
                        diag = A[i][j];
                        continue;
                    }
                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
                if (ii == jj) B[i-4][i] = diag;
            }
        }
    }

    for (i = en; i < N; i++) {
        for (j = en; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = 0; i < en; i++) {
        for (j = en; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = en; i < N; i++) {
        for (j = 0; j < en; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

char f4_desc[] = "loop unrolling";
void trans_f4(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, ii, jj, tmp, tmp1, tmp2, tmp3, tmp4;
    int bsize = 8;
    int en = bsize * (MIN(N, M) / bsize);

    for (ii = 0; ii < en; ii += bsize) {
        for (jj = 0; jj < en; jj += bsize) {
            /* left up */
            j = jj;
            for (i = ii; i < ii + bsize / 2; i++) {
                tmp1 = A[i][j];
                tmp2 = A[i][j+1];
                tmp3 = A[i][j+2];
                tmp4 = A[i][j+3];
                B[j][i] = tmp1;
                B[j+1][i] = tmp2;
                B[j+2][i] = tmp3;
                B[j+3][i] = tmp4;
            }
            /* right up */
            j = jj + bsize / 2;
            for (i = ii; i < ii + bsize / 2; i++) {
                tmp1 = A[i][j];
                tmp2 = A[i][j+1];
                tmp3 = A[i][j+2];
                tmp4 = A[i][j+3];
                B[j][i] = tmp1;
                B[j+1][i] = tmp2;
                B[j+2][i] = tmp3;
                B[j+3][i] = tmp4;
            }
            /* right down */
            j = jj + bsize / 2;
            for (i = ii + bsize / 2; i < ii + bsize; i++) {
                tmp1 = A[i][j];
                tmp2 = A[i][j+1];
                tmp3 = A[i][j+2];
                tmp4 = A[i][j+3];
                B[j][i] = tmp1;
                B[j+1][i] = tmp2;
                B[j+2][i] = tmp3;
                B[j+3][i] = tmp4;
            }
            /* left down */
            j = jj;
            for (i = ii + bsize / 2; i < ii + bsize; i++) {
                tmp1 = A[i][j];
                tmp2 = A[i][j+1];
                tmp3 = A[i][j+2];
                tmp4 = A[i][j+3];
                B[j][i] = tmp1;
                B[j+1][i] = tmp2;
                B[j+2][i] = tmp3;
                B[j+3][i] = tmp4;
            }
        }
    }

    for (i = en; i < N; i++) {
        for (j = en; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = 0; i < en; i++) {
        for (j = en; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = en; i < N; i++) {
        for (j = 0; j < en; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

char f5_desc[] = "8 temp";
void trans_f5(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    int n = N / 8 * 8;
    int m = M / 8 * 8;
    for (j = 0; j < m; j += 8) {
        for (i = 0; i < n; i++) {
            tmp = A[i][j];
            tmp1 = A[i][j+1];
            tmp2 = A[i][j+2];
            tmp3 = A[i][j+3];
            tmp4 = A[i][j+4];
            tmp5 = A[i][j+5];
            tmp6 = A[i][j+6];
            tmp7 = A[i][j+7];

            B[j][i] = tmp;
            B[j+1][i] = tmp1;
            B[j+2][i] = tmp2;
            B[j+3][i] = tmp3;
            B[j+4][i] = tmp4;
            B[j+5][i] = tmp5;
            B[j+6][i] = tmp6;
            B[j+7][i] = tmp7;

        }   
    } 

    for (i = n; i < N; i++) {
        for (j = m; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = 0; i < n; i++) {
        for (j = m; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = n; i < N; i++) {
        for (j = 0; j < m; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

char f6_desc[] = "4 partition and restoration";
void trans_f6(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, ii, jj, tmp, diag;
    int bsize = 8;
    int en = bsize * (MIN(N, M) / bsize);

    for (ii = 0; ii < en; ii += bsize) {
        for (jj = 0; jj < en; jj += bsize) {
            /* left up */
            for (i = ii; i < ii + bsize / 2; i++) {
                for (j = jj; j < jj + bsize / 2; j++) {
                    if (i == j) {
                        diag = A[i][j];
                        continue;
                    }
                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
                if (ii == jj) B[i][i] = diag;
            }
            /* right up */
            for (i = ii; i < ii + bsize / 2; i++) {
                for (j = jj + bsize / 2; j < jj + bsize; j++) {
                    tmp = A[i][j];
                    B[j-4][i+4] = tmp;
                }
            }
            /* right down */
            for (i = ii + bsize / 2; i < ii + bsize; i++) {
                for (j = jj + bsize / 2; j < jj + bsize; j++) {
                    if (i == j) {
                        diag = A[i][j];
                        continue;
                    }
                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
                if (ii == jj) B[i][i] = diag;
            }
            /* left down */
            for (i = ii + bsize / 2; i < ii + bsize; i++) {
                for (j = jj; j < jj + bsize / 2; j++) {
                    tmp = A[i][j];
                    B[j+4][i-4] = tmp;
                }
            }
            /* restore */
            for (i = ii; i < ii+4; i++) {
                for (j = jj+4; j < jj+8; j++) {
                    tmp = B[i][j];
                    B[i+4][j-4] = tmp;
                    B[i][j] = B[i+4][j-4];
                }
            }
        }
    }

    for (i = en; i < N; i++) {
        for (j = en; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = 0; i < en; i++) {
        for (j = en; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }

    for (i = en; i < N; i++) {
        for (j = 0; j < en; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

    // registerTransFunction(trans_f1, f1_desc);
    registerTransFunction(trans_f2, f2_desc);
    registerTransFunction(trans_f3, f3_desc);
    registerTransFunction(trans_f4, f4_desc);
    // registerTransFunction(trans_f5, f5_desc);
    //registerTransFunction(trans_f6, f6_desc);

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

