#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#define MAX_LINE 80 /* 80 chars per line, per command */

char *args[MAX_LINE/2+1]; //array de ponteiros de char
char cmd[MAX_LINE]; //user Inputs
char *texto;
char style[4]="seq";
int styleErr=0;
int cmd_count;
int cmd_total=0;
int cmd_args_count=0;
char *cmd_clean[MAX_LINE];
int execWorked=0;
int batchCheck=0;
char *lastCmd[MAX_LINE]={};
int histVerify=0;
int exitCheck =0;
int should_run = 1;        /* flag to help exit program*/
int execvpPar_count=0; /* flag to help count the execvpar calls*/
char *argv_p[MAX_LINE/2+1];


//PASSAR OS DADOS DO PARALELO POR ESTRUTURA!!!

typedef struct
{
    int size;
    char* cmds[MAX_LINE/2+1];
}Argv_ParStruct;


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



int *styleCheck(char *input){ //tbm cmd vazio!!
    int count=0;
    if(strcmp(input, "style parallel") == 0){
        strcpy(style, "par");
        styleErr=0;
    }else if (strcmp(input, "style sequential") == 0){
        strcpy(style, "seq");
        styleErr=0;
    }else if(isspace(*input)==0) {
// printf(" len %ld\n", strlen(cmd));
        for (int i = 0; i < strlen(input); ++i) {
            if (isspace(input[i]) == 0) {
                count++;
            }
        }
        if(count== strlen(input) && strcmp(input,"!!")&&execWorked==0){
            //fprintf(stderr,"No commands\n");
        }
    }
}

int execvpSeq(char *cmds[]){
    pid_t pid;
    int pid1;
    /* fork a child process */
    pid = fork();
    if(pid < 0){ /* error occured */
        fprintf(stderr, "Comando falhou! Fork failed!");
        return 1;
    }
    else if(pid == 0){ /* child process */
        pid1 = getpid();
        if(execvp(cmds[0], cmds)<0&& strcmp(cmds[0], "!!")){
            fprintf(stderr,"Execvp failed: %s not a Command\n",cmds[0]);
            kill(pid1,9);
            execWorked=0;
        }else{
            return 0;
        }
    }else{ /* parent process */
        wait(NULL);
    }
}

int execvpPar(Argv_ParStruct *Argv_par){
    //splitar o cmd dos args que recebe!!
    char *txt;
    txt = strtok(Argv_par->cmds[execvpPar_count], " ");
    int k = 0;
    while (txt != NULL) {
        argv_p[k] = txt;
        txt = strtok(NULL, " ");
        k++;
    }
    argv_p[k] = NULL; //ultima para NULL, necessidade do execvp
    k = 0;

    // execvpSeq(argv_p); --> PRECISO SEPARAR ANTEEES!! " " - cmd dos args!

    execvpPar_count++;
}


int main(int argc, char* argv[]) {
    int retorno=0;

    while (should_run < 2) {
        printf("mprb %s> ", style);
        fflush(stdout);

        //pegar o input do usuario e dividir a string
        //user input Keyboard
        fgets(cmd, MAX_LINE, stdin);
        cmd[strlen(cmd) - 1] = 0;
        fflush(stdout);

        printf("CMD: %s\n", cmd);

        //verificar Exit:
        if (strcmp(cmd, "exit") == 0) {
            printf("== Shell encerrado! ==\n");
            exitCheck = 1;
            should_run = 3;//condicao de saida

            break;
        }


            //verificar !! :                //FAZER O HISTORICO!!
        else if (strcmp(cmd, "!!") == 0) {
            if (strcmp(argv[0], "!!") != 0) {
                //fprintf(stdout, "%s\n",argv[0]);
                retorno = execvpSeq(argv);                        //sequencial
                if (retorno > 0) {
                } else {
                    fprintf(stdout, "No commands\n");
                }
            }
        }

        //verificar mudança de estilo!
        styleCheck(cmd);

        //Splittar os Comandos pela ;
        char **cmdsArray = splitString(cmd, &cmd_count);

        for (int i = 0; i <= cmd_total; ++i) {
            // printf("CMDSARRAY: %s\n",cmdsArray[i]);
        }

        for (int i = 0; i <= cmd_total; ++i) {
            //para separar os args de cada cmd e depois executá-los!
            for (int j = 0; j <= cmd_args_count; ++j) {
                char *txt;                  //splitar o cmd dos args que recebe!!
                txt = strtok(cmdsArray[i], " ");
                int k = 0;

                while (txt != NULL) {
                    argv[k] = txt;
                    txt = strtok(NULL, " ");
                    k++;
                }
                argv[k] = NULL; //ultima para NULL, necessidade do execvp

                //sequencial!!
                //printf("last: %s\n",lastCmd);
                if (strcmp(style, "seq") == 0 && strcmp(cmd, "style") && styleErr == 0 && histVerify == 0 &&
                    exitCheck == 0) {
                    //forkar para que o execvp nao encerre o processo atual:
                    execvpSeq(argv);
                }//fim Sequencial!!
            }
        }//fim do for sequencial!!
        cmd_args_count=0;
        cmd_total=0;
        histVerify=0;

    }
    return 0;
}