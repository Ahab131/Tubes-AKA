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