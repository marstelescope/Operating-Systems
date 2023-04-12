/*
Multithreaded solution to find if a given number is a perfect number. 
N is a perfect number if the sum of all its factors, excluding itself, is N; 
examples are 6, 28, and 496. The main program will read the numbers N and P. 
The main process will spawn a set of P threads. The numbers from 1 to N will 
be partitioned among these threads so that two threads do not work on the same 
number. For each number in this set, the thread will determine if the number
is a factor of N and add it to a global sum. The parent will then determine if
the input number is perfect (if N is a sum of all its factors) and report accordingly. 
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#define maxthread 100

int N = 1, P = 1;
int startindex[maxthread] = {0};
int A[maxthread] = {0};
int sum = 0;
pthread_mutex_t mylock;
int stride;  


void setstartindex(){
    int startval = 1;
    for (int i = 0; i < P; i++){ 
        startindex[i] = startval;
        startval = startval + stride;  
    }
}

// thread index 
void setthindex(int x){
    for (int i = 0; i < x; i++){
        A[i] = i;
    }
}

// each thread checks for factors in their assigned range and adds to sum
void * perfectnumber (void * idx){
    int id = *(int *)idx;
    for (int i = startindex[id]; i < startindex[id] + stride; i++){
        if (N%i == 0){
            pthread_mutex_lock(&mylock);
                sum += i;
            pthread_mutex_unlock(&mylock);
        }
    }
pthread_exit(NULL);
}

int main (){
    pthread_mutex_init(&mylock, NULL);

    printf("Enter numbers N and P: ");
    scanf("%d %d", &N, &P);

    while (P > 100 || P > N){
        printf("Re-enter P, must be <= 100 and <= N: ");
        scanf("%d", &P);
    }
    
    if (P > N/2){ 
        // removing excess threads, adjusting stride
        P = N/2;
        stride = 1;
    }
    else{
        // N divided by 2 for efficiency of factor checking
        stride = (N/2)/P; 
    }
   
    pthread_t *th = calloc(sizeof(pthread_t), P);
    setstartindex();
    setthindex(P);

    for (int i = 0; i < P; i++){
        pthread_create(&th[i], NULL, perfectnumber, (void *)&A[i]);
    }
    for (int i = 0; i < P; i++){
        pthread_join(th[i], NULL);
    }

    // if pthread strides don't reach N/2, we check missing elements and add those that qualify
    if (N > stride*P*2){
        for (int i = stride*P+1; i <= N/2; i++){
            if (N%i == 0){
                sum+= i;
            }
        }
    }

    if (N == sum){ printf("%d is a perfect number\n", N);} 
    else{ printf("%d is a not perfect number\n", N); }

    free(th);
    pthread_mutex_destroy(&mylock);

    return 0;
}