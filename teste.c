#include <stdio.h>

#include <string.h>

int main () {

    FILE *arq;

    char c, letra = '\n';

    int vezes=0;

    arq = fopen("batch.txt","r+");

    //Lendo o arquivo 1 por 1
    while(fread (&c, sizeof(char), 1, arq)) {
        if(c == letra) {
            vezes++;
        }
    }

    printf("\nLinhas: %d\n",vezes + 1);

    fclose(arq);

}