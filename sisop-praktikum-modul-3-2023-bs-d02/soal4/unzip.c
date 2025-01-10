#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){
    char path[100]="/home/thoriqaafif/sisop";
    chdir(path);

    //download
    system("wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=1rsR6jTBss1dJh2jdEKKeUyTtTRi_0fqp' -O hehe.zip");

    //unzip
    system("mkdir hehe");
    system("unzip hehe.zip -d hehe");
}