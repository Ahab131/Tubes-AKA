#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#define MAX_EXT 10
#define MAX_PATH_CHR 200
#define MAX_COMMAND 200

// extensions
int maxfile;                 // jumlah maks file pada folder extension
int numext = 0;              // jumlah extension
char extension[MAX_EXT][10]; // jenis extension
bool statusext[MAX_EXT];     // status akses folder extension
int numfile[MAX_EXT];        // jumlah file tiap extension

// log
bool statuslog = false; // status akses log

// search index of the file extension
int searchExt(char ext[])
{
    for (int i = 0; i < numext; i++)
    {
        if (strcmp(ext, extension[i]) == 0)
            return i;
    }
    return numext;
}

// write log for accessed
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

// made dir
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

// move file
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

void *listFilesRecursively(void *relativePath)
{
    char relPath[MAX_PATH_CHR];
    strcpy(relPath, (char *)relativePath);
    char path[MAX_PATH_CHR];     // path of file or folder
    char destPath[MAX_PATH_CHR]; // path of move destination
    char cmd[MAX_COMMAND];       // string of linux command
    int idxExt;                  // index of extension type
    pthread_t t_id[50];
    int numthread = 0;
    struct dirent *dp;
    DIR *dir = opendir(relPath);

    if (!dir)
        return NULL;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            // Construct new relative path from our base path
            strcpy(path, relPath);
            strcat(path, "/");
            strcat(path, dp->d_name); // new relative path

            // log access folder or file
            while (statuslog)
            {
            }
            statuslog = true;
            accessed(path); // write log
            statuslog = false;

            // if directory, access using thread
            if (isDir(path))
            {
                char const_path[MAX_PATH_CHR];
                strcpy(const_path, path);
                pthread_create(&t_id[numthread], NULL, *listFilesRecursively, (void *)const_path);
                numthread++;
            }
            // if file, check the extention
            else
            {
                // get file extension
                char *ext = (char *)malloc(sizeof(char) * 10);
                ext = strrchr(dp->d_name, '.');

                // dont have extension, copy to folder others
                if (ext == NULL)
                {
                    // log copy to folder other
                    while (statusext[numext] || statuslog)
                    {
                    }
                    statusext[numext] = true;
                    statuslog = true;
                    move(path, "categorized/other", "other");
                    numfile[numext]++;
                    statusext[numext] = false;
                    statuslog = false;
                }
                else
                {
                    // search extension
                    idxExt = searchExt(ext + 1);

                    while (statusext[idxExt] || statuslog)
                    {
                    }
                    statusext[idxExt] = true;
                    statuslog = true;
                    // initialize destPath
                    if (idxExt == numext)
                        sprintf(destPath, "categorized/%s", extension[idxExt]);
                    else if (numfile[idxExt] < maxfile)
                        sprintf(destPath, "categorized/%s", extension[idxExt]);
                    // if it isn't first directory
                    else
                    {
                        int n = (numfile[idxExt]) / maxfile + 1;
                        sprintf(destPath, "categorized/%s(%d)", extension[idxExt], n);

                        // if new directory
                        if (numfile[idxExt] % maxfile == 0)
                        {
                            made(destPath);
                        }
                    }
                    move(path, destPath, extension[idxExt]);
                    numfile[idxExt]++;
                    statusext[idxExt] = false;
                    statuslog = false;
                }
            }
        }
    }

    for (int i = 0; i < numthread; i++)
        pthread_join(t_id[i], NULL);
    closedir(dir);
}

void *madeCategorized()
{
    // log made categorized folder
    while (statuslog)
    {
    }
    statuslog = true;
    made("categorized");
    statuslog = false;
}

void *createFolder(void *path)
{
    FILE *fptr;
    char *ext_path = (char *)malloc(sizeof(char) * 100);
    ext_path = (char *)path;
    fptr = fopen(ext_path, "r");

    // log accessed categorized
    while (statuslog)
    {
    }
    statuslog = true;
    accessed("categorized");
    statuslog = false;

    // scan extension in extensions.txt and make folder
    while (fscanf(fptr, "%s", extension[numext]) > 0)
    {
        char cmd[MAX_COMMAND], rel_path[MAX_PATH_CHR];
        strcpy(rel_path, "categorized/");
        strcat(rel_path, extension[numext]);

        // make extension folder
        while (statuslog)
        {
        }
        statuslog = true;
        made(rel_path);
        statuslog = false;

        numext++;
    }
    strcpy(extension[numext], "other");

    // make folder 'other'
    while (statuslog)
    {
    }
    statuslog = true;
    made("categorized/other");
    statuslog = false;
    fclose(fptr);
}

int main()
{
    pthread_t t_id[3];
    FILE *fptr;
    char *path = (char *)malloc(sizeof(char) * 50);
    path = "/home/thoriqaafif/sisop/hehe";
    int i = 0;
    chdir(path);

    // make categorized folder
    pthread_create(&t_id[0], NULL, *madeCategorized, NULL);
    pthread_join(t_id[0], NULL);

    // get extention name
    char ext_path[50];
    strcpy(ext_path, path);
    strcat(ext_path, "/extensions.txt");

    // make extention folder
    pthread_create(&t_id[1], NULL, *createFolder, (void *)ext_path);
    pthread_join(t_id[1], NULL);

    // get max file in extension folder
    char max_path[50];
    strcpy(max_path, path);
    strcat(max_path, "/max.txt");
    fptr = fopen(max_path, "r");
    fscanf(fptr, "%d", &maxfile);
    fclose(fptr);

    // set extention folder acces status to false
    memset(statusext, false, sizeof(statusext));

    // set number file in extension folder to 0
    memset(numfile, 0, sizeof(numfile));

    // recursive directory listing using thread
    while (statuslog)
    {
    }
    statuslog = true;
    accessed("files"); // write log
    statuslog = false;
    pthread_create(&t_id[2], NULL, *listFilesRecursively, (void *)"files");

    pthread_join(t_id[2], NULL);

    int num = 0;
    for (int i = 0; i <= numext; i++)
    {
        printf("%s : %d\n", extension[i], numfile[i]);
        num += numfile[i];
    }
    printf("Jumlah File: %d\n", num);

    return 0;
}