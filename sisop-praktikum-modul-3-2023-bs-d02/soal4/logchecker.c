#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct Directory {
    char name[1000];
    int fileCnt;
} d[1000];

int dIdx = 0;
int extCnts[9] = {0};

int partition(int low, int high) {
    int pivot = d[high].fileCnt;
    int i = low - 1;

    for (int j = low; j <= high - 1; j++) {
        if (d[j].fileCnt <= pivot) {
            i++;
            struct Directory tmp = d[i];
            d[i] = d[j];
            d[j] = tmp;
        }
    }

    struct Directory tmp = d[i + 1];
    d[i + 1] = d[high];
    d[high] = tmp;

    return i + 1;
}

void sortByFileCnt(int low, int high) {
    if (low < high) {
        int pivot = partition(low, high);
        sortByFileCnt(low, pivot - 1);
        sortByFileCnt(pivot + 1, high);
    }
}

int cek_dir(char dirs[]) {
    for (int i = 0; i < dIdx; i++) {
        if (strcmp(dirs, d[i].name) == 0)
            return i;
    }
    return -1;
}

void countDirsAndExts() {
    char ln[1000];

    FILE *logFile;
    logFile = fopen("log.txt", "r");

    //count num of dirs
    while (fgets(ln, 1000, logFile)) {
        if (strstr(ln, "MADE ") != NULL) {
            ln[strlen(ln) - 1] = '\0';
            char *dir = strstr(ln, "MADE ") + strlen("MADE ");

            strcpy(d[dIdx].name, dir);
            d[dIdx].fileCnt = 0;
            dIdx++;
        }
    }
    fclose(logFile);

    logFile = fopen("log.txt", "r");

    //count num of files of each dir
    while (fgets(ln, 1000, logFile)) {
        if (strstr(ln, "> ") != NULL) {
            ln[strlen(ln) - 1] = '\0';
            char *dir = strstr(ln, "> ") + strlen("> ");

            int num = cek_dir(dir);
            d[num].fileCnt++;
        }
    }
    fclose(logFile);

    sortByFileCnt(0, dIdx - 1);

    //count num file of each extension
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
}

void outputExtensions() {
    printf("emc: %d\n", extCnts[0]);
    printf("jpg: %d\n", extCnts[1]);
    printf("js: %d\n", extCnts[2]);
    printf("png: %d\n", extCnts[3]);
    printf("py: %d\n", extCnts[4]);
    printf("txt: %d\n", extCnts[5]);
    printf("xyz: %d\n", extCnts[6]);
    printf("other: %d\n", extCnts[7]);
}

int main() {
    chdir("/home/thoriqaafif/sisop/hehe");
    // Hitung ACCESSED
    system("awk 'BEGIN {} /ACCESSED/ {n++} END {print \"Jumlah Accessed: \",n,\"\\n\"}' log.txt");

    // List Folder
    printf("Folders List : \n");
    countDirsAndExts();
    puts("");

    // Jumlah Extensions
    outputExtensions();
}