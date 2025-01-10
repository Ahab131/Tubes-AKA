# Lapres Praktikum Modul 3 Sisop
- Abdul Hakim Al Baihaqy (1201230031)
- Aditya Bayu Pradana (1201230036)
- Raihan Aryo Wicaksono (1201230010)
- Yafet Seno Armadanto (1201230005)
## Nomor 1
Program pada soal 1 melakukan kompresi file menggunakan algoritma huffman. Algoritma huffman sendiri merupakan algoritma untuk mengurangi ukuran file (jumlah bit) berdasarkan frekuensi kemunculan huruf. Huruf dengan frekuensi kemunculan tertinggi dikodekan dengan bit terpendek. Tugas ini dikerjakan dengan 2 proses yang saling berkomunikasi menggunakan pipes. Alur jalannya program ini adalah sebagai berikut:
1. Parent process membaca file yang akan dikompresi dan menghitung frekuensi kemunculan setiap huruf. Hasil akan dikirim ke child process dengan pipes.
2. Child process membaca pesan yang dikirim parent process. Pesan tersebut digunakan untuk menjalankan algoritma huffman sehingga menghasilkan huffman tree, kode untuk tiap huruf, dan kode huffman file. Hasil kemudian dikirim kembali ke parent process dengan pipes.
3. Parent process membaca huffman tree dan kode huffman yang telah dikirim lalu melakukan dekompresi, yaitu mengubah kode huffman ke huruf aslinya. Selanjutnya, parent process menghitung jumlah bit sebelum dan sesudah kompresi file untuk perbandingan.

### Solusi
#### ***Pipes***
Program menggunakan 2 pipe, pipe pertama untuk mengirim file dan frekuensi huruf dari parent process, pipe kedua untuk mengirim hasil huffman code dari child process. Pipes dapat dibuat dengan kode berikut:
``` R
int fd1[2];
int fd2[2];

pid_t id;

if (pipe(fd1) == -1)
{
    fprintf(stderr, "pipe failed");
    return 1;
}
if (pipe(fd2) == -1)
{
    fprintf(stderr, "pipe failed");
    return 1;
}
```
Penjelasan pembuatan pipe:
1. Deklarasikan variabel array integer dengan 2 elemen, indeks "0" untuk membaca dan indeks "1" untuk menulis.
2. Gunakan fungsi pipe() untuk membuat pipes
3. Jika pipes gagal dibuat (return fungsi pipe() sama dengan -1), kirim pesan error dan terminate program

#### ***Membaca file dan menghitung frekuensi huruf***
Setelah berhasil membuat pipes, lakukan spawning process menggunakan fork(). Pada parent process, terlebih dahulu dilakukan penutupan read end pada pipe pertama dengan `close(fd1[0])`. Selanjutnya, parent process membaca file yang akan dikompresi dan menghitung frekuensi tiap huruf yang muncul. Hal tersebut dilakukan dengan kode berikut:
```R
FILE *fptr;
fptr = fopen("/home/thoriqaafif/sisop/file.txt", "r");

char c;
char file[1000];
int freq[26], count = 0, jumlah_huruf = 0;
memset(freq, 0, 26 * sizeof(int));

while (fscanf(fptr, "%c", &c) == 1)
{
    c = toupper(c);
    file[count] = c;
    count++;

    if (c >= 'A' && c <= 'Z')
    {
        freq[c - 'A'] += 1;
        jumlah_huruf++;
    }
}
fclose(fptr);
```
Pembacaan file dilakukan menggunakan perulangan fungsi fscanf() karakter per karakter. Penghitungan frekuensi kemunculan huruf dilakukan sekaligus pada perulangan tersebut. Alur detail sebagai berikut:
1. buka file.txt dengan mode 'r' (read) menggunakan `fopen()`
2. Deklarasikan variabel char c, string file, dan integer array freq dengan masing-masing untuk menyimpan karakter yang sedang dibaca, isi file, dan frekuensi kemunculan tiap huruf. Array freq berukuran 26 karena terdapat 26 huruf.
3. Set semua kemunculan huruf dengan 0 menggunakan `memset()`
4. Lakukan perulangan hingga tidak terdapat karakter yang tersisa pada file
    1. Ubah karakter ke huruf kapital
    2. masukkan karakter yang terbaca ke string `file`
    3. jika karakter berupa huruf (nilai ascii diantara 'A' dan 'Z'), tambah frekuensi kemunculan huruf dengan 1.
Isi file dan frekuensi ditulis pada pipe pertama dengan `write(fd1[1], file, sizeof(file))` dan `write(fd1[1], freq, sizeof(freq))`. Write pertama akan menulis string `file` pada pipe pertama dengan memori sebesar `sizeof(file)`, sedangkan write kedua menulis array `freq` dengan memori sebesar `sizeof(freq)`. Karena telah selesai mengirim pesan, maka tutup write end pipe pertama pada parent process dengan `fclose(fd1[1])`.

#### ***Algoritma Huffman***
Algoritma huffman menggunakan minheap tree untuk menentukan kode bit yang akan diberikan pada tiap huruf. Karenanya, program membutuhkan 3 struct, yaitu code, node, dan minheap, dengan atribut sebagai berikut:
1. struct code, kode bit huffman
    - int size, panjang bit kode huffman
    - int val[], kode bit huffman
2. struct node, node untuk tiap minheap tree
    - char data, menyimpan karakter huruf
    - unsigned freq, menyimpan frekuensi kemunculan huruf
    - node * left, left child dari node
    - node * right, right child dari node
3. minheap, struktur dari minheap tree
    - unsigned size, jumlah node saat in yang ada pada tree
    - unsigned capacity, kapasitas tree
    - strcut node ** array, array node pointer
Selanjutnya, huffman tree dibangun menggunakan fungsi `buildHuffmanTree()` berikut:
``` R
node *buildHuffmanTree(int freq[])
{
    node *left, *right, *top;
    int size = 0, temp = 0;
    for (int i = 0; i < 26; i++)
    {
        if (freq[i])
            size++;
    }

    minheap *minHeap = createMinHeap(size);

    for (int i = 0; i < 26; ++i)
    {
        if (freq[i])
        {
            minHeap->array[temp] = newNode(('A' + i), freq[i]);
            temp++;
        }
    }

    minHeap->size = size;
    buildMinHeap(minHeap);

    while (!isSizeOne(minHeap))
    {
        //Extract the two minimum
        //freq items from min heap
        left = extractMin(minHeap);
        right = extractMin(minHeap);

        //create new internal node
        top = newNode('$', left->freq + right->freq);

        top->left = left;
        top->right = right;

        insertMinHeap(minHeap, top);
    }

    return extractMin(minHeap);
}
```
Langkah-langkah pembuatan huffman tree adalah:
1. Buat leaf node untuk tiap karakter dan bangun minheap dari node tersebut.
    - Menentukan size dengan iterasi semua huruf. Jika frekuensi kemunculan huruf tidak `0`, tambah size tree.
    - Inisialisasi tree dengan atributnya menggunakan fungsi `createMinHeap(size)`. Fungsi ini akan menglokasikan memori sebanyak yang diperlukan struct dan inisialisasi nilai atribut `size=0`, `capacity=size`, `array dengan elemen sebanyak capacity`.
    - Inisialisasi node dengan iterasi semua huruf. Jika frekuensi kemunculan tidak `0`, buat node dengan `data=huruf`, `freq=frekuensi`, `left=NULL`, dan `right=NULL`. Hal tersebut dilakukan menggunakan fungsi `newNode()`.
    - Ubah nilai atribut size sebanyak jenis huruf yang muncul, lalu bangun minheap dengan fungsi `buildMinHeap()`.
2. Ekstrak dua node dengan frekuensi terkecil menggunakan fungsi `extractMinHeap()`. Node tersebut kemudian di-assign ke variabel `left` (frekuensi terkecil) dan `right` (frekuensi kedua terkecil).
3. Buat node baru (disebut internal node) dengan nilai atribut `data='$'`, `freq=jumlah freq left dan right`, `left=hasil ekstrak pertama (variabel left)`, dan `right=hasil ekstrak kedua (variabel right)`. Selanjutnya, tambahkan node tersebut ke dalam min heap dengan fungsi `insertMinHeap()`.
4. Ulangi langkah tersebut hingga tersisa 1 node (berupa internal node) pada minHeap, pada program terdapat pada perulangan while dengan kondisi `(!isSizeOne(minHeap))`. Internal node terakhir tersebut merupakan root dari huffman tree.

