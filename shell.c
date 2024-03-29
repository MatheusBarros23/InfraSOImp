#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>
//#include <pthread.h> //Not used. Here, just for legacy
#include <fcntl.h>


#define MAX_LINE 200 /* 80 chars per line, per command */

FILE *pnt;
FILE *arqRed;

char *args[MAX_LINE/2+1]; //array de ponteiros de char
char cmd[MAX_LINE]; //user Inputs
char *texto;
char style[4]="seq";
int styleErr=0;
int cmd_count;
int cmd_count_pipe;
int cmd_count_red;
int cmd_count_redInp;
int cmd_count_redOut;
int cmd_total=0;
int cmd_total_pipe=0;
int cmd_total_red=0;
int cmd_total_redInp=0;
int cmd_total_redOut=0;
int cmd_args_count=0;
char *cmd_clean[MAX_LINE];
int execWorked=0;
int batchCheck=0;
char lastCmd[MAX_LINE/2+1]="";
int histVerify=0;
int exitCheck =0;
int should_run = 1;        /* flag to help exit program*/
int execvpPar_count=0; /* flag to help count the execvpar calls*/
int execvpPipe_count=0; /*Help pipe behavor*/
char *argv_p[MAX_LINE/2+1];
char *argv_pipe[MAX_LINE/2+1];
char *argv_pipe2[MAX_LINE/2+1];
char *argv_Red[MAX_LINE/2+1];
char *argv_Red2[MAX_LINE/2+1];
char *argv_RedInp[MAX_LINE/2+1];
char *argv_RedInp2[MAX_LINE/2+1];
char *argv_RedOut[MAX_LINE/2+1];
char *argv_RedOut2[MAX_LINE/2+1];
char *read_msg;
int h=0; //para contar linhas
int retNoCmd;


//lokar o contador para não dar prob quando passar as strings!!
//pthread_mutex_t lock; //Not needed anymore

//PASSAR OS DADOS DO PARALELO POR ESTRUTURA!!!

typedef struct
{
    int size;
    char* cmds[MAX_LINE/2+1];
}Argv_ParStruct;

typedef struct
{
    int size;
    char* cmds[MAX_LINE/2+1];
}Argv_PIPEStruct;

typedef struct
{
    int size;
    char* cmds[MAX_LINE/2+1];
}Argv_RedInpStruct;

typedef struct
{
    int size;
    char* cmds[MAX_LINE/2+1];
}Argv_RedOutStruct;


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
            string[i] = 0;
            cmd_total++;
        }
        //  printf("array[i]: %s %i\n",*array,i);
    }
    return array;
}

//Posso usar a de cima e só passar o delimitador que preciso! Fica mais legal a refatoracao
char **splitStringPipe(char *string, int *cmdCountPipe) { //legal fazer isso só antes da execução!
    char delim ='|';
    *cmdCountPipe =1;
    char **array = malloc(*cmdCountPipe * sizeof(char *)); //criar dinamicamente array
    array[0] = &string[0]; //armazena no array a posicao de cada comando
    for (int i = 1; string[i] != 0; i++) {
        if (string[i] == delim) {
            (*cmdCountPipe)++;
            array = realloc(array, sizeof(char *) * (*cmdCountPipe));
            array[(*cmdCountPipe) - 1] = &string[i + 1];
            string[i] = 0;
            cmd_total_pipe++;
        }
        //  printf("array[i]: %s %i\n",*array,i);
    }
    return array;
}

char **splitStringRed(char *string, int *cmdCountRed) { //legal fazer isso só antes da execução!
    char delim ='>';
    *cmdCountRed =1;
    char **array = malloc(*cmdCountRed * sizeof(char *)); //criar dinamicamente array
    array[0] = &string[0]; //armazena no array a posicao de cada comando
    for (int i = 1; string[i] != 0; i++) {
        if (string[i] == delim) {
            (*cmdCountRed)++;
            array = realloc(array, sizeof(char *) * (*cmdCountRed));
            array[(*cmdCountRed) - 1] = &string[i + 1];
            string[i] = 0;
            cmd_total_red++;
        }
          //printf("array[i]: %s %i\n",*array,i);
    }

    return array;
}

char **splitStringRedInp(char *string, int *cmdCountRedInp) { //legal fazer isso só antes da execução!
    char delim ='<';
    *cmdCountRedInp =1;
    char **array = malloc(*cmdCountRedInp * sizeof(char *)); //criar dinamicamente array
    array[0] = &string[0]; //armazena no array a posicao de cada comando
    for (int i = 1; string[i] != 0; i++) {
        if (string[i] == delim) {
            (*cmdCountRedInp)++;
            array = realloc(array, sizeof(char *) * (*cmdCountRedInp));
            array[(*cmdCountRedInp) - 1] = &string[i + 1];
            string[i] = 0;
            cmd_total_redInp++;
        }
          //printf("array[i]: %s %i\n",*array,i);
    }
    return array;
}

char **splitStringRedOut(char *string, int *cmdCountRedOut) { //legal fazer isso só antes da execução!
    char delim ='>';
    *cmdCountRedOut =1;
    char **array = malloc(*cmdCountRedOut * sizeof(char *)); //criar dinamicamente array
    array[0] = &string[0]; //armazena no array a posicao de cada comando
    for (int i = 0; string[i] != 0; i++) {
        if (string[i] == delim) {
            (*cmdCountRedOut)++;
            array = realloc(array, sizeof(char *) * (*cmdCountRedOut));
            array[(*cmdCountRedOut) - 1] = &string[i + 1];
            string[i] = 0;
            cmd_total_redOut++;
            i--;
        }
       //   printf("array[%i]: %s \n",i,*array);
    }
   //printf("array[0]: %s \n",array[0]);
   //printf("array[1]: %s \n",array[1]);
   //printf("array[2]: %s \n",array[2]);

    return array;
}

int *styleCheck(char *input){ //tbm cmd vazio!!
    int count=0;
    if(strstr(input, "style") != NULL &&  strstr(input, "parallel") != NULL){
        printf("[style parallel]\n");
        strcpy(style, "par");
        styleErr=0;
    }else if (strstr(input, "style") != NULL &&  strstr(input, "sequential") != NULL){
        printf("[style sequential]\n");
        strcpy(style, "seq");
        styleErr=0;
    }else if(isspace(*input)==0) {
        //printf(" len %ld\n", strlen(cmd));
        for (int i = 0; i < strlen(input); ++i) {   //ISSO EVITA BUG NO PARALELOOOOOO!!!
            if (isspace(input[i]) == 0) {
                count++;
            }
        }
    }
}

