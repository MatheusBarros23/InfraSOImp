#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>

#define typeof(var) _Generic( (var),\
char: "Char",\
int: "Integer",\
float: "Float",\
char *: "String",\
void *: "Pointer",\
default: "Undefined")

#define MAX_LINE 80 /* 80 chars per line, per command */

char *args[MAX_LINE/2+1]; //array de ponteiros de char
char cmd[MAX_LINE]; //user Inputs
char *texto;
char style[4]="seq";
int cmd_count;

char *cmd_clean[MAX_LINE];

void execCmd(char *args, char *argv){
    //execvp(cmd, argv);
}

//remover os " " dos comandos

//splitar a string!! Tentar ver por strtok
char **splitString(char *string, int *cmdCount) {
    char delim =';';
    *cmdCount =1;
    char **array = malloc(*cmdCount * sizeof(char *)); //criar dinamicamente array
    array[0] = &string[0]; //armazena no array a posicao de cada comando
    for (int i = 1; string[i] != 0; i++) {
        if (string[i] == delim) {
            (*cmdCount)++;
            array = realloc(array, sizeof(char *) * (*cmdCount));
            array[(*cmdCount) - 1] = &string[i + 1];
            string[i] = '\0';
        }
    }
    return array;
}

char **splitStringSpace(char *string, int *cmdCount) { //posso melhorar isso ne, plmd
    char delim =' ';
    *cmdCount =1;
    char **array = malloc(*cmdCount * sizeof(char *)); //criar dinamicamente array
    array[0] = &string[0]; //armazena no array a posicao de cada comando
    for (int i = 1; string[i] != 0; i++) {
        if (string[i] == delim) {
            (*cmdCount)++;
            array = realloc(array, sizeof(char *) * (*cmdCount));
            array[(*cmdCount) - 1] = &string[i + 1];
            string[i] = '\0';
        }
    }
    return array;
}




void *styleCheck(char *input){
    if(strcmp(input, "style parallel") == 0){
        strcpy(style, "par");
    }else if (strcmp(input, "style sequential") == 0){
        strcpy(style, "seq");
    }
}


void executeSystemCall(){
    //execvp("/bin/ls", "/home");
}

int main(int argc, char* argv[])
{
    int should_run = 1;		/* flag to help exit program*/


    while (should_run!=2) {
        printf("mprb %s> ",style);
        fflush(stdout);

        //pegar o input do usuario e dividir a string
        //user input Keyboard
        fgets(cmd, MAX_LINE,stdin);
        cmd[strlen(cmd)-1]=0;
        fflush(stdout);

        char **cmdsArray = splitString(cmd, &cmd_count);


        //printar os comandos!
        for (int i = 0; i < cmd_count ; i++) {

            //verificar Exit:
            if (strcmp(cmd, "exit") == 0) {
                printf("== Shell encerrado! ==\n");
                should_run++;//condicao de saida
                break;
            }

            //verificar mudança de estilo!
            styleCheck(cmd);

            //separar os comandos em um array para usar o execvp
            //printf("%s\n",cmdsArray[i]);
            int cmdsArrayElem_count;
            char **cmdsArrayElem = splitStringSpace(cmdsArray[i], &cmdsArrayElem_count);
            for (int i=0; i < cmdsArrayElem_count; i++) {
               // printf("ELem: %s\n", cmdsArrayElem[i]);
            }
//fazendo o fork pro processo atual nao ser encerrado
    //Melhorar isso!! Só ta funfando com o primeiro cmd, tentar um for!!
    //Nao funfou!! melhorar aqui!
            pid_t pid, pid1;

            /* fork a child process */
            for (int i=0; i<cmd_count; i++){
                pid=fork();
                if(pid == 0){ /* child process */
                    pid = getpid();
                    execvp(cmdsArrayElem[i], cmdsArrayElem);
                }else{ /* parent process */
                    wait(NULL);
                }
            }
        }

    }

    return 0;
}

/*
 FAZER Só com 1 comando!! para depois separa-lo!
 Pensar logo pegar o estilo! DEFAULT = Sequencial
Posso pensar em realizar um comando certinho, depois passo para outro... Assim ajuda na implementação de diferentes comandos!!
//entender o execvp
  */