#include <stdio.h>

#include <string.h>

int main () {

    FILE *arq;

    char c, letra = '\n';

    int vezes=0;
    char line[256];

    arq = fopen("batch.txt","r+");
    //Lendo o arquivo 1 por 1
    while(fread (&c, sizeof(char), 1, arq)) {
        if(c == letra) {
            vezes++;
        }
    }
    printf("\nLinhas: %d\n",vezes + 1);
    fclose(arq);

    arq = fopen("batch.txt","r+");
    //Lendo o arquivo 1 por 1

    for (int i = 0; i <= vezes; ++i) {
        fgets(line, sizeof(line), arq);
        printf("LINHA %s",line);
    }
    fclose(arq);



    //Lendo o arquivo 1 por 1

}