#### ***Kompress File menggunakan Algoritma Huffman***
Pada child process, hanya digunakan read end pipe pertama sehingga terlebih dahulu tutup write end dengan fungsi `close(fd1[1])`. Selanjutnya, baca pesan yang telah ditulis parent process pada pipe pertama dengan `read(fd1[0], file, sizeof(file))` dan `read(fd1[0], freq, sizeof(freq))`. Child process memiliki 2 tugas, yaitu membuat huffman tree dan mengkodekan isi file sesuai dengan huffamn tree yang telah terbentuk.<br>
##### ***Membuat huffman tree***
Huffman tree dapat dibuat menggunakan fungsi `buildHuffmanTree()` di atas. Namun, fungsi ini memberi return value berupa node* (root tree). Hasil tersebut dapat dikirim melalui pipes, tetapi tidak beserta variabel yang ditunjuk oleh pointer. Dengan begitu, huffman tree yang telah terbentuk perlu diubah dari berbentuk eksplisit (menggunakan pointer) ke bentuk array. Sebelum merubahnya, inisialisasi tiap node pada array dengan nilai seperti pada kode berikut:
``` R
for(int i=0;i<MAX_TREE_NODE;i++){
    htree[i].data='0';
    htree[i].freq=-1;
    htree[i].left=NULL;
    htree[i].right=NULL;
}
```
Selanjutnya, digunakan fungsi `getHtree()` (berjalan secara rekursif) untuk mengubah huffman tree ke bentuk array sekaligus menentukan kode bit tiap huruf (menggunakan variabel `table_code[][]`). Argumen pada fungsi ini adalah:
1. node *root, node huffman tree yang sedang dikunjungi
2. int bit[], array dari kode bit root
3. node tree[], huffman tree dalam bentuk array
4. int top, panjang kode bit node yang sedang dikunjungi
5. int idx, index array yang menunjukkan node yang sedang dikunjungi
``` R
void getHtree(node *root, int bit[], node tree[], int top, int idx)
{
    tree[idx].data=root->data;
    tree[idx].freq=root->freq;

    // Assign 0 to left edge and recur
    if (root->left)
    {
        int left=2*idx+1;
        bit[top] = 0;
        getHtree(root->left, bit, tree, top + 1, left);
    }

    if (root->right)
    {
        int right=2*idx+2;
        bit[top] = 1;
        getHtree(root->right, bit, tree, top + 1, right);
    }

    if (isLeaf(root))
    {
        int i, letter = root->data - 'A';
        for (i = 0; i < top; i++)
        {
            code_table[letter].val[i] = bit[i];
        }
        code_table[letter].size = top;
    }
}
```
Fungsi ini berjalan dengan menjelajahi semua node pada tree hingga ditemukan leaf (node asli, bukan internal node). Hal tersebut dilakukan dengan alur:
1. inisialisasi `data` dan `freq` dari tree berbentuk array sesuai dengan `data` dan `freq` root node.
2. jika terdapat left child, kunjungi node tersebut
    - inisialisasi indeks left child dengan `2*idx+1`
    - inisialisasi `bit[top]` dengan 0
    - jelajahi node kiri dengan memanggil `getHtree` (rekursif), increment `top`, dan gunakan indeks left child
3. jika terdapat right child, kunjungi node tersebut
    - inisialisasi indeks right child dengan `2*idx+1`
    - inisialisasi `bit[top]` dengan 0
    - jelajahi node kiri dengan memanggil `getHtree` (rekursif), increment `top`, dan gunakan indeks right child
4. jika berada pada leaf node,
    - inisialisasi indeks huruf dengan `letter = root->data - 'A'`
    - masukkan nilai kode bit pada array bit ke code_table[letter]

Huffman tree dan tabel kodenya yang telah terbentuk kemudian digunakan untuk mengubah isi file ke kode huffman. Hal tersebut dapat dilakukan dengan mengakses tiap karakter pada file secara iteratif. Karakter tersebut kemudian diubah sesuai kodenya pada tabel `code_table[][]`. Setiap bit pada kode huruf tersebut kemudian dimasukkan pada variabel `code_files[]`, yang menampung nilai kode bit seluruh file. Pada program, hal ini dijalankan dengan kode:
``` R
for (int i = 0; i < strlen(file); i++)
{
    if (file[i] >= 'A' && file[i] <= 'Z')
    {
        int let_index = file[i] - 'A';
        for (int j = 0; j < code_table[let_index].size; j++)
        {
            code_files[index] = code_table[let_index].val[j];
            index++;
        }
    }
}
```
Sebelum dilakukan while di atas, nilai array `code_files[]` diset dengan `-1` menggunakan `memset(code_files, -1, sizeof(code_files))`.
##### ***Mengirim huffman tree dan kode huffman file***
Setelah berhasil membangun huffman tree dan mengubah isi file ke bentuk kode huffman, child process perlu mengirimnya ke parent process menggunakan pipe kedua. Sebelum melakukan write pada pipe kedua, terlebih dahulu tutup read end pipe pertama dan kedua dengan `close(fd1[0])` dan `close(fd2[0])`. Kemudian write pada pipe kedua dengan:
- `write(fd2[1], htree, MAX_TREE_NODE*sizeof(node))`, tulis nilai dari array `htree` dengan memori sebesar `MAX_TREE_NODE*sizeof(node)`
- `write(fd2[1], code_files, sizeof(code_files))`, tulis kode huffman file dengan memori sebesar ukuran array `code_files`
- `close(fd2[1])`, tutup write end pipe kedua

#### ***Dekompresi file kode huffman***
Setelah mengirim isi file dan frekuensi, parent process menunggu child process membangun huffman tree. Agar ia tidak melanjutkan eksekusi, digunakan fungsi `wait(NULL)`. Setelah child process menulis hasil algoritma huffman dan menyelesaikan eksekusinya, parent process lalu menutup write end pipe kedua dan membaca hasilnya pada read end pipe kedua dengan
- `close(fd2[1])`, menutup write end pipe kedua
- `read(fd2[0], htree, MAX_TREE_NODE*sizeof(node))`, membaca huffman tree dan dimasukkan ke variabel `htree` dengan memori sebesar `MAX_TREE_NODE*sizeof(node)`
- `read(fd2[0], code_file, sizeof(code_file))`, membaca kode huffman file dan dimasukkan ke variabel `code_file` dengan memori sebesar `MAX_TREE_NODE*sizeof(node)`
Parent process kemudian menghitung panjang kode huffman dari files pada dengan menghitung jumlah bit pada `code_file`. Hal ini dilakukan dengan melakukan iterasi while hingga nilai dari code_file adalah -1 (akhir dari kode bit file) seperti berikut:
``` R
int sum_huffman_bits=0;
while(code_file[sum_huffman_bits]!=-1){
    sum_huffman_bits++;
}
```
Kode huffman dari file kemudian diterjemahkan kembali ke bentuk aslinya (huruf) dengan melakukan dekompresi. Hal tersebut dilakukan dengan fungsi `decompress()` dengan argumen:
- `node tree[]`, huffman tree
- `int code[]`, kode huffman dari file
- `int bits`, jumlah bit
``` R
void decompress(node tree[], int code[], int bits){
    int i=0, idx=0;
    for(i=0;i<=bits;i++){
        if(tree[idx].data>='A' && tree[idx].data<='Z'){
            printf("%c", tree[idx].data);
            idx=0;              //back to root
        }
        if(code[i]){
            idx=idx*2+2;        //go right
        }
        else{
            idx=idx*2+1;        //go left
        }
    }
    printf("\n");
    return;
}
```
Fungsi `decompress()` berjalan dengan alur:
1. inisialisasi `int i`, indeks bit yang sedang dikunjungi, dan `int idx`, index node tree yang sedang dikunjungi
2. lakukan iterasi sebanyak jumlah bit
3. jika `tree[idx]` berupa huruf (ascii di antara 'A' dan 'Z'), print huruf (`tree[idx].data`) dan kembalikan tree ke root (idx=0)
4. jika bit bernilai 1, kunjungi right child dari huffman tree (idx=idx*2+2)
5. jika bit bernilai 0, kunjungi left child dari huffman tree (idx=idx*2+1)
<br>

### Hasil
Hasil dari dekompresi kode huffman dan perbandingan jumlah bit sebelum dan setelah kompresi
<p align="center">
    <img src="https://github.com/Thoriqaafif/picture/blob/main/hasil%20lossless.c.png?raw=true">
</p>

## Nomor 2

