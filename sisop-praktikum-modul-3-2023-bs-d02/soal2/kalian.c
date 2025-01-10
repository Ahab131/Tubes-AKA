#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/*
-----------------------------------------------------------------------------------------------------------------------------
  program kalian.c berjalan melakukan 2 perkalian matriks (4x2) dan (2x5) dengan generate isi matriks random sesuai aturan  	
-----------------------------------------------------------------------------------------------------------------------------
*/


int main() {
    
    	// random generate buat isi matriks
	srand(time(NULL));
	
	int mrx1[4][2]; // matriks 1 4x2
	int mrx2[2][5]; // matriks 2 2x5
	
    	int hasil_mrx[4][5] = {0}; // inisiasi awal 0 untuk hasil matriks
	
	/* 
	  for loop untuk generate angka random matriks
	*/
         
        // matriks 1
	for (int i = 0; i < 4; i++) {
	  for (int j = 0; j < 2; j++) {
	    mrx1[i][j] = rand() % 5 + 1;
	  }
	}
	
	// matriks 2 
	for (int i = 0; i < 2; i++) {
	  for (int j = 0; j < 5; j++) {
	    mrx2[i][j] = rand() % 4 + 1;
	  }
	}
	
	// perhitungan perkalian kedua matriks
	for (int i = 0; i < 4; i++) {
	  for (int j = 0; j < 5; j++) {
	    for (int k = 0; k < 2; k++) {
	      hasil_mrx[i][j] += mrx1[i][k] * mrx2[k][j];
	    }
	  }  
	}
	
	// buat segment shm (key, id)
	key_t keyShm = 1234;
	int shmid = shmget(keyShm, sizeof(hasil_mrx), IPC_CREAT | 0666);
	if (shmid == -1) {
		perror("error shmget");
		return 1;
	}
	
	// attach / connect shm ke var hasil_mrx
	int (*shmhasil_mrx)[5] = shmat(shmid, NULL, 0);
	if (shmhasil_mrx == (void *) -1) {
		perror("error shmat");
		return 1;
	}
	
	// salin hasil_mrx ke shm
	for (int i = 0; i < 4; i++) {
	  for (int j = 0; j < 5; j++) {
	    shmhasil_mrx[i][j] = hasil_mrx[i][j];
	  }
	}   
	
	
    // get data - hasil matriks	
	printf("Hasil matrix kalian.c adalah :\n");
    
	for (int i = 0; i < 4; i++) {
	  for (int j = 0; j < 5; j++) {
	    printf("%d ", hasil_mrx[i][j]);
	  }
	    printf("\n");
	}
    
	// detach / end shm yang berisi hslmrx
	shmdt(shmhasil_mrx);
	
	return 0;
}	    
