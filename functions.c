#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include "functions.h"

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
    //printf("array[0]: %s \n",array[0]);
    // printf("array[1]: %s \n",array[1]);
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
    // printf("array[0]: %s \n",array[0]);
    // printf("array[1]: %s \n",array[1]);
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

        execvpSeqRed(argvRedOutExec.cmds, argv_RedOut2[0]);
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