### Deskripsi Soal
Fajar sedang sad karena tidak lulus dalam mata kuliah Aljabar Linear. Selain itu, dia tambah sedih lagi karena bertemu matematika di kuliah Sistem Operasi seperti kalian 🥲. Karena dia tidak bisa ngoding dan tidak bisa menghitung, jadi Fajar memohon jokian kalian. Tugas Fajar adalah sebagai berikut.

    A. Membuat program C dengan nama kalian.c, yang berisi program untuk melakukan perkalian matriks. Ukuran matriks pertama adalah 4×2 dan matriks kedua 2×5. Isi dari matriks didefinisikan di dalam code. Matriks nantinya akan berisi angka random dengan rentang pada matriks pertama adalah 1-5 (inklusif), dan rentang pada matriks kedua adalah 1-4 (inklusif). Tampilkan matriks hasil perkalian tadi ke layar.
    
    B. Buatlah program C kedua dengan nama cinta.c. Program ini akan mengambil variabel hasil perkalian matriks dari program kalian.c (program sebelumnya). Tampilkan hasil matriks tersebut ke layar. 
    (Catatan: wajib menerapkan konsep shared memory)

    C. Setelah ditampilkan, berikutnya untuk setiap angka dari matriks tersebut, carilah nilai faktorialnya. Tampilkan hasilnya ke layar dengan format seperti matriks.
    -- Contoh: 
    array [[1, 2, 3, 4], [5, 6, 7, 8], [9, 10, 11, 12], ...],
    -- maka:
    1 2 6 24 120 720 ... ... …
    (Catatan: Wajib menerapkan thread dan multithreading dalam penghitungan faktorial)

    D. Buatlah program C ketiga dengan nama sisop.c. Pada program ini, lakukan apa yang telah kamu lakukan seperti pada cinta.c namun tanpa menggunakan thread dan multithreading. Buat dan modifikasi sedemikian rupa sehingga kamu bisa menunjukkan perbedaan atau perbandingan (hasil dan performa) antara program dengan multithread dengan yang tidak. 
    Dokumentasikan dan sampaikan saat demo dan laporan resmi.

### Penyelesaian Soal 

_**Poin A**_

- Membuat program **`kalian.c`** untuk kedua matriks / matrix dengan men-definisikan fungsi `srand(time(NULL))` untuk generate random, deklarasi matriks 1, matriks 2 dan hasil matriks.  
```R
srand(time(NULL));

int mrx1[4][2]; // matriks 1 4x2
int mrx2[2][5]; // matriks 2 2x5

int hasil_mrx[4][5] = {0}; // inisiasi awal 0 untuk hasil matriks
```

- Mengisi angka random untuk tiap matriks
  - Matriks 1 angka random 1-5
  - Matriks 2 angka random 1-4
  - rand() % 5/4 + 1 agar rentang nya dimulai dari 1-5/4
```R
// matriks 1
for (int i = 0; i < 4; i++) {
	for (int j = 0; j < 2; j++) {
	    mrx1[i][j] = rand() % 5 + 1;
    }
}
```
```R
// matriks 2 
for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 5; j++) {
	    mrx2[i][j] = rand() % 4 + 1;
	}
}
```

- Melakukan perhitungan perkalian matriks untuk kedua matriks, kemudian ditampung pada var `hasil_mrx` (hasil matriks)
```R
for (int i = 0; i < 4; i++) {
	for (int j = 0; j < 5; j++) {
	    for (int k = 0; k < 2; k++) {
	      hasil_mrx[i][j] += mrx1[i][k] * mrx2[k][j];
	    }
    }  
}
```

- Buat shared memory dan connecting-kan dengan var `hasil_mrx` (hasil matriks), agar dapat diakses pada program `cinta.c`
```R
key_t keyShm = 1234;
int shmid = shmget(keyShm, sizeof(hasil_mrx), IPC_CREAT | 0666);
if (shmid == -1) {
	perror("error shmget");
	return 1;
}

int (*shmhasil_mrx)[5] = shmat(shmid, NULL, 0);
if (shmhasil_mrx == (void *) -1) {
	perror("error shmat");
	return 1;
}
```

- Setelah berhasil membuat shared memory, selanjutnya yaitu menyalin hasil dari perhitungan matriks dari var `hasil_mrx` kedalam shared memory
```R
for (int i = 0; i < 4; i++) {
	for (int j = 0; j < 5; j++) {
	    shmhasil_mrx[i][j] = hasil_mrx[i][j];
	}
}
```
- Menampilkan hasil matriks / matrix setelah dilakukan perhitungan perkalian 
  - shmdt untuk detach / end connecting dari shared memory
```R
printf("Hasil matrix kalian.c adalah :\n");
    
for (int i = 0; i < 4; i++) {
	for (int j = 0; j < 5; j++) {
	printf("%d ", hasil_mrx[i][j]);
	}
	printf("\n");
}
    	
shmdt(shmhasil_mrx);
```

_**Poin B & C**_

Penjelasan poin 

- Membuat program **`cinta.c`** dengan menangkap hasil perhitungan matriks dari program **`kalian.c`**
- Disini saya melakukan revisi terhadap `pola kode dalam mendapatkan faktorial` yang nantinya akan bersambung dengan program **`sisop.c`**

**Kode Praktikum**
```R
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>


/*
----------------------------------------------------------------------------------------------------
   1. program cinta.c menangkap hasil perhitungan matriks dari program kalian.c
   2. program cinta.c menghitung faktorial dari setiap iterasi matriks yang didapat dari kalian.c
----------------------------------------------------------------------------------------------------   
*/

// rumus dasar faktorial 
int facto_formula (int n) {
    if (n == 0) {
          return 1;
    } else {
           return n * facto_formula(n-1);
    }
}                 

// hitung faktorial
void *calc_facto (void *arg) {
// inisiasi var untuk setiap iterasi matriks
int i, j;
int (*hasil_mrx)[5] = arg;
     
for (i = 0; i < 4; i++) {
    for (j = 0; j < 5; j++) {
        hasil_mrx[i][j] = facto_formula(hasil_mrx[i][j]);
        }
    }
    pthread_exit(NULL);
}              

int main() {

    int shmid;
    key_t keyShm = 1234; // key shm untuk nangkap hasil kalian.c
    int (*hasil_mrx)[5];
    pthread_t thid;
    
    // buat segment shm
    shmid = shmget(keyShm, sizeof(hasil_mrx), IPC_CREAT | 0666);
    
    if (shmid == -1) {
        perror("error shmget");
        return 1;
    }
    
    // tunggu hingga program kalian.c selesai ngisi shm
    while (shmctl(shmid, IPC_STAT, NULL) == 0) {
       sleep(2);
    }   
    
    // attach / connect hm ke variabel hasil_mrx
    hasil_mrx = shmat(shmid, NULL, 0);
    
    if (hasil_mrx == (void *) -1) {
        perror("error shmat");
        return 1;
    }
    
    
    // get data - hasil matriks sebelum faktorial
    printf("  Hasil matrix cinta.c sebelum faktorial adalah :\n");
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            printf("%d ", hasil_mrx[i][j]);
        }
        printf("\n");
    }
    
    // buat thread u/ hitung faktorial
    pthread_create(&thid, NULL, calc_facto, hasil_mrx);
    // catch id thread
    pthread_join(thid, NULL);
    
    
    // get data - hasil setelah faktorial
    printf("  Hasil matrix cinta.c setelah faktorial adalah :\n");
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            printf("%d ", hasil_mrx[i][j]);
        }
        printf("\n");
    }
    
    // detach / end shm
    shmdt(hasil_mrx);
    
    return 0;
}
```

**Kode Revisi** 
```R
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

    int *shared_result = (int *)shmat(shmid, NULL, 0);
    if (shared_result == (int *)-1) {
        perror("shmat");
        exit(1);
    }

    printf("Hasil perkalian matriks dari program kalian.c:\n");
    printf("[");
    for (int i=0; i<4; i++) {
        printf("[");
        for (int j=0; j<5; j++) {
            printf("%d", *(shared_result + i*5 + j));
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
            *arg = *(shared_result + i*5 + j);
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
```

Penjelasan Kode Final (Revisi) :

_**Poin B**_

- Menangkap hasil dari perhitungan `hasil_mrx` dari program `kalian.c` dengan membuat shared memory pada fungsi main dengan kode yang sama dengan program `kalian.c`
```R
key_t key = 1234;
    
int shmid = shmget(key, sizeof(int), 0666);
if (shmid < 0) {
    perror("shmget");
    exit(1);
}

int *shared_result = (int *)shmat(shmid, NULL, 0);
if (shared_result == (int *)-1) {
    perror("shmat");
    exit(1);
}
```

- Menampilkan hasil matriks perkalian dari program `kalian.c` kedalam program `cinta.c`
```R 
printf("Hasil perkalian matriks dari program kalian.c:\n");
    printf("[");
    
    for (int i=0; i<4; i++) {
        printf("[");
        
        for (int j=0; j<5; j++) {
            printf("%d", *(shared_result + i*5 + j));
            
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
```

_**Poin C**_

