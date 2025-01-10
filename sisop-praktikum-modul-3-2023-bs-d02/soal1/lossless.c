#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX_TREE_NODE 400
#define MAX_CHAR_BIT 30
#define MAX_BIT 6000

struct code
{
    int size;
    int val[MAX_CHAR_BIT];
} typedef code;

code code_table[26]; // huffman code

// node of min heap
struct node
{
    char data;
    unsigned freq;

    struct node *left, *right;
} typedef node;

node htree[MAX_TREE_NODE];

// min heap structure
struct minheap
{
    unsigned size;
    unsigned capacity;
    node **array;
} typedef minheap;

node *newNode(char data, unsigned freq)
{
    node *temp = (node *)malloc(
        sizeof(node));

    temp->left = temp->right = NULL;
    temp->data = data;
    temp->freq = freq;

    return temp;
}

minheap *createMinHeap(unsigned capacity){
    minheap *minHeap = (struct minheap *)malloc(sizeof(minheap));

    // current size is 0
    minHeap->size = 0;

    minHeap->capacity = capacity;

    minHeap->array = (node **)malloc(
        minHeap->capacity * sizeof(node *));
    return minHeap;
}

void swapMinHeapNode(node **a, node **b){
    node *t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(minheap *minHeap, int idx){
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx)
    {
        swapMinHeapNode(&minHeap->array[smallest],
                        &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

int isSizeOne(minheap *minHeap)
{

    return (minHeap->size == 1);
}

node *extractMin(minheap *minHeap)
{

    node *temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];

    --minHeap->size;
    minHeapify(minHeap, 0);

    return temp;
}

void insertMinHeap(minheap *minHeap,
                   node *minHeapNode)
{
    ++minHeap->size;
    int i = minHeap->size - 1;

    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq)
    {

        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }

    minHeap->array[i] = minHeapNode;
}

void buildMinHeap(minheap *minHeap)

{

    int n = minHeap->size - 1;
    int i;

    for (i = (n - 1) / 2; i >= 0; --i)
        minHeapify(minHeap, i);
}

int isLeaf(node *root)
{
    return !(root->left) && !(root->right);
}

void getHtree(node *root, int bit[], node tree[], int top, int idx)
{
    //assign node in explicit tree
    //to implicist tree (use array)
    tree[idx].data=root->data;
    tree[idx].freq=root->freq;

    // Assign 0 to left edge and recur
    if (root->left)
    {
        int left=2*idx+1;       //left node are in index 2*i+1
        bit[top] = 0;
        getHtree(root->left, bit, tree, top + 1, left);
    }

    // Assign 1 to right edge and recur
    if (root->right)
    {
        int right=2*idx+2;        //right node are in index 2*i+2
        bit[top] = 1;
        getHtree(root->right, bit, tree, top + 1, right);
    }

    // If this is a leaf node, then
    // it contains one of the input
    // characters, print the character
    // and its code from arr[]
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

// The main function that builds Huffman tree
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

    // Iterate while size of heap doesn't become 1
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

int main()
{
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

    id = fork();

    // fork failed
    if (id < 0)
    {
        fprintf(stderr, "fork failed");
    }

    // parent process
    else if (id == 0)
    {
        close(fd1[0]); // close reading end of first pipe

        // read file and
        // count frequencies
        FILE *fptr;
        fptr = fopen("/home/thoriqaafif/sisop/file.txt", "r");

        char c;
        char file[1000];
        int freq[26], count = 0, jumlah_huruf = 0;
        memset(freq, 0, 26 * sizeof(int)); // set all frequency to zero

        while (fscanf(fptr, "%c", &c) == 1)
        {
            c = toupper(c);
            file[count] = c;
            count++;

            // count frequency
            if (c >= 'A' && c <= 'Z')
            {
                freq[c - 'A'] += 1;
                jumlah_huruf++;
            }
        }
        fclose(fptr);

        //print frekuensi kemunculan tiap huruf
        printf("Frekuensi kemunculan huruf:\n");
        for(int i=0;i<26;i++){
            printf("%c: %d\n",('A'+i),freq[i]);
        }
        printf("\n");

        // send file and array of letter's frequencies
        write(fd1[1], file, sizeof(file)); // write file
        write(fd1[1], freq, sizeof(freq)); // write letter's frequencies
        close(fd1[1]);                     // close writing end of the first pipe

        // wait for the child process
        // until huffman coding finish
        wait(NULL);

        close(fd2[1]); // close writing end of the second pipe

        // read result of the huffman coding
        node *htree=(node*)malloc(MAX_TREE_NODE*sizeof(node));;
        int code_file[MAX_BIT];
        memset(code_file, -1, sizeof(code_file));
        read(fd2[0], htree, MAX_TREE_NODE*sizeof(node));    // read huffman tree
        read(fd2[0], code_file, sizeof(code_file));               // read bit of compressed file

        //read huffman code and decompress
        int sum_huffman_bits=0;
        while(code_file[sum_huffman_bits]!=-1){
            sum_huffman_bits++;
        }
        printf("Hasil dekompresi kode huffman:\n");
        decompress(htree, code_file,sum_huffman_bits);

        //display bits sum
        printf("\nPerbandingan jumlah bit:\n");
        printf("Sebelum kompresi: %d\n", jumlah_huruf*8);
        printf("Sesudah kompresi: %d\n", sum_huffman_bits);

        close(fd2[0]);      //close reading end
    }

    // child process
    else
    {
        close(fd1[1]); // close writing end of the first pipe

        // reading files and letter's frequencies
        char file[1000];
        int freq[26];
        read(fd1[0], file, sizeof(file));
        read(fd1[0], freq, sizeof(freq));

        // compress file using huffman coding
        // get huffman tree and code of the file
        node *htreeptr = buildHuffmanTree(freq);
        node *htree= (node*)malloc(MAX_TREE_NODE*sizeof(node));
        int arr[MAX_CHAR_BIT];
        int code_files[MAX_BIT], index = 0;

        //set htree
        for(int i=0;i<MAX_TREE_NODE;i++){
            htree[i].data='0';
            htree[i].freq=-1;
            htree[i].left=NULL;
            htree[i].right=NULL;
        }
        //get hufman tree
        getHtree(htreeptr, arr, htree, 0, 0);

        memset(code_files, -1, sizeof(code_files));

        // code the file
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

        //kode huffman karakter yang muncul
        printf("Kode huffman karakter yang muncul\n");
        for(int i=0;i<26;i++){
            if(freq[i]>0){
                printf("%c: ", ('A'+i));
                for(int j=0;j<code_table[i].size;j++){
                    printf("%d", code_table[i].val[j]);
                }
                printf("\n");
            }
        }
        printf("\n");

        // close both reading ends
        close(fd1[0]);
        close(fd2[0]);

        // write huffman tree and bit of compressed file
        // then close writing end
        write(fd2[1], htree, MAX_TREE_NODE*sizeof(node));
        write(fd2[1], code_files, sizeof(code_files));
        close(fd2[1]);
    }
}