int execvpSeq(char *cmds[]){
    if(strstr(cmds[0],"exi")!=NULL){
        kill(0,9);
    }
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
        if(execvp(cmds[0], cmds)<0&& strcmp(cmds[0], "!!")!=0&& strcmp(cmds[0], "style")!=0 && retNoCmd==0){
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

//PROCESSO DEFUNTO!!! - RESOLVIDO
int execvpSeqPipe(char *cmds[], char *cmds2[]){ //FUNCIONA PERFEITOO!!
    int pipefd[2];

    /* create the pipe */
    if (pipe(pipefd) == -1) {
        fprintf(stderr,"Pipe failed");
        return 1;
    }

    /* fork a child process */
    int pid1 = fork();
    if(pid1 < 0){ /* error occured */
        fprintf(stderr, "Comando falhou! Fork failed!");
        return 2;
    }

    if(pid1==0){
        dup2(pipefd[1],STDOUT_FILENO); //to write
        close(pipefd[0]);
        close(pipefd[1]); //we dont use this

        execvp(cmds[0],cmds);
    }

    /*fork a 2nd process*/
    int pid2;
    pid2 = fork();
    if(pid2 < 0){ /* error occured */
        fprintf(stderr, "Comando falhou! Fork failed! Can't Read");
        return 3;
    }

    if(pid2==0){ /*Recive process*/
        dup2(pipefd[0],STDIN_FILENO); //to read
        close(pipefd[0]);
        close(pipefd[1]);

        execvp(cmds2[0],cmds2);
    }

    //fechar os handles do processo pai
    close(pipefd[0]);
    close(pipefd[1]);

//Esperar os processos acabarem para juntar e encerrar!
    waitpid(pid1,NULL,0);
    waitpid(pid2,NULL,0);
    return 0;
}

int execvpSeqRed(char *cmds[],char *arq){

    /* fork a child process */
    int pid1 = fork();
    if(pid1 < 0){ /* error occured */
        fprintf(stderr, "Comando falhou! Fork failed!");
        return 2;
    }
    if(strstr(cmds[0],"exi")!=NULL){
        exit(0);
    }

    if(pid1==0){
        int fileRed = open(arq,O_WRONLY | O_CREAT, 0777); //abrir o arq, escrever ou cria-lo! 0777 (permissoes) - like chmod (octal)
        if(fileRed==-1){
            printf("Error creating the file\n");
        }

        /*Alterar o 1(stdout) para o arq que iremos criar!*/
        dup2(fileRed, STDOUT_FILENO); //duplica o fd do fileRed! e substitui a saida para o file!!
        close(fileRed);

        execvp(cmds[0],cmds);
    }

    waitpid(pid1,NULL,0);
    printf("Command's ""[%s]"" output saved in the end of the file: %s\n", cmds[0], arq);

    return 0;
}

int execvpSeqRedAppnd(char *cmds[],char *arq){

    /* fork a child process */
    int pid1 = fork();
    if(pid1 < 0){ /* error occured */
        fprintf(stderr, "Comando falhou! Fork failed!");
        return 2;
    }
    if(strstr(cmds[0],"exi")!=NULL){
        exit(0);
    }

    if(pid1==0){
        int fileRed = open(arq,O_RDWR | O_APPEND | O_CREAT, 0777); //abrir o arq, appenda-lo ou cria-lo! 0777 (permissoes) - like chmod (octal)
        if(fileRed==-1){
            printf("Error creating the file\n");
        }

        /*Alterar o 1(stdout) para o arq que iremos criar!*/
        dup2(fileRed, STDOUT_FILENO); //duplica o fd do fileRed! e substitui a saida para o file!!
        close(fileRed);

        execvp(cmds[0],cmds);
    }

    waitpid(pid1,NULL,0);
    printf("Command's ""[%s]"" output saved in the end of the file: %s\n", cmds[0], arq);

    return 0;
}

char execvpPar2(char *Argv_par){
   // printf("\tArgv_par[0] %s\n",Argv_par);
    if(strstr(Argv_par,"|")!=NULL){

        char **cmdsArrayPipe = splitStringPipe(Argv_par, &cmd_count_pipe);

        //para o child
        char *txt;
        txt = strtok(cmdsArrayPipe[0], " ");
        int k = 0;

        while (txt != NULL) {
            argv_pipe[k] = txt;
            txt = strtok(NULL, " ");
            k++;
        }
        argv_pipe[k] = 0; //ultima para NULL, necessidade do execvp
        //  printf("argv_pipe[0]: %s\n",argv_pipe[0]);
        //para o pai

        char *txt2;
        txt2 = strtok(cmdsArrayPipe[1], " ");
        k = 0;

        while (txt2 != NULL) {
            argv_pipe2[k] = txt2;
            txt2 = strtok(NULL, " ");
            k++;
        }
        argv_pipe2[k] = 0; //ultima para NULL, necessidade do execvp

        execvpSeqPipe(argv_pipe,argv_pipe2);
    }
    else if(strstr(Argv_par," > ")!=NULL){
        char **cmdsArrayRed = splitStringRed(Argv_par, &cmd_count_red);
        char *txt;
        txt = strtok(cmdsArrayRed[0], " ");
        int k = 0;
        while (txt != NULL) {
            argv_Red[k] = txt;
            txt = strtok(NULL, " ");
            k++;
        }

        argv_Red[k] = NULL;

        //corrigir espacamento do file de saida!
        char *txt2;
        txt2 = strtok(cmdsArrayRed[1], " ");
        k = 0;
        while (txt2 != NULL) {
            argv_Red2[k] = txt2;
            txt2 = strtok(NULL, " ");
            k++;
        }
        argv_Red2[k] = NULL;

        execvpSeqRed(argv_Red,argv_Red2[0]);
    }
    else if(strstr(Argv_par," < ")!=NULL){
        char **cmdsArrayRedInp = splitStringRedInp(Argv_par, &cmd_count_redInp);
        char *txt;
        txt = strtok(cmdsArrayRedInp[0], " ");
        int k = 0;
        while (txt != NULL) {
            argv_RedInp[k] = txt;
            txt = strtok(NULL, " ");
            k++;
        }

        argv_RedInp[k] = NULL;

        //corrigir espacamento do file de saida!
        char *txt2;
        txt2 = strtok(cmdsArrayRedInp[1], " ");
        k = 0;
        while (txt2 != NULL) {
            argv_RedInp2[k] = txt2;
            txt2 = strtok(NULL, " ");
            k++;
        }
        argv_RedInp2[k] = NULL;

        Argv_RedInpStruct argvRedInpExec = {3};

        //organizar tudo num struct especifica!
        argvRedInpExec.cmds[0] = argv_RedInp[0];
        argvRedInpExec.cmds[1] = argv_RedInp2[0];
        argvRedInpExec.cmds[2] = NULL;

        execvpPar_count++;

        execvpSeq(argvRedInpExec.cmds);
    }else if(strstr(Argv_par," >> ")!=NULL){
        char **cmdsArrayRedOut = splitStringRedOut(Argv_par, &cmd_count_redOut);

        //separar os cmd do args do 1 array!!
        char *txt;
        txt = strtok(cmdsArrayRedOut[0], " ");
        int k = 0;
        while (txt != NULL) {
            argv_RedOut[k] = txt;
            txt = strtok(NULL, " ");
            k++;
        }
        argv_RedOut[k] = NULL;

        //limpar o espaçamento + ">" que sobrou no 2 elemento do array!
        char *txt2;
        txt2 = strtok(cmdsArrayRedOut[1], "> ");
        k = 0;
        while (txt2 != NULL) {
            argv_RedOut2[k] = txt2;
            txt2 = strtok(NULL, " ");
            k++;
        }
        argv_RedOut2[k] = NULL;

        //atribuindo cada elemento ao array na struct!
        Argv_RedOutStruct argvRedOutExec = {3};
        argvRedOutExec.cmds[0] = argv_RedOut[0];
        argvRedOutExec.cmds[1] = argv_RedOut[1];
        argvRedOutExec.cmds[2] = argv_RedOut2[1];

        execvpSeqRedAppnd(argvRedOutExec.cmds, argv_RedOut2[0]);
    }
    else{
        char *txt;
        txt = strtok(Argv_par, " ");
        int k = 0;

        while (txt != NULL) {
            argv_p[k] = txt;
            txt = strtok(NULL, " ");
            k++;
        }
        argv_p[k] = 0; //ultima para NULL, necessidade do execvp}
        execvpSeq(argv_p);
    }


}
/*Deprecated! Not using Threads anymore, only fork()! */
    // Here, just to show how it was compared to now (execvpar2())

/*char execvpPar(Argv_ParStruct *Argv_par){
 //Duvida tirada com Erico, a necessidade do (void *) antes quando chamamos a thread é pelaa assinatura char * antes. Se fosse void * funcionava normal!
    if(strstr(Argv_par->cmds[execvpPar_count],"|")!=NULL){

        char **cmdsArrayPipe = splitStringPipe(Argv_par->cmds[execvpPar_count], &cmd_count_pipe);

        //para o child
        char *txt;
        txt = strtok(cmdsArrayPipe[0], " ");
        int k = 0;

        while (txt != NULL) {
            argv_pipe[k] = txt;
            txt = strtok(NULL, " ");
            k++;
        }
        argv_pipe[k] = 0; //ultima para NULL, necessidade do execvp

        //para o pai
        char *txt2;
        txt2 = strtok(cmdsArrayPipe[1], " ");
        k = 0;

        while (txt2 != NULL) {
            argv_pipe2[k] = txt2;
            txt2 = strtok(NULL, " ");
            k++;
        }
        argv_pipe2[k] = 0;
       pthread_mutex_lock(&lock);
        execvpPar_count++;
        pthread_mutex_unlock(&lock);
        execvpSeqPipe(argv_pipe,argv_pipe2);
    }
    else if(strstr(Argv_par->cmds[execvpPar_count]," > ")!=NULL){
        char **cmdsArrayRed = splitStringRed(Argv_par->cmds[execvpPar_count], &cmd_count_red);
        char *txt;
        txt = strtok(cmdsArrayRed[0], " ");
        int k = 0;
        while (txt != NULL) {
            argv_Red[k] = txt;
            txt = strtok(NULL, " ");
            k++;
        }

        argv_Red[k] = NULL;

        //corrigir espacamento do file de saida!
        char *txt2;
        txt2 = strtok(cmdsArrayRed[1], " ");
        k = 0;
        while (txt2 != NULL) {
            argv_Red2[k] = txt2;
            txt2 = strtok(NULL, " ");
            k++;
        }
        argv_Red2[k] = NULL;

        pthread_mutex_lock(&lock); //começo o lock
        execvpPar_count++;
       pthread_mutex_unlock(&lock);//termino o lock

        execvpSeqRed(argv_Red,argv_Red2[0]);
    }else if(strstr(Argv_par->cmds[execvpPar_count]," < ")!=NULL){
        char **cmdsArrayRedInp = splitStringRedInp(Argv_par->cmds[execvpPar_count], &cmd_count_redInp);
        char *txt;
        txt = strtok(cmdsArrayRedInp[0], " ");
        int k = 0;
        while (txt != NULL) {
            argv_RedInp[k] = txt;
            txt = strtok(NULL, " ");
            k++;
        }

        argv_RedInp[k] = NULL;

        //corrigir espacamento do file de saida!
        char *txt2;
        txt2 = strtok(cmdsArrayRedInp[1], " ");
        k = 0;
        while (txt2 != NULL) {
            argv_RedInp2[k] = txt2;
            txt2 = strtok(NULL, " ");
            k++;
        }
        argv_RedInp2[k] = NULL;

        Argv_RedInpStruct argvRedInpExec = {3};

        argvRedInpExec.cmds[0] = argv_RedInp[0];
        argvRedInpExec.cmds[1] = argv_RedInp2[0];
        argvRedInpExec.cmds[2] = NULL;

        pthread_mutex_lock(&lock); //começo o lock
        execvpPar_count++;
        pthread_mutex_unlock(&lock);//termino o lock

        execvpSeq(argvRedInpExec.cmds);
    }else if(strstr(Argv_par->cmds[execvpPar_count]," >> ")!=NULL){
        char **cmdsArrayRedOut = splitStringRedOut(Argv_par->cmds[execvpPar_count], &cmd_count_redOut);

        //separar os cmd do args do 1 array!!
        char *txt;
        txt = strtok(cmdsArrayRedOut[0], " ");
        int k = 0;
        while (txt != NULL) {
            argv_RedOut[k] = txt;
            txt = strtok(NULL, " ");
            k++;
        }
        argv_RedOut[k] = NULL;

        //limpar o espaçamento + ">" que sobrou no 2 elemento do array!
        char *txt2;
        txt2 = strtok(cmdsArrayRedOut[1], "> ");
        k = 0;
        while (txt2 != NULL) {
            argv_RedOut2[k] = txt2;
            txt2 = strtok(NULL, " ");
            k++;
        }
        argv_RedOut2[k] = NULL;

        //atribuindo cada elemento ao array na struct!
        Argv_RedOutStruct argvRedOutExec = {3};
        argvRedOutExec.cmds[0] = argv_RedOut[0];
        argvRedOutExec.cmds[1] = argv_RedOut[1];
        argvRedOutExec.cmds[2] = argv_RedOut2[1];

        //lock para nao ter interferencia!
        pthread_mutex_lock(&lock); //começo o lock
        execvpPar_count++;
        pthread_mutex_unlock(&lock);//termino o lock

        execvpSeqRed(argvRedOutExec.cmds, argv_RedOut2[0]);

    }
    else {
        char *txt;
        txt = strtok(Argv_par->cmds[execvpPar_count], " ");
        int k = 0;

        while (txt != NULL) {
            argv_p[k] = txt;
            txt = strtok(NULL, " ");
            k++;
        }
        argv_p[k] = 0; //ultima para NULL, necessidade do execvp}
        printf("\t%s\n",argv_p[0]);
        printf("\t%s\n",argv_p[1]);
        pthread_mutex_lock(&lock); //começo o lock
        execvpPar_count++; //var estava dando prob pq era incrementada pelas threads que chegava antes, entao solução foi lockar para cada 1 acessar
        pthread_mutex_unlock(&lock);//termino o lock

        execvpSeq(argv_p);
    }
} */



int main(int argc, char* argv[]) {
    int retorno=0;

    while (should_run < 2) {
        retNoCmd=0;
        while (argc==1){
            printf("mprb %s> ", style);
            fflush(stdout);

            //pegar o input do usuario e dividir a string
            //user input Keyboard
            fgets(cmd, MAX_LINE, stdin);
            if(cmd[0]=='\0'){
                printf("\n");
                retNoCmd=1;
                fprintf(stderr,"Error - 'Ctrl + D' not a valid command\n");
                exit(0);
            }else if(cmd[0]=='\n'){
                fprintf(stdout,"No command\n");
                retNoCmd=1;
            }else{
                cmd[strlen(cmd) - 1] = 0;
                fflush(stdout);
                retNoCmd=0;
            }

            //verificar Exit:
            if (strcmp(cmd, "exit") == 0) {
                printf("== Shell encerrado! ==\n");
                kill(0,9);
                exitCheck=1;
                should_run=3;//condicao de saida

                break;
            }


                //verificar !! :                //FAZER O HISTORICO!!
            else if (strcmp(cmd, "!!") == 0){
                if(strcmp(argv[0], "!!") != 0){
                    if(strcmp(style,"seq")==0){
                        if(strstr (lastCmd,"|") != NULL){
                            printf("last: %s\n", lastCmd);
                            char **cmdsArrayPipe = splitStringPipe(lastCmd, &cmd_count_pipe);
                            char *txt;
                            txt = strtok(cmdsArrayPipe[0], " ");
                            int k = 0;

                            while (txt != NULL) {
                                argv_pipe[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv_pipe[k] = 0; //ultima para NULL, necessidade do execvp

                            //para o pai
                            char *txt2;
                            txt2 = strtok(cmdsArrayPipe[1], " ");
                            k = 0;

                            while (txt2 != NULL) {
                                argv_pipe2[k] = txt2;
                                txt2 = strtok(NULL, " ");
                                k++;
                            }
                            argv_pipe2[k] = 0; //ultima para NULL, necessidade do execvp

                            //esta tudo aqui!!
                            execvpSeqPipe(argv_pipe,argv_pipe2);
                        }else if(strstr(lastCmd," < ")!=NULL){
                            printf("last: %s\n", lastCmd);
                            char **cmdsArrayRedInp = splitStringRedInp(lastCmd, &cmd_count_red);

                            char *txt;
                            txt = strtok(cmdsArrayRedInp[0], " ");
                            int k = 0;
                            while (txt != NULL) {
                                argv_RedInp[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv_RedInp[k] = NULL;

                            char *txt2;
                            txt2 = strtok(cmdsArrayRedInp[1], " ");
                            k = 0;
                            while (txt2 != NULL) {
                                argv_RedInp2[k] = txt2;
                                txt2 = strtok(NULL, " ");
                                k++;
                            }
                            argv_RedInp2[k] = NULL;

                            Argv_RedInpStruct argvRedInpExec = {3};

                            argvRedInpExec.cmds[0] = argv_RedInp[0];
                            argvRedInpExec.cmds[1] = argv_RedInp2[0];
                            argvRedInpExec.cmds[2] = NULL;
                            execvpSeq(argvRedInpExec.cmds);

                        }
                        else if(strstr(lastCmd," >> ")!=NULL){
                            printf("last: %s\n", lastCmd);
                            char **cmdsArrayRedOut = splitStringRedOut(lastCmd, &cmd_count_redOut);

                            char *txt;
                            txt = strtok(cmdsArrayRedOut[0], " ");
                            int k = 0;
                            while (txt != NULL) {
                                argv_RedOut[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv_RedOut[k] = NULL;

                            char *txt2;
                            txt2 = strtok(cmdsArrayRedOut[1], "> ");
                            k = 0;
                            while (txt2 != NULL) {
                                argv_RedOut2[k] = txt2;
                                txt2 = strtok(NULL, " ");
                                k++;
                            }
                            argv_RedOut2[k] = NULL;

                            Argv_RedOutStruct argvRedOutExec = {4};

                            argvRedOutExec.cmds[0] = argv_RedOut[0];
                            argvRedOutExec.cmds[1] = argv_RedOut[1];
                            argvRedOutExec.cmds[2] = argv_RedOut2[1];

                            execvpSeqRedAppnd(argvRedOutExec.cmds, argv_RedOut2[0]);

                        }else if(strstr(lastCmd," > ")!=NULL){
                            printf("last: %s\n", lastCmd);
                            char **cmdsArrayRed = splitStringRed(lastCmd, &cmd_count_red);

                            char *txt;
                            txt = strtok(cmdsArrayRed[0], " ");
                            int k = 0;
                            while (txt != NULL) {
                                argv_Red[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv_Red[k] = NULL;

                            char *txt2;
                            txt2 = strtok(cmdsArrayRed[1], " ");
                            k = 0;
                            while (txt2 != NULL) {
                                argv_Red2[k] = txt2;
                                txt2 = strtok(NULL, " ");
                                k++;
                            }
                            argv_Red2[k] = NULL;

                            execvpSeqRed(argv_Red,argv_Red2[0]);

                        }else{
                            printf("last: %s\n", lastCmd);
                            char *txt;                  //splitar o cmd dos args que recebe!! //esta dando segmentation fault!!
                            txt = strtok(lastCmd, " ");
                            int k = 0;
                            while (txt != NULL) {
                                argv[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv[k] = NULL; //ultima para NULL, necessidade do execvp
                            retorno = execvpSeq(argv);
                        }
                    }else if(strcmp(style,"par")==0){
                        printf("last: %s\n", lastCmd);
                        if(strstr (lastCmd,"|") != NULL){
                            printf("last: %s\n", lastCmd);
                            char **cmdsArrayPipe = splitStringPipe(lastCmd, &cmd_count_pipe);
                            char *txt;
                            txt = strtok(cmdsArrayPipe[0], " ");
                            int k = 0;

                            while (txt != NULL) {
                                argv_pipe[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv_pipe[k] = 0; //ultima para NULL, necessidade do execvp

                            //para o pai
                            char *txt2;
                            txt2 = strtok(cmdsArrayPipe[1], " ");
                            k = 0;

                            while (txt2 != NULL) {
                                argv_pipe2[k] = txt2;
                                txt2 = strtok(NULL, " ");
                                k++;
                            }
                            argv_pipe2[k] = 0; //ultima para NULL, necessidade do execvp

                            //esta tudo aqui!!
                            execvpSeqPipe(argv_pipe,argv_pipe2);
                        }else if(strstr(lastCmd," < ")!=NULL){
                            printf("last: %s\n", lastCmd);
                            char **cmdsArrayRedInp = splitStringRedInp(lastCmd, &cmd_count_red);

                            char *txt;
                            txt = strtok(cmdsArrayRedInp[0], " ");
                            int k = 0;
                            while (txt != NULL) {
                                argv_RedInp[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv_RedInp[k] = NULL;

                            char *txt2;
                            txt2 = strtok(cmdsArrayRedInp[1], " ");
                            k = 0;
                            while (txt2 != NULL) {
                                argv_RedInp2[k] = txt2;
                                txt2 = strtok(NULL, " ");
                                k++;
                            }
                            argv_RedInp2[k] = NULL;

                            Argv_RedInpStruct argvRedInpExec = {3};

                            argvRedInpExec.cmds[0] = argv_RedInp[0];
                            argvRedInpExec.cmds[1] = argv_RedInp2[0];
                            argvRedInpExec.cmds[2] = NULL;
                            execvpSeq(argvRedInpExec.cmds);

                        }
                        else if(strstr(lastCmd," >> ")!=NULL){
                            printf("last: %s\n", lastCmd);
                            char **cmdsArrayRedOut = splitStringRedOut(lastCmd, &cmd_count_redOut);

                            char *txt;
                            txt = strtok(cmdsArrayRedOut[0], " ");
                            int k = 0;
                            while (txt != NULL) {
                                argv_RedOut[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv_RedOut[k] = NULL;

                            char *txt2;
                            txt2 = strtok(cmdsArrayRedOut[1], "> ");
                            k = 0;
                            while (txt2 != NULL) {
                                argv_RedOut2[k] = txt2;
                                txt2 = strtok(NULL, " ");
                                k++;
                            }
                            argv_RedOut2[k] = NULL;

                            Argv_RedOutStruct argvRedOutExec = {4};

                            argvRedOutExec.cmds[0] = argv_RedOut[0];
                            argvRedOutExec.cmds[1] = argv_RedOut[1];
                            argvRedOutExec.cmds[2] = argv_RedOut2[1];

                            execvpSeqRedAppnd(argvRedOutExec.cmds, argv_RedOut2[0]);

                        }else if(strstr(lastCmd," > ")!=NULL){
                            printf("last: %s\n", lastCmd);
                            char **cmdsArrayRed = splitStringRed(lastCmd, &cmd_count_red);

                            char *txt;
                            txt = strtok(cmdsArrayRed[0], " ");
                            int k = 0;
                            while (txt != NULL) {
                                argv_Red[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv_Red[k] = NULL;

                            char *txt2;
                            txt2 = strtok(cmdsArrayRed[1], " ");
                            k = 0;
                            while (txt2 != NULL) {
                                argv_Red2[k] = txt2;
                                txt2 = strtok(NULL, " ");
                                k++;
                            }
                            argv_Red2[k] = NULL;

                            execvpSeqRed(argv_Red,argv_Red2[0]);

                        }else{
                            printf("last: %s\n", lastCmd);
                            char *txt;                  //splitar o cmd dos args que recebe!! //esta dando segmentation fault!!
                            txt = strtok(lastCmd, " ");
                            int k = 0;
                            while (txt != NULL) {
                                argv[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv[k] = NULL; //ultima para NULL, necessidade do execvp
                            retorno = execvpSeq(argv);
                        }
                       
                    }
                    if(retorno>0){
                    }
                }else if(strstr(argv[0], "!!") != NULL){
                    if(strstr (lastCmd,"|") != NULL){
                        printf("last: %s\n", lastCmd);
                        char **cmdsArrayPipe = splitStringPipe(lastCmd, &cmd_count_pipe);
                        char *txt;
                        txt = strtok(cmdsArrayPipe[0], " ");
                        int k = 0;

                        while (txt != NULL) {
                            argv_pipe[k] = txt;
                            txt = strtok(NULL, " ");
                            k++;
                        }
                        argv_pipe[k] = 0;

                        //para o pai
                        char *txt2;
                        txt2 = strtok(cmdsArrayPipe[1], " ");
                        k = 0;

                        while (txt2 != NULL) {
                            argv_pipe2[k] = txt2;
                            txt2 = strtok(NULL, " ");
                            k++;
                        }
                        argv_pipe2[k] = 0; //ultima para NULL, necessidade do execvp

                        //esta tudo aqui!!
                        execvpSeqPipe(argv_pipe,argv_pipe2);
                    }else if(strstr(lastCmd," >> ")!=NULL){
                        printf("last: %s\n", lastCmd);

                        char **cmdsArrayRedOut = splitStringRedOut(lastCmd, &cmd_count_redOut);

                        char *txt;
                        txt = strtok(cmdsArrayRedOut[0], " ");
                        int k = 0;
                        while (txt != NULL) {
                            argv_RedOut[k] = txt;
                            txt = strtok(NULL, " ");
                            k++;
                        }
                        argv_RedOut[k] = NULL;

                        char *txt2;
                        txt2 = strtok(cmdsArrayRedOut[1], "> ");
                        k = 0;
                        while (txt2 != NULL) {
                            argv_RedOut2[k] = txt2;
                            txt2 = strtok(NULL, " ");
                            k++;
                        }
                        argv_RedOut2[k] = NULL;

                        Argv_RedOutStruct argvRedOutExec = {4};

                        argvRedOutExec.cmds[0] = argv_RedOut[0];
                        argvRedOutExec.cmds[1] = argv_RedOut[1];
                        argvRedOutExec.cmds[2] = argv_RedOut2[1];

                        execvpSeqRedAppnd(argvRedOutExec.cmds, argv_RedOut2[0]);

                    }else if(strstr(lastCmd," < ")!=NULL){
                        printf("last: %s\n", lastCmd);
                        char **cmdsArrayRedInp = splitStringRedInp(lastCmd, &cmd_count_red);

                        char *txt;
                        txt = strtok(cmdsArrayRedInp[0], " ");
                        int k = 0;
                        while (txt != NULL) {
                            argv_RedInp[k] = txt;
                            txt = strtok(NULL, " ");
                            k++;
                        }
                        argv_RedInp[k] = NULL;

                        char *txt2;
                        txt2 = strtok(cmdsArrayRedInp[1], " ");
                        k = 0;
                        while (txt2 != NULL) {
                            argv_RedInp2[k] = txt2;
                            txt2 = strtok(NULL, " ");
                            k++;
                        }
                        argv_RedInp2[k] = NULL;

                        Argv_RedInpStruct argvRedInpExec = {3};

                        argvRedInpExec.cmds[0] = argv_RedInp[0];
                        argvRedInpExec.cmds[1] = argv_RedInp2[0];
                        argvRedInpExec.cmds[2] = NULL;
                        execvpSeq(argvRedInpExec.cmds);

                    }else if(strstr(lastCmd," > ")!=NULL){
                        printf("last: %s\n", lastCmd);
                        char **cmdsArrayRed = splitStringRed(lastCmd, &cmd_count_red);

                        char *txt;
                        txt = strtok(cmdsArrayRed[0], " ");
                        int k = 0;
                        while (txt != NULL) {
                            argv_Red[k] = txt;
                            txt = strtok(NULL, " ");
                            k++;
                        }
                        argv_Red[k] = NULL;

                        char *txt2;
                        txt2 = strtok(cmdsArrayRed[1], " ");
                        k = 0;
                        while (txt2 != NULL) {
                            argv_Red2[k] = txt2;
                            txt2 = strtok(NULL, " ");
                            k++;
                        }
                        argv_Red2[k] = NULL;

                        execvpSeqRed(argv_Red,argv_Red2[0]);

                    }else{
                        printf("last: %s\n", lastCmd);

                        char *txt;
                        txt = strtok(lastCmd, " ");
                        int k = 0;
                        while (txt != NULL) {
                            argv[k] = txt;
                            txt = strtok(NULL, " ");
                            k++;
                        }
                        argv[k] = NULL; //ultima para NULL, necessidade do execvp
                        retorno = execvpSeq(argv);
                    }
                }
            }

            //verificar mudança de estilo!
            styleCheck(cmd);

            //Splittar os Comandos pela ;
            char **cmdsArray = splitString(cmd, &cmd_count);

//Executar os comandos!!
            //printf("CMDTOTAL: %d\n",cmd_total);
            if(strcmp(style, "seq") == 0 && strcmp(cmd,"!!")) {
                for (int i = 0; i <= cmd_total; ++i) {
                    //para separar os args de cada cmd e depois executá-los! (SEQUENTIAL) - per space!!
                    for (int j = 0; j <= cmd_args_count; ++j) {
                        if (i+1>cmd_total){
                            strcpy(lastCmd, cmdsArray[i]);   // calling strcpy function
                        }
                        if(strstr(cmdsArray[i],"|")==NULL && strstr(cmdsArray[i]," > ")==NULL && strstr(cmdsArray[i]," < ")==NULL && strstr(cmdsArray[i]," >> ")==NULL){
                            char *txt;                  //splitar o cmd dos args que recebe!! //esta dando segmentation fault!!
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
                            if (strcmp(style, "seq") == 0 && strcmp(cmd, "style") && strstr(cmdsArray[i],"|")==NULL && strstr(cmdsArray[i]," > ")==NULL && strstr(cmdsArray[i]," < ")==NULL && strstr(cmdsArray[i]," >> ")==NULL) {
                                //forkar para que o execvp nao encerre o processo atual:
                                execvpSeq(argv);
                            }//fim Sequencial!!
                        }else if(strstr(cmdsArray[i]," > ")!=NULL){
                            char **cmdsArrayRed = splitStringRed(cmdsArray[i], &cmd_count_red);

                            //separar os cmd do args
                            char *txt;
                            txt = strtok(cmdsArrayRed[0], " ");
                            int k = 0;
                            while (txt != NULL) {
                                argv_Red[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }

                            argv_Red[k] = NULL;

                            //corrigir espacamento do file de saida!
                            char *txt2;
                            txt2 = strtok(cmdsArrayRed[1], " ");
                            k = 0;
                            while (txt2 != NULL) {
                                argv_Red2[k] = txt2;
                                txt2 = strtok(NULL, " ");
                                k++;
                            }
                            argv_Red2[k] = NULL;

                            execvpSeqRed(argv_Red,argv_Red2[0]);
                            //executar como no PIPE e salvar no arquivo escrito!!

                        }else if(strstr(cmdsArray[i]," < ")!=NULL){
                            char **cmdsArrayRedInp = splitStringRedInp(cmdsArray[i], &cmd_count_red);
                            for (int i = 0; i <= cmd_count; ++i) {
                               // printf("CMDSARRAYRedInp[0]: %s\n",cmdsArrayRedInp[i]);
                            }

                            //separar os cmd do args
                            char *txt;
                            txt = strtok(cmdsArrayRedInp[0], " ");
                            int k = 0;
                            while (txt != NULL) {
                                argv_RedInp[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv_RedInp[k] = NULL;

                            for (int l = 0; l <= k; ++l) {
                             //   printf("argv_RedInp[l]: %s\n",argv_RedInp[l]);
                            }

                            //corrigir espacamento do file de saida!
                            char *txt2;
                            txt2 = strtok(cmdsArrayRedInp[1], " ");
                            k = 0;
                            while (txt2 != NULL) {
                                argv_RedInp2[k] = txt2;
                                txt2 = strtok(NULL, " ");
                                k++;
                            }
                            argv_RedInp2[k] = NULL;

                            Argv_RedInpStruct argvRedInpExec = {3};

                            argvRedInpExec.cmds[0] = argv_RedInp[0];
                            argvRedInpExec.cmds[1] = argv_RedInp2[0];
                            argvRedInpExec.cmds[2] = NULL;

                            //So falta executar o processo com o arq de input!!
                            execvpSeq(argvRedInpExec.cmds);

                        }else if(strstr(cmdsArray[i]," >> ")!=NULL){
                            char **cmdsArrayRedOut = splitStringRedOut(cmdsArray[i], &cmd_count_redOut);

                        //ainda fica com o > no arq!, preciso limpar isso + os espaços!!

                            //separar os cmd do args do 1 array!!
                            char *txt;
                            txt = strtok(cmdsArrayRedOut[0], " ");
                            int k = 0;
                            while (txt != NULL) {
                                argv_RedOut[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv_RedOut[k] = NULL;

                            for (int l = 0; l <= k; ++l) {
                                 //  printf("argv_RedOut[l]: %s\n",argv_RedOut[l]);
                            }

                            //limpar o espaçamento + ">" que sobrou no 2 elemento do array!
                                //fiz isso para manter o mesmo padrão que os demais, sei que poderia ser feito a separação aqui mesmo!
                            char *txt2;
                            txt2 = strtok(cmdsArrayRedOut[1], "> ");
                            k = 0;
                            while (txt2 != NULL) {
                                argv_RedOut2[k] = txt2;
                                txt2 = strtok(NULL, " ");
                                k++;
                            }
                            argv_RedOut2[k] = NULL;

                            for (int l = 0; l <= k; ++l) {
                              //  printf("argv_RedOut2[l]: %s\n",argv_RedOut2[l]);
                            }

                            Argv_RedOutStruct argvRedOutExec = {4};

                            argvRedOutExec.cmds[0] = argv_RedOut[0];
                            argvRedOutExec.cmds[1] = argv_RedOut[1];
                            argvRedOutExec.cmds[2] = argv_RedOut2[1];

                            execvpSeqRedAppnd(argvRedOutExec.cmds, argv_RedOut2[0]);

                        }else{

                            //preciso separar novamnente pelo | pipe
                            char **cmdsArrayPipe = splitStringPipe(cmdsArray[i], &cmd_count_pipe);

                            for (int i = 0; i <= cmd_count; ++i) {
                             //   printf("CMDSARRAYPIPE[0]: %s\n",cmdsArrayPipe[i]);
                                //splitar o cmd dos args!!
                            }
                            //para o child
                            char *txt;
                            txt = strtok(cmdsArrayPipe[0], " ");
                            int k = 0;

                            while (txt != NULL) {
                                argv_pipe[k] = txt;
                                txt = strtok(NULL, " ");
                                k++;
                            }
                            argv_pipe[k] = 0; //ultima para NULL, necessidade do execvp
                            printf("argv_pipe[0]: %s\n",argv_pipe[0]);

                            //para o pai
                            char *txt2;
                            txt2 = strtok(cmdsArrayPipe[1], " ");
                            k = 0;

                            while (txt2 != NULL) {
                                argv_pipe2[k] = txt2;
                                txt2 = strtok(NULL, " ");
                                k++;
                            }
                            argv_pipe2[k] = 0; //ultima para NULL, necessidade do execvp

                            //esta tudo aqui!!
                            execvpSeqPipe(argv_pipe,argv_pipe2);
                        }
                    }
                }//fim do for sequencial!!
                cmd_args_count=0;
                cmd_total=0;
                histVerify=0;
            }

                //inicio Paralelo
            else if (strcmp(style, "par") == 0 && strcmp(cmd,"!!") && retNoCmd==0) {
            
                //  printf("AGORA PARALLELO: %s\n",argv[0]);
                
                //inicializando a struct para armazenar os dados! Conforme o tamanho
                Argv_ParStruct argvPar = {cmd_total};

                for (int l = 0; l < cmd_count; ++l) {
                    argvPar.cmds[l] = cmdsArray[l]; //problema aqui é que estou passando todos os comandos!! peciso fazer com que cada comando seja passado para sua thread!!    
                }

                //for para analisar o struct criado!!
                for (int l = 0; l < cmd_count; ++l) {
                  // printf("\tparalelo -  argvPar->cmds - %s\n", argvPar.cmds[l]);
                   if (l+1>=cmd_count)
                   {
                        strcpy(lastCmd,argvPar.cmds[l]);
                   }
                   
                } 


                for (int i = 0; i < cmd_count; ++i) {
                    // printf("cmd_count: %d\n",cmd_count);
                    int pid1 = fork();

                    if (pid1<0){
                        fprintf(stderr, "Comando falhou! Fork failed!");
                        return 2;
                    }

                    if(pid1 == 0)
                    {
                      //  printf("process: %d\n",i);
                        execvpPar2(argvPar.cmds[i]);
                        exit(0);
                    }
                }

                for(int i=0;i<cmd_count;i++){
                    wait(NULL);
                }
            }
            cmd_args_count=0;
            cmd_total=0;
            histVerify=0;

        }
        while (argc>1 && argc<=3 && strstr(argv[1]," < ")==NULL ){ //para nao entrar
            //peguei o tamanho!!
            // agora pegar o nome do arq -- PEGUEI -- argv[1]
            printf("Get inputs in file: %s\n",argv[1]);

           /*Corrigir quando nao encotra o arq!!*/
            pnt=fopen(argv[1],"r");
            if (pnt == NULL )
            {
                printf("Arquivo não existe! Tente novamente\n");
                exit(0);
            }
            fclose(pnt);

            char *cmdString = malloc(MAX_LINE * sizeof(char *)); //criar dinamicamente array
            char c, letra='\n';
            int linhas=0;

            fopen(argv[1],"r");
            //Lendo o arquivo 1 por 1
            while(fread (&c, sizeof(char), 1, pnt)) {
                if(c == letra) {
                    linhas++;
                }
            }
            fclose(pnt);

            pnt=fopen(argv[1],"r");
            while (fscanf(pnt, "%[^\n] ", cmdString) != EOF ){
                h++;
                if(h+1>linhas+1){
                    exit(0);
                }

                cmdString[strlen(cmdString) -1] =0; //posso concatenar com exit! para resolver!

                if(strstr(cmdString, "exi")!=NULL){
                    printf("mprb %s> exit\n", style);
                }else{
                    printf("mprb %s> %s\n", style,cmdString);
                }


                //LIMPAR A STRING!!
                styleCheck(cmdString);

                char **cmdsArray = malloc(*cmdString * sizeof(char *));
                cmdsArray = splitString(cmdString, &cmd_count);

                /*for (int i = 0; i <= cmd_total ; ++i) {
                    printf("CMD: %s\n",cmdsArray[i]);
                }*/

                // executar quando sequencial:
                if(strcmp(style, "seq") == 0 && strcmp(cmd,"!!") && strstr(cmdString, "style")==NULL) {
                    if(strstr(cmdString,"exi")!=NULL){
                        exit(0);
                    }

                    for (int i = 0; i <= cmd_total; ++i) {
                        //para separar os args de cada cmd e depois executá-los! (SEQUENTIAL)
                        for (int j = 0; j <= cmd_args_count; ++j) {
                            if(strstr(cmdsArray[i],"|")==NULL && strstr(cmdsArray[i]," > ")==NULL && strstr(cmdsArray[i]," < ")==NULL && strstr(cmdsArray[i]," >> ") == NULL){
                                char *txt;                  //splitar o cmd dos args que recebe!! //esta dando segmentation fault!!
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
                                if (strcmp(style, "seq") == 0 && strcmp(cmd, "style") && strstr(cmdsArray[i], "|") == NULL && strstr(cmdsArray[i]," > ")==NULL) {
                                    //forkar para que o execvp nao encerre o processo atual:
                                    execvpSeq(argv);
                                }
                            }
                            else if(strstr(cmdsArray[i]," > ")!=NULL){
                                char **cmdsArrayRed = splitStringRed(cmdsArray[i], &cmd_count_red);

                                char *txt;
                                txt = strtok(cmdsArrayRed[0], " ");
                                int k = 0;
                                while (txt != NULL) {
                                    argv_Red[k] = txt;
                                    txt = strtok(NULL, " ");
                                    k++;
                                }

                                argv_Red[k] = NULL;

                                //corrigir espacamento do file de saida!
                                char *txt2;
                                txt2 = strtok(cmdsArrayRed[1], " ");
                                k = 0;
                                while (txt2 != NULL) {
                                    argv_Red2[k] = txt2;
                                    txt2 = strtok(NULL, " ");
                                    k++;
                                }
                                argv_Red2[k] = NULL;

                                execvpSeqRed(argv_Red,argv_Red2[0]);

                            }else if(strstr(cmdsArray[i]," < ")!=NULL){
                                    char **cmdsArrayRedInp = splitStringRedInp(cmdsArray[i], &cmd_count_red);
                                    /*for (int i = 0; i <= cmd_count; ++i) {
                                        // printf("CMDSARRAYRedInp[0]: %s\n",cmdsArrayRedInp[i]);
                                    }*/

                                    //separar os cmd do args
                                    char *txt;
                                    txt = strtok(cmdsArrayRedInp[0], " ");
                                    int k = 0;
                                    while (txt != NULL) {
                                        argv_RedInp[k] = txt;
                                        txt = strtok(NULL, " ");
                                        k++;
                                    }
                                    argv_RedInp[k] = NULL;

                                    //corrigir espacamento do file de saida!
                                    char *txt2;
                                    txt2 = strtok(cmdsArrayRedInp[1], " ");
                                    k = 0;
                                    while (txt2 != NULL) {
                                        argv_RedInp2[k] = txt2;
                                        txt2 = strtok(NULL, " ");
                                        k++;
                                    }
                                    argv_RedInp2[k] = NULL;

                                    Argv_RedInpStruct argvRedInpExec = {3};

                                    argvRedInpExec.cmds[0] = argv_RedInp[0];
                                    argvRedInpExec.cmds[1] = argv_RedInp2[0];
                                    argvRedInpExec.cmds[2] = NULL;


                                    //So falta executar o processo com o arq de input!!
                                    execvpSeq(argvRedInpExec.cmds);
                            }else if(strstr(cmdsArray[i]," >> ") != NULL){
                                char **cmdsArrayRedOut = splitStringRedOut(cmdsArray[i], &cmd_count_redOut);
                                //ainda fica com o > no arq!, preciso limpar isso + os espaços!!

                                //separar os cmd do args do 1 array!!
                                char *txt;
                                txt = strtok(cmdsArrayRedOut[0], " ");
                                int k = 0;
                                while (txt != NULL) {
                                    argv_RedOut[k] = txt;
                                    txt = strtok(NULL, " ");
                                    k++;
                                }
                                argv_RedOut[k] = NULL;

                                //limpar o espaçamento + ">" que sobrou no 2 elemento do array!
                                //fiz isso para manter o mesmo padrão que os demais, sei que poderia ser feito a separação aqui mesmo!
                                char *txt2;
                                txt2 = strtok(cmdsArrayRedOut[1], "> ");
                                k = 0;
                                while (txt2 != NULL) {
                                    argv_RedOut2[k] = txt2;
                                    txt2 = strtok(NULL, " ");
                                    k++;
                                }
                                argv_RedOut2[k] = NULL;

                                for (int l = 0; l <= k; ++l) {
                                    //  printf("argv_RedOut2[l]: %s\n",argv_RedOut2[l]);
                                }

                                Argv_RedOutStruct argvRedOutExec = {4};

                                argvRedOutExec.cmds[0] = argv_RedOut[0];
                                argvRedOutExec.cmds[1] = argv_RedOut[1];
                                argvRedOutExec.cmds[2] = argv_RedOut2[1];

                                execvpSeqRedAppnd(argvRedOutExec.cmds, argv_RedOut2[0]);

                            }
                            else{
                                char **cmdsArrayPipe = splitStringPipe(cmdsArray[i], &cmd_count_pipe);

                                for (int i = 0; i <= cmd_count; ++i) {
                                    // printf("CMDSARRAYPIPE[0]: %s\n",cmdsArrayPipe[i]);
                                    //splitar o cmd dos args!!
                                }
                                //para o child
                                char *txt;
                                txt = strtok(cmdsArrayPipe[0], " ");
                                int k = 0;

                                while (txt != NULL) {
                                    argv_pipe[k] = txt;
                                    txt = strtok(NULL, " ");
                                    k++;
                                }
                                argv_pipe[k] = 0; //ultima para NULL, necessidade do execvp

                                //para o pai
                                char *txt2;
                                txt2 = strtok(cmdsArrayPipe[1], " ");
                                k = 0;

                                while (txt2 != NULL) {
                                    argv_pipe2[k] = txt2;
                                    txt2 = strtok(NULL, " ");
                                    k++;
                                }
                                argv_pipe2[k] = 0; //ultima para NULL, necessidade do execvp

                                //esta tudo aqui!!
                                execvpSeqPipe(argv_pipe,argv_pipe2);
                            }
                        }
                    }//fim do for sequencial!!
                    cmd_args_count=0;
                    cmd_total=0;
                    histVerify=0;
                }
                    //executar paralelo
                else if (strcmp(style, "par") == 0 && strcmp(cmd,"!!")) {
                   // Refazer! Não usar o threads!! - FEITO
                    //inicializando a struct para armazenar os dados! Conforme o tamanho
                    Argv_ParStruct argvPar = {cmd_total};

                    for (int l = 0; l < cmd_count; ++l) {
                        argvPar.cmds[l] = cmdsArray[l];
                    }

                    //execução dos COMANDOS!!
                    for (int i = 0; i < cmd_count; ++i) {
                        int pid1 = fork();

                        if (pid1<0){
                            fprintf(stderr, "Comando falhou! Fork failed!");
                            return 2;
                        }

                        if(pid1 == 0)
                        {
                            execvpPar2(argvPar.cmds[i]);
                            exit(0);
                        }
                    }

                    for(int i=0;i<cmd_count;i++){
                        wait(NULL);
                    }
                }
                cmd_args_count=0;
                cmd_total=0;
                histVerify=0;
            }
            fclose(pnt);
            //FIM PARALLELO

            //condição de saida quando não consigo abrir aqr!
            should_run=3;
            break;
        }
        if(argc>=3 && strstr(argv[1],"<")==NULL){
            fprintf(stderr,"Too many arguments! Choose the correct file\n");

            exit(0);
        }
    }
    return 0;
}
