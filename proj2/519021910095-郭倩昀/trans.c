//Guo Qianyun 519021910095
/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"
#define PARTSIZE 17
int is_transpose(int M, int N, int A[N][M], int B[M][N]);

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
	if(M == 32 && N == 32)
	{
		int t0, t1, t2, t3, t4, t5, t6, t7;
		for(int i = 0; i < N; i += 8)
		{
			for(int j = 0; j < M; j +=8)
			{
				for(int a = i; a < i+8; a++)
				{
					int b = j;
					// 8 integer from a line in A
					t0 = A[a][b];
					t1 = A[a][b+1];
					t2 = A[a][b+2];
					t3 = A[a][b+3];
					t4 = A[a][b+4];
					t5 = A[a][b+5];
					t6 = A[a][b+6];
					t7 = A[a][b+7];
					//put to B
					B[b][a] = t0;
					B[b+1][a] = t1;
					B[b+2][a] = t2;
					B[b+3][a] = t3;
					B[b+4][a] = t4;
					B[b+5][a] = t5;
					B[b+6][a] = t6;
					B[b+7][a] = t7;
				}
			}	
		}
	}
	else if(M == 64 && N == 64)
	{
		int t0, t1, t2, t3, t4, t5, t6, t7;
		for(int i = 0; i < N; i += 8)
		{
			for(int j = 0; j < M; j +=8)
			{
				for(int a = i; a < i+4; a++)
				{
					int b = j;
					
					t0 = A[a][b];
					t1 = A[a][b+1];
					t2 = A[a][b+2];
					t3 = A[a][b+3];
					t4 = A[a][b+4];
					t5 = A[a][b+5];
					t6 = A[a][b+6];
					t7 = A[a][b+7];
					
					//up_left part of A to up_left part of B
					B[b][a] = t0;
					B[b+1][a] = t1;
					B[b+2][a] = t2;
					B[b+3][a] = t3;
					
					//temporarily up_right part of A to up_right part of B
					B[b][a+4] = t4;
					B[b+1][a+4] = t5;
					B[b+2][a+4] = t6;
					B[b+3][a+4] = t7;
				}
				for(int b = j; b < j+4; b++)
				{
					int a = i;
					t0 = A[a+4][b];
					t1 = A[a+5][b];
					t2 = A[a+6][b];
					t3 = A[a+7][b];
					
					t4 = B[b][a+4];
					t5 = B[b][a+5];
					t6 = B[b][a+6];
					t7 = B[b][a+7];
					
					//down_left part of A to up_right part of B
					B[b][a+4] = t0;
					B[b][a+5] = t1;
					B[b][a+6] = t2;
					B[b][a+7] = t3;
					
					//the temporary up_right part of B move to down_left part of B
					B[b+4][a] = t4;
					B[b+4][a+1] = t5;
					B[b+4][a+2] = t6;
					B[b+4][a+3] = t7;
				}
				for(int a = i+4; a<i+8; a++)
				{
					int b = j+4;
					t0 = A[a][b];
					t1 = A[a][b+1];
					t2 = A[a][b+2];
					t3 = A[a][b+3];
					
					//down_right part of A to down_right part of B
					B[b][a] = t0;
					B[b+1][a] = t1;
					B[b+2][a] = t2;
					B[b+3][a] = t3;
				}
			}
		}
	}
	else if(M == 61 && N == 67)
	{
		for(int i = 0; i < N; i+=PARTSIZE)
		{
			for(int j = 0; j < M; j+=PARTSIZE)
			{
				for(int a = i; a < i+PARTSIZE && a < N; a++)
				{
					for(int b = j; b < j+PARTSIZE && b < M; b++)
					{
						B[b][a] = A[a][b];
					}
				}
			}
		}
	}
	else
	{
	    int i, j, tmp;
		for (i = 0; i < N; i++) {
			for (j = 0; j < M; j++) {
				tmp = A[i][j];
				B[j][i] = tmp;
			}
		}   
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