- Mendefinisikan variabel `facto` sebagai var faktorial dan membuat thread untuk rumus perhitungan faktorial dengan nama `calc_facto`
```R
unsigned long long int facto;

void *calc_facto(void *arg) {
    int num = *((int *)arg);
    facto = 1;
    
    for (int i=1; i<=num; i++) {
        facto *= i;
    }
    return (void *)facto;
}
```

- Rumus perhitungan faktorial dengan menggunakan thread `calc_facto`
```R
pthread_t tid[4*5];
int index = 0;

for (int i=0; i<4; i++) {
        for (int j=0; j<5; j++) {
            int *arg = (int *)malloc(sizeof(*arg));
            *arg = *(shared_result + i*5 + j);
        
        pthread_create(&tid[index], NULL, calc_faktorial, (void *)arg);
        index++;
    }
}
```

- Menampilkan hasil dari perhitungan faktorial dengan menggunakan thread join 
```R 
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
```

** Note : `Kode Praktikum` dengan `Kode Revisi` memiliki perbedaan pada penulisan kode perhitungan faktorial

_**Poin D**_

- Membuat program **`sisop.c`** dengan inti program menangkap hasil yang sama dengan program `cinta.c` tanpa menggunakan ***thread dan multithreading***. Diawali dengan mendefinisikan variabel `facto` sebagai faktorial dan membuat fungsi perhitungan faktorial
```R
unsigned long long int facto;

unsigned long long int calc_facto(int num) {
    facto = 1;
    
    for (int i=1; i<=num; i++) {
        facto *= i;
    }
    return facto;
}
```

- Menangkap hasil dari program `cinta.c` dengan menggunakan shared memory pada fungsi `main` dan mendefinsikan key yang sama agar memory dapat di-loading pada program `sisop.c`. Kemudian ditampilkan hasilnya. 
```R
key_t key = 1234;
    
int shmid = shmget(key, sizeof(int), 0666);
if (shmid < 0) {
    perror("shmget");
    exit(1);
}

int *shared_result = (int *)shmat(shmid, NULL, 0);
if (shared_result == (int *)-1) {
    perror("shmat");
    exit(1);
}

printf("Hasil perkalian matriks dari program kalian.c:\n");
    printf("[");
    
    for (int i=0; i<4; i++) {
        printf("[");
        
        for (int j=0; j<5; j++) {
            printf("%d", *(shared_result + i * 5 + j));
            
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
```

- Membuat fungsi perhitungan faktorial dengan nama `calc_facto` tanpa menggunakan ***thread*** dan kemudian ditampilkan hasilnya
```R
printf("\nHasil faktorial:\n");
    printf("[");
    
    for (int i=0; i<4; i++) {
        printf("[");
        
        for (int j=0; j<5; j++) {
            facto = calc_facto(*(shared_result + i*5 + j));
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
```

### Output Program 


- Running **`kalian.c`** 

