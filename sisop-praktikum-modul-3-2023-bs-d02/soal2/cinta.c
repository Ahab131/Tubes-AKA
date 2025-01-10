#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

unsigned long long int facto;

void *calc_facto(void *arg) {
    int num = *((int *)arg);
    facto = 1;
    
    for (int i=1; i<=num; i++) {
        facto *= i;
    }
    return (void *)facto;
}

int main() {
    key_t key = 1234;
    
    int shmid = shmget(key, sizeof(int), 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    int *hasil_mrx = (int *)shmat(shmid, NULL, 0);
    if (hasil_mrx == (int *)-1) {
        perror("shmat");
        exit(1);
    }

    printf("Hasil perkalian matriks dari program kalian.c:\n");
    printf("[");
    for (int i=0; i<4; i++) {
        printf("[");
        for (int j=0; j<5; j++) {
            printf("%d", *(hasil_mrx + i*5 + j));
            if (j < 5-1) {
                printf(", ");
            }
        }
        printf("]");
        if (i < 4-1) {
            printf(",\n");
        }
    }
    printf("]\n");

    pthread_t tid[4*5];
    int index = 0;
    for (int i=0; i<4; i++) {
        for (int j=0; j<5; j++) {
            int *arg = (int *)malloc(sizeof(*arg));
            *arg = *(hasil_mrx + i*5 + j);
            pthread_create(&tid[index], NULL, calc_facto, (void *)arg);
            index++;
        }
    }

    printf("\nHasil faktorial:\n");
    printf("[");
    index = 0;
    
    for (int i=0; i<4; i++) {
        printf("[");
        
        for (int j=0; j<5; j++) {
            pthread_join(tid[index], (void *)&facto);
            printf("%llu", facto);
            
            if (j < 5-1) {
                printf(", ");
            }
            index++;
        }
        printf("]");
        
        if (i < 4-1) {
            printf(", \n");
        }
    }
    printf("]\n");

    return 0;
}