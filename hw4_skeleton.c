#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <malloc.h>

#include <pthread.h>

void Display(int dim, float *mat);
void Fill(int size, float *data);
void MatMul(int dim, float *A, float *B, float *Product);
void MatMul_MT(int dim, float *A, float *B, float *Product, int noThread);
void* MatMul_ThreadFn(void *param);

int main(int argc, char *argv[])
{
	int dim = 10;
	int noThread = 10;

	struct timeval t1, t2;
	float elapsed_time = 0.F;

	if(argc < 2){
		printf("Usage: %s <dim> <no_threads>\n", argv[0]);
		exit(0);
	}

//max dim 1k
	dim = atoi(argv[1]);
	if(dim > 1000)
		dim = 1000;
	noThread = atoi(argv[2]);

	srand(time(NULL));

	float *A = (float*)malloc(dim * dim * sizeof(float));
	float *B = (float*)malloc(dim * dim * sizeof(float));
	float *Product = (float*)malloc(dim * dim * sizeof(float));

	if(A == NULL || B == NULL || Product == NULL){
		printf("Failed to allocate memory.\n");
		exit(-1);
	}

	Fill(dim * dim, A);
	Fill(dim * dim, B);

	if(dim <= 10){
		printf("A = \n");
		Display(dim, A);

		printf("B = \n");
		Display(dim, B);
	}

	gettimeofday(&t1, NULL);

	MatMul(dim, A, B, Product);

	gettimeofday(&t2, NULL);
	elapsed_time = (t2.tv_sec + t2.tv_usec / 1000000.) - (t1.tv_sec + t1.tv_usec / 1000000.);
	printf("elapsed time (single thread)\t= %15f sec\n", elapsed_time);

	if(dim <= 10){
		printf("A * B (single thread) = \n");
		Display(dim, Product);
	}

	gettimeofday(&t1, NULL);

	MatMul_MT(dim, A, B, Product, noThread);

	gettimeofday(&t2, NULL);
	elapsed_time = (t2.tv_sec + t2.tv_usec / 1000000.) - (t1.tv_sec + t1.tv_usec / 1000000.);
	printf("elapsed time (%d threads)\t= %15f sec\n", noThread, elapsed_time);

	if(dim <= 10){
		printf("A * B (%d threads) = \n", noThread);
		Display(dim, Product);
	}


	free(A);
	free(B);
	free(Product);

	return 0;
}

void Display(int dim, float *mat)
{
	if(dim >= 10)
		return;
	for(int i = 0; i < dim; i++){
		for(int j = 0; j < dim; j++)
			printf("%.2f ", mat[i * dim + j]);
		printf("\n");
	}
}

void Fill(int size, float *data)
{
	int i = 0;
	for(i = 0; i < size; i++)
		data[i] = i;
}

void MatMul(int dim, float *A, float *B, float *Product)
{
	int i = 0, j = 0, k = 0;

	for(i = 0; i < dim; i++){
		for(j = 0; j < dim; j++){
			Product[i * dim + j] = 0.F;
			for(k = 0; k < dim; k++)
				Product[i * dim + j] += A[i * dim + k] * B[k * dim + j]; 
		}
	}
}

typedef struct {
	int id;					// thread index
	int noThread;
	int dim;
	float *A, *B, *Product;
} ThreadInfo;


void MatMul_MT(int dim, float *A, float *B, float *Product, int noThread)
{
    pthread_t *th_container;
    ThreadInfo *threadInfo;
    threadInfo = malloc(sizeof(ThreadInfo)*noThread);
    th_container = malloc(sizeof(pthread_t)*noThread);
    
    int i;
    for (i = 0; i < noThread; i++)
    {
        threadInfo[i].id = i;
        threadInfo[i].noThread = noThread;
        threadInfo[i].dim = dim;
        threadInfo[i].A = A;
        threadInfo[i].B = B;
        threadInfo[i].Product = Product;
        if (pthread_create(&th_container[i], NULL, MatMul_ThreadFn, &threadInfo[i]))
        {
            printf("Failed to create thread: %d\n",i);
        }
    }


    for (i = 0; i < noThread; i++)
    {
        if (pthread_join(th_container[i], NULL))
        {
            printf("Error joining threads:%d",i);
        }
    }

    free(th_container);
    free(threadInfo);
	// dynamically allocate pthread_t-type array whose length is noThread
	// dynamically allocate ThreadInfo-type array whose length is noThread
	
	// Repeat for noThread times
		// Fill threadInfo[i] with appropriate values
		//	ex) threadInfo[i].id = i;
		// 		threadInfo[i].noThread = noThread;
		//		...
		//
		// create child thread passing &aThreadInfo[i] to the thread function
		

	// wait for the children threads to terminate
		// call pthread_join t-times

	// deallocate pthread_t and ThreadInfo array
}

void* MatMul_ThreadFn(void *param)		// thread function
{
	ThreadInfo *pInfo = (ThreadInfo *)param;
	
    int th_id = pInfo->id;
    int th_noThread = pInfo->noThread;
    int th_dim = pInfo->dim / th_noThread; 
    int remainder = pInfo->dim % th_noThread;
    int i, k, j; 
    int th_rem = 0;

    if (th_id == th_noThread-1)
    {
       th_rem = remainder; 
    }
    for (i = th_id*th_dim; i < ((th_id + 1)*th_dim + th_rem); i++)
    {
        for (j = th_id*th_dim; j < pInfo->dim; j++)//((th_id + 1)*th_dim + th_rem); j++)
        {
            for (k = th_id*th_dim; k < pInfo->dim; k++)//((th_id + 1)*th_dim + th_rem); k++)
            {
                pInfo->Product[i*th_dim +j] += pInfo->A[i*th_dim +k] * pInfo->B[k*th_dim +j];
            }
        }
    }

	// multiply two matrics pInfo->A and pInfo->B put the Product in pInfo->Product
	// i-th thread computes only (k + i * noThread)-th rows
}
