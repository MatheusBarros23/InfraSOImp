#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define MAX_LINE 80 /* 80 chars per line, per command */

char *args[MAX_LINE/2+1]; //array de ponteiros de char
char cmd[MAX_LINE]; //user Inputs
char *texto;
char style[4]="seq";

char *cmd_clean[MAX_LINE];

void execCmd(char *args, char *argv){
    //execvp(cmd, argv);
}

//remover os " " dos comandos
void removeSpace(char *string){
        if(isspace(string[strlen(string)-1])!=0){
            string[strlen(string)-1]=0;
        }
    }

void *styleCheck(char *input){
    if(strcmp(input, "style parallel") == 0){
        strcpy(style, "par");
    }else if (strcmp(input, "style sequential") == 0){
        strcpy(style, "seq");
    }
}

int main(int argc, char* argv[])
{
    int should_run = 1;		/* flag to help exit program*/
    int i = 0;
    int cmd_count=0;


    while (should_run!=2) {

    	printf("mprb %s> ",style);
        fflush(stdout);

    //pegar o input do usuario e dividir a string
        //user input Keyboard
        fgets(cmd, MAX_LINE,stdin);
        fflush(stdin);
        removeSpace(cmd);

        //remover os espaços
       /* for (int i = 0; i < MAX_LINE; ++i) {
            if(cmd[i]==' '){ //só funciona com ''
                cmd[i]='\0';
            }
        }*/

    //dividir os comandos por ;
        texto = strtok(cmd,";"); //salvar no mesmo vetor, o restante
        while(texto != NULL){
            printf("%s\n",texto);
            texto = strtok(NULL,";"); //salvar no mesmo vetor, o restante
            cmd_count++;
        }
 //revisar

        //removeSpace(*args);


    //printar os comandos!
        for (i = 0; i < cmd_count ; i++) {

            //verificar Exit:
            if (strcmp(args[i], "exit") == 0) {
                printf("== Shell encerrado! ==\n");
                break;
            }

            //verificar mudança de estilo!
            styleCheck(args[i]);
            //executar comandos - sequential

            for (int j = 0; j < cmd_count; ++j) {
                printf("%s\n",args[j]);
            }

            for (int j = 0; j < cmd_count; ++j) {
                execvp(args[j], args);
            }

        }


        //printf("%s \n",cmd);

        //condicao de Saida
        should_run++;
    }
	return 0;
}

/*
 FAZER Só com 1 comando!! para depois separa-lo!
 Pensar logo pegar o estilo! DEFAULT = Sequencial

Posso pensar em realizar um comando certinho, depois passo para outro... Assim ajuda na implementação de diferentes comandos!!

  */