![outKal](https://user-images.githubusercontent.com/91828276/237708338-9568fcd8-a273-4d66-9f50-eb4649ca2953.png)

- Running **`cinta.c`**

![outCin](https://user-images.githubusercontent.com/91828276/237708332-0b9adaea-70e7-4f9d-8982-e74624148187.png)


- Running **`sisop.c`**

![outSis](https://user-images.githubusercontent.com/91828276/237708327-a397acff-5324-4246-abaa-81fabd659869.png)

---

**Perbandingan performa untuk program **`cinta.c`** dengan **`sisop.c`****

- Performa **`cinta.c`**

![perbCin](https://user-images.githubusercontent.com/91828276/237708324-acaf13fe-50ea-4ab5-8e1b-caae3e0c14ba.png)

- Performa **`sisop.c`**

![perbSis](https://user-images.githubusercontent.com/91828276/237708321-69cfc49f-7dbc-41ba-9526-194aae0795fb.png)

### Kendala
- Program `sisop.c` tidak dapat menerima sama dengan program `cinta.c` dengan sempurna
- Sempat stuck pada saat melakukan perubahan pola kode agar program `sisop.c` sama dengan program `cinta.c` 


## Nomor 3
### Deskripsi Soal

Elshe saat ini ingin membangun usaha sistem untuk melakukan stream lagu. Namun, Elshe tidak paham harus mulai dari mana.

- Bantulah Elshe untuk membuat sistem stream (receiver) stream.c dengan user (multiple sender dengan identifier) user.c menggunakan message queue (wajib). Dalam hal ini, user hanya dapat mengirimkan perintah berupa STRING ke sistem dan semua aktivitas sesuai perintah akan dikerjakan oleh sistem.

- User pertama kali akan mengirimkan perintah DECRYPT kemudian sistem stream akan melakukan decrypt/decode/konversi pada file song-playlist.json (dapat diunduh manual saja melalui link berikut) sesuai metodenya dan meng-output-kannya menjadi playlist.txt diurutkan menurut alfabet.
Proses decrypt dilakukan oleh program stream.c tanpa menggunakan koneksi socket sehingga struktur direktorinya adalah sebagai berikut:
├── 

	    ├── Soal3
            ├── playlist.txt    
            ├── song-playlist.json
            ├── stream.c
            └── user.c


- Selain itu, user dapat mengirimkan perintah LIST, kemudian sistem stream akan menampilkan daftar lagu yang telah di-decrypt
Sample Output:
17 - MK
1-800-273-8255 - Logic
1950 - King Princess
…
Your Love Is My Drug - Kesha
YOUTH - Troye Sivan
ZEZE (feat. Travis Scott & Offset) - Kodak Black


- User juga dapat mengirimkan perintah PLAY <SONG> dengan ketentuan sebagai berikut.
PLAY "Stereo Heart" 
    sistem akan menampilkan: 
    USER <USER_ID> PLAYING "GYM CLASS HEROES - STEREO HEART"
PLAY "BREAK"
    sistem akan menampilkan:
    THERE ARE "N" SONG CONTAINING "BREAK":
    1. THE SCRIPT - BREAKEVEN
    2. ARIANA GRANDE - BREAK FREE
dengan “N” merupakan banyaknya lagu yang sesuai dengan string query. Untuk contoh di atas berarti THERE ARE "2" SONG CONTAINING "BREAK":
PLAY "UVUWEVWEVWVE"
    THERE IS NO SONG CONTAINING "UVUVWEVWEVWE"

- Untuk mempermudah dan memperpendek kodingan, query bersifat tidak case sensitive 😀
User juga dapat menambahkan lagu ke dalam playlist dengan syarat sebagai berikut:
User mengirimkan perintah
ADD <SONG1>
ADD <SONG2>
sistem akan menampilkan:
USER <ID_USER> ADD <SONG1>

- User dapat mengedit playlist secara bersamaan tetapi lagu yang ditambahkan tidak boleh sama. Apabila terdapat lagu yang sama maka sistem akan meng-output-kan “SONG ALREADY ON PLAYLIST”
Karena Elshe hanya memiliki resource yang kecil, untuk saat ini Elshe hanya dapat memiliki dua user. Gunakan semaphore (wajib) untuk membatasi user yang mengakses playlist.
Output-kan "STREAM SYSTEM OVERLOAD" pada sistem ketika user ketiga mengirim perintah apapun.
Apabila perintahnya tidak dapat dipahami oleh sistem, sistem akan menampilkan "UNKNOWN COMMAND".

- Catatan: 
Untuk mengerjakan soal ini dapat menggunakan contoh implementasi message queue pada modul.
Perintah DECRYPT akan melakukan decrypt/decode/konversi dengan metode sebagai berikut:
ROT13
ROT13 atau rotate 13 merupakan metode enkripsi sederhana untuk melakukan enkripsi pada tiap karakter di string dengan menggesernya sebanyak 13 karakter.
Contoh:

- Base64
Base64 adalah sistem encoding dari data biner menjadi teks yang menjamin tidak terjadinya modifikasi yang akan merubah datanya selama proses transportasi. Tools Decode/Encode.
Hex
Hexadecimal string merupakan kombinasi bilangan hexadesimal (0-9 dan A-F) yang merepresentasikan suatu string. Tools.

### Penyelesaian
Untuk menyelesaikan permasalahan kali ini, kita bisa mmebutuhkan **user.c** sebagai user service dan **stream.c** sebagai systems service yang dapat menerima query dari user. Berikut merupakan kode dari **user.c**
```c
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <stdbool.h>

#define MAX_MESSAGE_LENGTH 200
#define PROJECT_IDENTIFIER "streamfile"

struct msg_buffer{
    long msgType;
    int userId;
    char msg[MAX_MESSAGE_LENGTH];
    char msgQuery[MAX_MESSAGE_LENGTH];
}message;

void Init(int *userId, key_t *key){
    srand(time(NULL));
    *key = ftok(PROJECT_IDENTIFIER, 18);
    *userId = rand() % 100;
}

void printHelp(){
    printf("Command List:\n\n");
    printf("-H             \t\t\tShow this message\n");
    printf("DECRYPT        \t\t\tDecrypt message from song-playlist.json and auto sort them\n");
    printf("LIST           \t\t\tList all decrypted song file\n");
    printf("PLAY <SONG>    \t\t\tPlay the song if available\n");        
    printf("ADD <SONG>     \t\t\tAdd new song in the playlist\n");
}

void upperCasing(char* str){
    for(int i=0; str[i] != '\0'; i++){
        str[i] = toupper(str[i]);
    }
}

void find_query_string(char* input,char* song) {
    char queryString[200];
    bool flag=0;
    int c = 0;
    for(int i=0; input[i]!='\0'; i++){
        if(input[i]=='\"' && flag==0){
            flag=1;
        }
        else if(input[i]=='\"'){
            queryString[c]='\0';
            strcpy(song,queryString);
            return;
        }
        else if(flag){
            queryString[c]=input[i];
            c++;
        }
    }
}

int main(){
    key_t key;
    int userId;
    Init(&userId, &key);
    int msgId;
    printf("Welcome To Sisop Stream... press -h for help: ");
    while(1){
        char command[MAX_MESSAGE_LENGTH];
        fgets(command,MAX_MESSAGE_LENGTH,stdin);
        command[strcspn(command, "\n")] = 0;
        upperCasing(command);
        if(strcmp(command,"-H")==0){
            printHelp();  
        }else if(strcmp(command,"DECRYPT")==0){

            msgId = msgget(key, 0666 | IPC_CREAT);
            strcpy(message.msg,command);
            message.msgType = 1;
            message.userId = userId;
            msgsnd(msgId, &message, sizeof(message), 0);

        }else if(strcmp(command,"LIST")==0){

            msgId = msgget(key, 0666 | IPC_CREAT);
            strcpy(message.msg,command);
            message.msgType = 1;
            message.userId = userId;
            msgsnd(msgId, &message, sizeof(message), 0);

        }else if(strstr(command,"PLAY")!=NULL){

            msgId = msgget(key, 0666 | IPC_CREAT);
            strcpy(message.msg,"PLAY");
            char songList[200];
            find_query_string(command,songList);
            strcpy(message.msgQuery,songList);
            printf("%s\n", message.msgQuery);
            message.msgType = 1;
            message.userId = userId;
            msgsnd(msgId, &message, sizeof(message), 0);

        }else if(strstr(command,"ADD")!=NULL){

            msgId = msgget(key, 0666 | IPC_CREAT);
            strcpy(message.msg,"ADD");
            char songList[200];
            find_query_string(command,songList);
            strcpy(message.msgQuery,songList);
            printf("%s\n", message.msgQuery);
            message.msgType = 1;
            message.userId = userId;
            msgsnd(msgId, &message, sizeof(message), 0);

        }else if(strcmp(command,"EXIT")==0){
            msgId = msgget(key, 0666 | IPC_CREAT);
            strcpy(message.msg,command);
            message.msgType = 1;
            message.userId = userId;
            msgsnd(msgId, &message, sizeof(message), 0);
            exit(0);
        }
        else{

            printf("There's No such command %s\n", command);
            printHelp();
       
        }
        printf("Insert Command: ");
    }
    return 0;
}
```

Program tersebut menginisialisasi value dari msgId sebagai identifier dari message queue dan mengirimkan message ke sistem message queue ketika user memberikan input perintah ke stream.c

Fungsi find_query_string merupakan fungsi untuk menemukan query dari perintah **ADD** dan **PLAY** agar query tersebut dapat diproses oleh stream service.

Struktur dari message queue sendiri terdiri atas message type, message query, message name, dan user id. user id digunakan sebagai identifier user di stream service untuk membatasi jumlah user yang dapat mengakses sebagaimana permintaan yang diberikan pada soal.

Berikut merupakan kode **stream.c**
```c
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <stdbool.h>
#include <semaphore.h>

#define FILE_PATH "song-playlist.json"
#define MAX_MESSAGE_LENGTH 200
#define PROJECT_IDENTIFIER "streamfile"
#define SONG_PLAYLIST "song.txt"
#define TEMP_FILE "temp.txt"

int users[2];
int allowed;

sem_t sem;
struct msg_buffer{
    long msgType;
    int userId;
    char msg[MAX_MESSAGE_LENGTH];
    char msgQuery[MAX_MESSAGE_LENGTH];
}message;

key_t keyInit(){
    key_t key = ftok(PROJECT_IDENTIFIER, 18);
    return key;
}

//ROT 13 Decoder
void rot13Decode(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];
        if (isalpha(c)) { 
            if (islower(c)) {
                str[i] = ((c - 'a' + 13) % 26) + 'a';
            } else {
                str[i] = ((c - 'A' + 13) % 26) + 'A';
            }
        } else { 
            str[i] = c;
        }
    }
}
//BASE64 Decoder
void base64Decode(char* input, int length) {
    BIO *b64, *bmem;

    char* buffer = (char*)malloc(length);
    memset(buffer, 0, length);

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new_mem_buf((void*)input, length);
    bmem = BIO_push(b64, bmem);

    BIO_read(bmem, buffer, length);

    BIO_free_all(bmem);

    strcpy(input,buffer);
}
//Hex To String Decoder
void hexToString(char *input) {
    int len = strlen(input) / 2;
    char *byteString = malloc(len + 1);

    for (int i = 0; i < len; i++) {
        sscanf(input + 2*i, "%2hhx", byteString + i);
    }
    byteString[len] = '\0';
    sprintf(input, "%s", byteString);
    free(byteString);
}


int songcmp(const void* a, const void* b){
    json_object *obj1 = *((json_object **)a);
    json_object *obj2 = *((json_object **)b);
    const char *song1 = json_object_get_string(json_object_object_get(obj1, "song"));
    const char *song2 = json_object_get_string(json_object_object_get(obj2, "song"));
    return strcmp(song1, song2);
}

void upperCasing(char* str){
    for(int i=0; str[i] != '\0'; i++){
        str[i] = toupper(str[i]);
    }
}


void controller(){
    sem_wait(&sem);
    if(users[1] == 1 && users[0] != 0){
        users[1] = message.userId;
        allowed = 1;
    }
    else if(users[0] == 0){
        users[0] = message.userId;
        users[1] = 1;
        allowed = 1;
    }
    else if(users[0]==message.userId && strcmp(message.msg,"EXIT")==0){
        printf("USER %d HAS LEFT THE SYSTEM\n",message.userId);
        users[0] = 0;
        allowed = 0;       
    }
    else if(users[1]==message.userId && strcmp(message.msg,"EXIT")==0){
        printf("USER %d HAS LEFT THE SYSTEM\n",message.userId);
        users[1] = 1;
        allowed = 0;      
    }
    else if(users[1] != 0 && message.userId!=users[1] && message.userId!= users[0] && users[0] != 0){
        printf("SYSTEM OVERLOAD\n");
        allowed = 0;
    }else{
        allowed = 1;
    }
    if(allowed != 0){
        if(strcmp(message.msg,"DECRYPT")==0){

                    FILE *json_file = fopen(FILE_PATH, "r");
                    if (!json_file) {
                        fprintf(stderr, "Failed to open %s\n", FILE_PATH);
                        return 1;
                    }

                    // Read the JSON data from the file
                    struct json_object *json_data = json_object_from_file(FILE_PATH);
                    if (!json_data) {
                        fprintf(stderr, "Failed to parse JSON data\n");
                        fclose(json_file);
                        return 1;
                    }

                    // Close the JSON file
                    fclose(json_file);

                    // Convert the JSON data to an array of json_object pointers
                    int num_objects = json_object_array_length(json_data);
                    json_object **objects = malloc(num_objects * sizeof(json_object *));
                    for (int i = 0; i < num_objects; i++) {
                        objects[i] = json_object_array_get_idx(json_data, i);
                    }
                    int counterbase64 = 0, countrot13=0, counthex =0;
                    // Decode the ROT13-encoded strings
                    for (int i = 0; i < num_objects; i++) {
                        json_object *obj = objects[i];
                        const char *method = json_object_get_string(json_object_object_get(obj, "method"));
                        const char *song = json_object_get_string(json_object_object_get(obj, "song"));
                        if (strcmp(method, "rot13") == 0) {
                            countrot13++;
                            rot13Decode(song);
                        }
                        else if(strcmp(method, "base64") == 0 ){
                            counterbase64++;
                            int songLen = strlen(song);
                            base64Decode(song,songLen);
                        }else if(strcmp(method, "hex")== 0){
                            counthex++;
                            hexToString(song);
                        }
                    }

                    // Sort the objects based on the song field
                    qsort(objects, num_objects, sizeof(json_object *), songcmp);

                    // Open the output file for writing
                    FILE *output_file = fopen(SONG_PLAYLIST, "w");
                    if (!output_file) {
                        fprintf(stderr, "Failed to open output file\n");
                        json_object_put(json_data);
                        free(objects);
                       return;
                    }

                    // Write the sorted JSON data to the output file
                    for (int i = 0; i < num_objects; i++) {
                        json_object *obj = objects[i];
                        const char *json_string = json_object_get_string(json_object_object_get(obj, "song"));
                        fprintf(output_file, "%s\n", json_string);
                    }
                    fclose(output_file);
                    free(objects);
                }
        else if(strcmp(message.msg,"LIST")==0){

                    FILE *songList = fopen(SONG_PLAYLIST, "r");
                    if(!songList){
                        fprintf(stderr, "Failed to open %s file\n", SONG_PLAYLIST);
                        fclose(songList);
                       return;
                    }
                    char buffer[200];
                    while(fgets(buffer,sizeof(buffer),songList) != NULL)printf("%s",buffer);
                    fclose(songList);

                }
        else if(strcmp(message.msg,"PLAY")==0){
                    
                    //Recieve song from message query
                    char song[200];
                    strcpy(song,message.msgQuery);
                    FILE *songList = fopen(SONG_PLAYLIST, "r");
                    if(!songList){
                        fprintf(stderr, "Failed to open %s file\n", SONG_PLAYLIST);
                       return;
                    }

                    FILE *tempFile = fopen(TEMP_FILE,"w");
                    if(!tempFile){
                        fprintf(stderr, "Failed to open %s file\n", TEMP_FILE);
                        fclose(songList);
                       return;
                    }

                    char buffer[200];
                    int counter = 0;
                    bool flag = false;
                    while(fgets(buffer,sizeof(buffer),songList)){
                        upperCasing(buffer);
                        if(strstr(buffer,song)!=NULL){
                            fprintf(tempFile,"%s",buffer);
                            counter++;
                            flag = true;
                        }
                    }
                    fclose(songList);             
                    fclose(tempFile);

                    tempFile = fopen(TEMP_FILE,"r");
                    if(!tempFile){
                        fprintf(stderr, "Failed to open %s file\n", TEMP_FILE);
                       return;
                    }

                    if(flag){
                        printf("THERE ARE %d SONGS CONTAINING %s:\n", counter, song);
                        for(int i=0;fgets(buffer,sizeof(buffer),tempFile) != NULL;i++){
                            printf("%d. %s",i+1,buffer);
                        }
                        fclose(tempFile);
                        system("rm temp.txt");
                    }else{
                        printf("THERE IS NO SONG CONTAINING \"%s\"\n",song);
                    }
                }
        else if(strcmp(message.msg,"ADD")==0){
                    bool flag = false;
                    char song[200];
                    strcpy(song, message.msgQuery);

                    FILE *songList = fopen(SONG_PLAYLIST, "r");
                    if (!songList) {
                        fprintf(stderr, "Failed to open %s file\n", SONG_PLAYLIST);
                       return;
                    }

                    char buffer[200];
                    while (fgets(buffer, sizeof(buffer), songList) != NULL) {
                        upperCasing(buffer);

                        if (strstr(buffer, song) != NULL) {
                            printf("SONG ALREADY ON PLAYLIST\n");
                            flag = true;
                            break;
                        }
                    }
                    fclose(songList);

                    if (!flag) {
                        songList = fopen(SONG_PLAYLIST, "a");
                        if (!songList) {
                            fprintf(stderr, "Failed to open %s file\n", SONG_PLAYLIST);
                           return;
                        }

                        printf("User %d ADD %s\n", message.userId, song);
                        fprintf(songList, "%s\n", song);
                        fclose(songList);
                    }
                }
    }
    sem_post(&sem);
}

int main(){
    sem_init(&sem, 0, 2);
    key_t key = keyInit();
    int msgId = msgget(key, 0666 | IPC_CREAT);
    
    while(1){
        if(msgrcv(msgId, &message, sizeof(message), 1, 0)){
            printf("New command coming from: %d\nCommand: %s\n\n", message.userId,message.msg);
            controller();
        }
    }
    msgctl(msgId, IPC_RMID, NULL);
    sem_destroy(&sem);
}
```

Pada stream service, kita mengimplementasikan message queue dimana dia merupakan reciever, dan juga menigmplementasikan semaphore untuk mengatasi pengaksesan critical resource. Pada service ini, akan dilakukan looping, lalu apabila ada mesasge yang masuk, maka akan diproses oleh stream service pada message fungsi **controller()**. Terdapat pula 3 fungsi decoder yakni **rot13decoder**, **base64decoder**, dan **hextostring** untuk melakukan decoding pada proses DECRYPT. Pada stream service ini juga ditambahkan mekanisme untuk menolak user ke3 sebelum user 1 atau user 2 exit dari program user service.

### Running Program
Contoh jalannya program
![runnning program](https://github.com/DJumanto/dummypict/blob/main/Dummy.png?raw=true)

## Nomor 4
Permasalahan pada nomor 4 ini melakukan download file `zip` yang ada pada google drive, unzip file tersebut, dan mengelompokkan file yang ada pada folder `files` sesuai dengan extensionnya. Aturan pengelompokan sebagai berikut:
1. pengelompokan dilakukan dengan membuat direktori `categorized`, pada direktori tersebut dibuat subdirektori dengan nama sesuai extension untuk mengelompokkan file.
2. jenis extension yang ingin dikumpulkan berada pada file `extensions.txt`
3. file dengan jenis extension selain yang ditentukan dimasukkan ke direktori other (di dalam direktori categorized)
4. pada max.txt, terdapat angka yang menjadi isi maksimum direktori, sehingga jika dierektori telah melebihi kapasitas, dibuat direktori baru dengan nama `extensions (2)`, `extensions (3)`, dst. (tidak berlaku untuk direktory other)
5. setiap pengaksesan folder, pemindahan file, dan pembuatan folder dicatat pada log.txt

Solusi dari permasalahan ini menggunakan 3 program c dengan tugas sebagai berikut:
- unzip.c, melakukan download file zip di google drive dan unzip file tersebut.
- categorize.c, melakukan pengelompokan mulai dari membuat folder, mengakses direktori files, memindahkan file sesuai extension, hingga melakukan pencatatan di log.txt
- logchecker.c, memvalidasi `log.txt` yang telah dibuat dengan menghitung jumlah accessed, list seluruh folder pada `categorized` beserta jumlah filenya, serta list jumlah file untuk tiap extension

### unzip.c
Program ini menggunakan fungsi `system()` dan command line linux untuk melakukan tugasnya dengan rincian berikut:
- Download file: `system("wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=1rsR6jTBss1dJh2jdEKKeUyTtTRi_0fqp' -O hehe.zip");`, kode ini melakukan download melalui url dengan command wget dan memberi nama file yang didownload dengan hehe.zip
- Unzip file: `system("mkdir hehe"); system("unzip hehe.zip -d hehe");`, kode ini melakukan pembuatan direktori degan nama hehe, lalu melakukan unzip file `hehe.zip` yang telah didownload dengan nama folder extract yaitu `hehe`

### categorize.c
program ini menggunakan 3 fungsi thread, yaitu:
- `void *madeCategorized()`, membuat direktori categorized
- `void *createFolder`, membaca extensions.txt dan membuat subdirektori categorized sesuai dengan extension pada file tersebut beserta other
- `void *listFilesRecursively()`, melakukan file listing secara rekursif, menentukan extension file, dan melakukan copy file tersebut sesuai dengan extensionnya.

Program ini juga menggunakan beberapa macro constant dan variabel global dengan rincian berikut:
- MAX_EXT 10, jumlah maksimal jenis extension (beserta other)
- MAX_PATH_CHR 200, jumlah maksimal karakter suatu path
- MAX_COMMAND 200, jumlah maksimal karakter suatu string command
- int maxfile, jumlah maksimal file untuk setiap direktori extension (berdasarkan max.txt)
- int numext = 0, jumlah jenis extension
- char extension[MAX_EXT][10], nama jenis setiap extension
- bool statusext[MAX_EXT], status akses setiap extension agar tercipta race condition ketika memindahkan file
- int numfile[MAX_EXT], jumlah file setiap extension
- bool statuslog = false, status pengaksesan `log.txt`

#### ***access, made, dan move***
Karena kode yang terlalu panjang, pada program ini terdapat 3 fungsi untuk melakukan log ketika acces folder, membuat direktori beserta lognya, dan memindahkan file beserta lognya. Ketiga fungsi tersebut adalah sebagai berikut:
``` R
void accessed(char path[])
{
    char cmd[MAX_COMMAND];
    strcpy(cmd, "echo $(date \"+%d-%m-%Y %X\") ACCESSED ");
    strcat(cmd, "\'");
    strcat(cmd, path);
    strcat(cmd, "\'");
    strcat(cmd, " >> log.txt");
    system(cmd);
}

void made(char path[])
{
    char cmd[MAX_COMMAND];

    // make directory
    strcpy(cmd, "mkdir ");
    strcat(cmd, "\'");
    strcat(cmd, path);
    strcat(cmd, "\'");
    system(cmd);

    // write log
    strcpy(cmd, "echo \"$(date \"+%d-%m-%Y %X\") MADE ");
    strcat(cmd, path);
    strcat(cmd, "\" >> log.txt");
    system(cmd);
}

void move(char src[], char dest[], char ext[])
{
    char cmd[MAX_COMMAND];

    // move file
    strcpy(cmd, "cp ");
    strcat(cmd, "\'");
    strcat(cmd, src);
    strcat(cmd, "\'");
    strcat(cmd, " ");
    strcat(cmd, "\'");
    strcat(cmd, dest);
    strcat(cmd, "\'");
    system(cmd);

    // write log
    strcpy(cmd, "echo \"$(date \"+%d-%m-%Y %X\") MOVED ");
    strcat(cmd,ext);
    strcat(cmd," ");
    strcat(cmd, src);
    strcat(cmd, " > ");
    strcat(cmd, dest);
    strcat(cmd, "\" >> log.txt");
    system(cmd);
}
```
Penjelasan untuk tiap fungsi:
1. `void accessed()`
    - menerima input string berupa path yang sedang diakses
    - menggunakan string cmd untuk menyimpan command linux yang akan dieksekusi, dan mengisi string menggunakan `strcpy()` dan `strcat()`
    - memasukkan command linux, `echo $(date "+%d-%m-%Y %X") ACCESSED '{path}' >> log.txt`, ke string cmd untuk melakukan pencatatan ke log.txt
    - menjalan command dengan `system (cmd)`
2. `void made()`
    - menerima input string berupa path direktori yang akan dibuat
    - menggunakan string cmd untuk menyimpan command linux yang akan dieksekusi, dan mengisi string menggunakan `strcpy()` dan `strcat()`
    - memasukkan command linux, `mkdir '{path}'`, ke string cmd untuk melakukan pencatatan ke log.txt
    - menjalan command dengan `system (cmd)`
    - memasukkan command linux, `echo $(date "+%d-%m-%Y %X") MADE '{path}' >> log.txt`, ke string cmd untuk melakukan pencatatan ke log.txt
    - menjalan command dengan `system (cmd)`
3. `void move()`
    - menerima input string berupa path awal file yang akan dipindah dan path tujuan pemindahan
    - menggunakan string cmd untuk menyimpan command linux yang akan dieksekusi, dan mengisi string menggunakan `strcpy()` dan `strcat()`
    - memasukkan command linux, `cp '{path_awal}' '{path_tujuan}'`, ke string cmd untuk melakukan pencatatan ke log.txt
    - menjalan command dengan `system (cmd)`
    - memasukkan command linux, `echo "$(date "+%d-%m-%Y %X") MOVED '{path_awal}' '{path_tujuan}'" >> log.txt`, ke string cmd untuk melakukan pencatatan ke log.txt
    - menjalan command dengan `system (cmd)`

Karena ketiga fungsi melakukan pencatatan pada log.txt, maka perlu dilakukan pengecekan `statuslog` sebelum memanggil salah satu diantara 3 fungsi di atas seperti berikut:
``` R
while (statuslog)
{
}
statuslog = true;
/*pemanggilan fungsi*/
statuslog = false;
```
#### ***madeCategorized dan createFolder***
Fungsi thread madeCategorized membuat direktori dengan menggunakan fungsi made yang telah dibuat. Namun sebelum pemanggilan fungsi tersebut, dipastikan terlebih dahulu tidak ada thread lain yang sedang menulis di log.txt dengan mengecek variabel `statuslog`. Jika bernilai `true`, lakukan while (agar fungsi made tidak berjalan). Jika `false`, ubah `statuslog` menjadi `true`, memanggil `made()`, dan ubah `statuslog` menjadi `false`. Hal tersebut dilakukan dengan kode berikut:
``` R
while (statuslog)
{
}
statuslog = true;
made("categorized");
statuslog = false;
```
Fungsi thread createFolder melakukan log access folder categorized (dengan fungsi access), membaca jenis extension pada `extensions.txt`, membuat folder sesuai jenis extension, dan membuat folder `other`. log access dilakukan menggunakan fungsi `accessed()`. Sedangkan pembacaan `extensions.txt` dan pembuatan folder dilakukan dengan melakukan looping `fscanf()` file hingga akhir file. Hasil pembacaan jenis extension kemudian dimasukkan ke variabel extensions[numExt]. Selanjutnya, nama extension digabung dengan path categorized yang dikirim sebagai argumen menggunakan `strcat()`. String baru tersebut kemudian digunakan sebagi argumen untuk memanggil fungsi `made()`. Setelah semua extension berhasil dibuat folder, fungsi ini memasukkan string "other" ke array extensions[numext] dan membuat driektori other dengan fungsi `made("other")`. Implementasi kode sebagai berikut:
``` R
while (fscanf(fptr, "%s", extension[numext]) > 0)
{
    char cmd[MAX_COMMAND], rel_path[MAX_PATH_CHR];
    strcpy(rel_path, "categorized/");
    strcat(rel_path, extension[numext]);

    while (statuslog)
    {
    }
    statuslog = true;
    made(rel_path);
    statuslog = false;

    numext++;
}
strcpy(extension[numext], "other");

while (statuslog)
{
}
statuslog = true;
made("categorized/other");
statuslog = false;
```

#### ***listFilesRecursively()***
Variabel yang digunakan:
- char relPath[MAX_PATH_CHR], path folder yang akan dilakukan file listing
- char path[MAX_PATH_CHR], path suatu file atau folder (dapat diganti sesuai path yang sedang dikunjungi)
- char destPath[MAX_PATH_CHR], path tujuan ketika akan memindahkan file ke folder extension yang sesuai
- char cmd[MAX_COMMAND], string command linux yang akan dieksekusi
- int idxExt, index extension
- pthread_t t_id[50], id thread untuk membuat thread
- int numthread = 0, jumlah thread yang dibuat (diinisialisasi dengan 0)

Thread ini akan membuka suatu direktori, kemudian membaca seluruh isi (file maupun direktori) yang ada di dalamnya. Jika isi berupa direktori, akses direktori tersebut dengan membuat thread `listFilesRecursively()` baru sehingga thread/fungsi ini bekerja secara rekursif. Membuka dan membaca isi direktori dilakukan dengan menggunakan fungsi `opendir()` dan `readdir()` yang ada pada library `dirent.h`. Untuk membaca seluruh isi, lakukan `readdir()` secara berulang hingga tidak terdapat isi lagi pada direktori tersebut (`readdir()` mengembalikan nilai `NULL`). Karena mungkin terdapat file "." dan "..", maka dilakukan filter terlebih dahulu dengan `if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)` agar hanya isi asli yang diakses. Selanjutnya, simpan path isi pada variabel `path` dengan menggabungkan `relpath` dengan nama isi yang tersimpan pada `dp->d_name`. Hal tersebut dilakukan menggunakan `strcpy()` dan `strcat()`. Selanjutnya, panggil fungsi accessed (untuk log access file/folder) dan cek apakah isi berupa file atau direktori dengan fungsi isDir berikut:
``` R
bool isDir(char path[])
{
    struct dirent *dp;
    DIR *dir = opendir(path);

    if (!dir)
        return false;

    if ((dp = readdir(dir)) != NULL)
    {
        return true;
    }

    return false;
}
```
Jika isi berupa folder, maka program perlu melakukan listing file di folder tersebut sehingga dibuat thread `listFilesRecursively` dengan argumen path folder tersebut, lalu jumlah thread juga perlu ditambah (numthread diincrement) seperti pada kode berikut:
``` R
char const_path[MAX_PATH_CHR];
strcpy(const_path, path);
pthread_create(&t_id[numthread], NULL, *listFilesRecursively, (void *)const_path);
numthread++;
```
Jika isi berupa file, cek extension file tersebut menggunakan `strstr(path, '.')`, fungsi ini akan mereturn string yang berada setelah karakter '.' terakhir. Namun, terdapat file yang tidak memiliki extension sehingga fungsi `strstr(path, '.')` akan mereturn NULL. File tersebut dijadikan kondisi khusus dan dianggap akan dipindah (menggunakan fungsi `move()`) ke folder other. Setelah pemindahan, evaluasi jumlah file other dengan ditambah 1 dengan `numfile[numext]++`, numnext merupakan jumlah extension. Karena other, pada pembacaan extension, ditaruh paling belakang di array, numnext juga berarti indeks yang merujuk pada extension/folder other. Implementasi kode adalah sebagai berikut:
``` R
while (statusext[numext] || statuslog)
{
}
statusext[numext] = true;
statuslog = true;
move(path, "categorized/other", "other");
numfile[numext]++;
statusext[numext] = false;
statuslog = false;
```
Untuk file yang memiliki extension, dilakukan search index dari jenis extensionnya menggunakan `searchExt()`. Fungsi search ini melakukan linear search pada array `extensions[]` dan mereturn index jika ditemukan jenis extension yang sesuai. Selanjutnya, ditentukan path tujuan berdasarkan extensionnya. Terdapat 3 kasus untuk masalah ini:
1. Extension berupa other, maka path tujuan adalah folder other (extensions[idxExt])
2. Extension bukan other dan jumlah file pada extension tersebut kurang dari maxfile, maka path tujuan adalah folder `extensions[idxExt]`
3. Extension bukan other dan jumlah file telah melebihi maxfile, maka path tujuan adalah `extension[idxExt](n)`, dengan n bernilai `(numfile[idxExt]) / maxfile + 1`
    - pada kasus ini, jika file merupakan file pertama di folder (`numfile[idxExt] % maxfile == 0`), maka buat directory untuk `extension[idxExt](n)` terlebih dahulu dengan fungsi `made()`

Path tujuan tersebut kemudian digunakan sebagai argumen untuk memindahkan file ke direktori extension yang sesuai dalam fungsi `move()`. Implementasi kode untuk proses ini adalah sebagai berikut:
``` R
idxExt = searchExt(ext + 1);

while (statusext[idxExt] || statuslog)
{
}
statusext[idxExt] = true;
statuslog = true;
if (idxExt == numext)
    sprintf(destPath, "categorized/%s", extension[idxExt]);
else if (numfile[idxExt] < maxfile)
    sprintf(destPath, "categorized/%s", extension[idxExt]);
else
{
    int n = (numfile[idxExt]) / maxfile + 1;
    sprintf(destPath, "categorized/%s(%d)", extension[idxExt], n);

    if (numfile[idxExt] % maxfile == 0)
    {
        made(destPath);
    }
}
move(path, destPath, extension[idxExt]);
numfile[idxExt]++;
statusext[idxExt] = false;
statuslog = false;
```

#### ***main***
Variabel yang digunakan:
- `pthread_t t_id[3]`, variabel untuk membuat thread
- `path = "/home/thoriqaafif/sisop/hehe"`, variabel menyimpan path ke direktori hehe

Pada main, dilakukan beberapa hal berikut:
1. mengubah working direktori ke folder hehe dengan `chdir(path)`
2. membuat thread `madeCategorized` dengan `pthread_create(&t_id[0], NULL, *madeCategorized, NULL)`. Selanjutnya, digunakan `pthread_join()` agar main tidak melanjutkan eksekusi hingga direktori categorized terbentuk
3. membuat string `ext_path` yang menyimpan path `extensions.txt` dan buat thread `createFolder` dengan argumen `pthread_create(&t_id[1], NULL, *createFolder, (void *)ext_path)`
4. baca `max.txt` dan masukkan nilai pada file txt tersebut ke variabel `maxfile`
5. set `statusExt` menjadi `false` dan `numfile` menjadi `0` menggunakan `memset()`
6. membuat thread `listFilesRecursively` dengan `pthread_create(&t_id[2], NULL, *listFilesRecursively, (void *)"files")`
7. menampilkan jumlah file setiap extension dan total. Hal ini dilakukan dengan melakukan perulangan for sejumlah jenis extension, menampilkan jumlah file pada extension[i], kemudian menambahkan variabel num (total file, diinisialisai dengan 0) dengan jumlah file extension[i]. Setelah semua extension berhasil ditampilkan, tampilkan total file dengan `printf("Jumlah File: %d\n", num)`

### categorize.c
Untuk mengecek apakah `log.txt` yang dibuat sudah tepat, logchecker.c mengecek jumlah access, list folder beserta jumlah filenya, dan jumlah file tiap extension. Program ini menggunakan struct dan variabel:
1. Struct Directory, dengan atribut `char name[1000]`, nama direktori, dan `int fileCnt`, jumlah file pada direktori tersebut
2. array `d[1000]` bertipe struct Directory, array direktori yang relah dibuat
3. `int dIdx`, indeks direktori pada aray `d[1000]`
4. `int extCnts[9]`, jumlah file untuk tiap extension
Jumlah access dihitung menggunakan awk yang dijalankan dengan fungsi system() seperti berikut:
``` R
system("awk 'BEGIN {} /ACCESSED/ {n++} END {print \"Jumlah Accessed: \",n,\"\\n\"}' log.txt");
```
List folder didapatkan menggunakan fungsi `countDirsAndExts()`. Fungsi ini berjalan dengan 3 proses utama, yaitu menghitung jumlah dir, menghitung file untuk tiap dir, dan menampilkan dir beserta jumlah filenya secara terurut. Menghitung jumlah dir dilakukan dengan membaca baris per baris `log.txt` dan mencari baris yang mengandung string `MADE`. Sesuai dengan format log, maka nama dir berada setelah kata MADE sehingga string nama dir didapatkan dengan `char *dir = strstr(ln, "MADE ") + strlen("MADE ")`. Implementasi kode sebagai berikut:
``` R
while (fgets(ln, 1000, logFile)) {
    if (strstr(ln, "MADE ") != NULL) {
        ln[strlen(ln) - 1] = '\0';
        char *dir = strstr(ln, "MADE ") + strlen("MADE ");

        strcpy(d[dIdx].name, dir);
        d[dIdx].fileCnt = 0;
        dIdx++;
    }
}
```
Selanjutnya, jumlah file didapatkan dengan cara yang sama, namun di sini dicari baris yang mengandung `MOVE `, lalu cari indeks direktori tersebut dengan fungsi `cek_dir()` dan assign ke variabel num. indeks tersebut kemudian digunakan untuk evaluasi (increment) jumlah file dari direktori tersebut. Implementasi kode sebagai berikut:
``` R
while (fgets(ln, 1000, logFile)) {
    if (strstr(ln, "> ") != NULL) {
        ln[strlen(ln) - 1] = '\0';
        char *dir = strstr(ln, "> ") + strlen("> ");

        int num = cek_dir(dir);
        d[num].fileCnt++;
    }
}
```
Karena folder harus terurut berdasarkan jumlah filenya, digunakan fungsi sorting yang menerapkan quicksort, yaitu `sortByFileCnt(0, dIdx - 1)`. array `d[]` yang telah terurut tadi ditampilkan satu persatu dengan melakukan looping seperti berikut:
``` R
for (int i = 0; i < dIdx; i++) {
    printf("%s | File Count : %d\n", d[i].name, d[i].fileCnt);

    if (strstr(d[i].name, "emc") != NULL) {
        extCnts[0] += d[i].fileCnt;
    } else if (strstr(d[i].name, "jpg") != NULL) {
        extCnts[1] += d[i].fileCnt;
    } else if (strstr(d[i].name, "js") != NULL) {
        extCnts[2] += d[i].fileCnt;
    } else if (strstr(d[i].name, "png") != NULL) {
        extCnts[3] += d[i].fileCnt;
    } else if (strstr(d[i].name, "py") != NULL) {
        extCnts[4] += d[i].fileCnt;
    } else if (strstr(d[i].name, "txt") != NULL) {
        extCnts[5] += d[i].fileCnt;
    } else if (strstr(d[i].name, "xyz") != NULL) {
        extCnts[6] += d[i].fileCnt;
    } else {
        extCnts[7] += d[i].fileCnt;
    }
}
```
<br>

### Hasil
- unzip
<p align="center">
    <img src="https://github.com/Thoriqaafif/picture/blob/main/Screenshot%202023-05-13%20104604.png?raw=true">
</p>
- Folder categorized
<p align="center">
    <img src="https://github.com/Thoriqaafif/picture/blob/main/Screenshot%202023-05-13%20104936.png?raw=true">
</p>
- isi `log.txt`
<p align="center">
    <img src="https://github.com/Thoriqaafif/picture/blob/main/Screenshot%202023-05-13%20104955.png?raw=true">
</p>
- output categorize.c
<p align="center">
    <img src="https://github.com/Thoriqaafif/picture/blob/main/Screenshot%202023-05-13%20104900.png?raw=true">
</p>
- output logchecker.c
<p align="center">
    <img src="https://github.com/Thoriqaafif/picture/blob/main/Screenshot%202023-05-13%20105027.png?raw=true">
</p>
