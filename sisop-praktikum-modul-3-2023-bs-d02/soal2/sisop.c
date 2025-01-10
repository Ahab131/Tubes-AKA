#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

unsigned long long int facto;

unsigned long long int calc_facto(int num) {
    facto = 1;
    
    for (int i=1; i<=num; i++) {
        facto *= i;
    }
    return facto;
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
            printf("%d", *(hasil_mrx + i * 5 + j));
            
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

    printf("\nHasil faktorial:\n");
    printf("[");
    
    for (int i=0; i<4; i++) {
        printf("[");
        
        for (int j=0; j<5; j++) {
            facto = hitung_faktorial(*(hasil_mrx + i*5 + j));
            printf("%llu", facto);
            
            if(j < 5-1){
                printf(", ");
            }
        }
        printf("]");
        
        if(i < 4-1){
            printf(", \n");
        }
    }
    printf("]\n");

    return 0;
}