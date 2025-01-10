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