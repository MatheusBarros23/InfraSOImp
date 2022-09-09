#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>


#define MAX_LINE 80 /* 80 chars per line, per command */

char *args[MAX_LINE/2+1]; //array de ponteiros de char
char cmd[MAX_LINE]; //user Inputs
char *texto;
char style[4]="seq";
int cmd_count;
int cmd_total=0;
int cmd_args_count=0;
char *cmd_clean[MAX_LINE];
int execWorked=0;

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
            cmd_total++;
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
            cmd_args_count++;
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


int execvpSeq(char *argv[]){
    pid_t pid;
    /* fork a child process */
    pid = fork();
    if(pid < 0){ /* error occured */
        fprintf(stderr, "Comando falhou! Fork failed!");
        return 1;
    }
    else if(pid == 0){ /* child process */
        pid = getpid();
        if(execvp(argv[0], argv)<0&& strcmp(argv[0],"!!")){
            fprintf(stderr,"Execvp failed: %s not a Command\n",argv[0]);

        }

    }else{ /* parent process */
        wait(NULL);
    }
}


int main(int argc, char* argv[]) {
    int should_run = 1;        /* flag to help exit program*/
    char lastCmd[MAX_LINE];


    while (should_run < 2) {
        printf("mprb %s> ", style);
        fflush(stdout);

        //pegar o input do usuario e dividir a string
        //user input Keyboard
        fgets(cmd, MAX_LINE, stdin);
        cmd[strlen(cmd) - 1] = 0;
        fflush(stdout);

        //verificar Exit:
        if (strcmp(cmd, "exit") == 0) {
            printf("== Shell encerrado! ==\n");
            should_run=3;//condicao de saida
            break;
        }
            //verificar !! :
        else if (strcmp(cmd, "!!") == 0){
            if(strcmp(argv[0], "!!") != 0){
                fprintf(stdout, "%s  \n",argv[0]);
                execvpSeq(argv);
            }else if(execWorked==0 && execvp(argv[0], argv)<0 ){
                fprintf(stdout, "No commands\n");
            }
        }

        //verificar mudança de estilo!
        styleCheck(cmd);

        //Splittar os Comandos pela ;
        char **cmdsArray = splitString(cmd, &cmd_count);

//Executar os comandos!!
        printf("CMDTOTAL: %d\n",cmd_total);
        for (int i = 0; i <= cmd_total; ++i) {
            //printf("Agora: %s\n", cmdsArray[i]);

            //para separar os args de cada cmd e depois executá-los! (SEQUENTIAL)
            for (int j = 0; j <= cmd_args_count; ++j) {

                char *txt;                  //splitar o cmd dos args que recebe!! //esta dando segmentation fault!!
                txt = strtok(cmdsArray[i]," ");
                int j=0;
                while(txt!=NULL){
                    argv[j]=txt;
                    txt = strtok(NULL," ");
                    j++;
                }
                argv[j]=NULL; //ultima para NULL, necessidade do execvp

                //forkar para que o execvp nao encerre o processo atual:
                pid_t pid;
                /* fork a child process */
                pid = fork();
                if(pid < 0){ /* error occured */
                    fprintf(stderr, "Comando falhou! Fork failed!");
                    return 1;
                }
                else if(pid == 0){ /* child process */
                    pid = getpid();
                    if(cmdsArray[i]==NULL){

                    }else if(execvp(argv[0], argv)<0 && strcmp(argv[0],"!!")) {
                        fprintf(stderr,"Execvp failed: %s not a Command\n",argv[0]);
                    }
                }else{ /* parent process */
                    wait(NULL);
                }
            }
        }
        cmd_args_count=0;
        cmd_total=0;
    }
    return 0;
}

//fazer paralelo!! Aqui, forks e 2 filhos processando em simultaneo!!