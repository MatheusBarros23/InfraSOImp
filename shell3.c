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




#define MAX_LINE 200 /* 80 chars per line, per command */
#define READ_END	0
#define WRITE_END	1

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
char *lastCmd[MAX_LINE/2+1];
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
pthread_mutex_t lock;

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
    if(strcmp(input, "style parallel") == 0){
        printf("[style parallel]\n");
        strcpy(style, "par");
        styleErr=0;
    }else if (strcmp(input, "style sequential") == 0){
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
        //if(count== strlen(input) && strcmp(input,"!!")&&execWorked==0){
        //   fprintf(stderr,"No commands\n");
        // }
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
        //execlp("ping","ping","-c", "5","google.com",NULL); //com esse teste funciona!!
        execvp(cmds[0],cmds);
    }

    /*fork a 2nd process*/
    int pid2;
    pid2 = fork();
    if(pid2 < 0){ /* error occured */
        fprintf(stderr, "Comando falhou! Fork failed! PIPE2");
        return 3;
    }

    if(pid2==0){ /*Recive process*/
        dup2(pipefd[0],STDIN_FILENO); //to read
        close(pipefd[0]);
        close(pipefd[1]);
        //execlp("grep","grep","rtt",NULL);
        execvp(cmds2[0],cmds2);
    }

    //fechar os handles do processo pai
    close(pipefd[0]);
    close(pipefd[1]);

//posso fazer um for aqui!
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

    if(pid1==0){                 // USAR ESSE COMANDO\/
        int fileRed = open(arq,O_RDWR | O_APPEND | O_CREAT, 0777); //abrir o arq, ou cria-lo!
        if(fileRed==-1){
            printf("Error creating the file\n");
        }

        /*Alterar o 1(stdout) para o arq que iremos criar!*/
        dup2(fileRed, STDOUT_FILENO); //duplica o fd do fileRed! e substitui a saida para o file!!
        close(fileRed);

        execvp(cmds[0],cmds);
    }

//posso fazer um for aqui!
    waitpid(pid1,NULL,0);
    printf("Command's ""[%s]"" output saved in the end of the file: %s\n", cmds[0], arq);

    return 0;
}


char *execvpPar(Argv_ParStruct *Argv_par){
    // splitar cada Argv_par->cmds[execvpPar_count] em um cmd e args e chamar a execução!!
    if(strstr(Argv_par->cmds[execvpPar_count],"|")!=NULL){

        char **cmdsArrayPipe = splitStringPipe(Argv_par->cmds[execvpPar_count], &cmd_count_pipe);

        for (int i = 0; i <= cmd_count; ++i) {
            //  printf("CMDSARRAYPIPE[0]: %s\n",cmdsArrayPipe[i]);
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
        //  printf("argv_pipe[1]: %s\n",argv_pipe[1]);
        //esta tudo aqui!!
        pthread_mutex_lock(&lock); //começo o lock
        execvpPar_count++; //var estava dando prob pq era incrementada pelas threads que chegava antes, entao solução foi lockar para cada 1 acessar
        pthread_mutex_unlock(&lock);//termino o lock
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

      //organizar tudo num struct especifica!
        argvRedInpExec.cmds[0] = argv_RedInp[0];
        argvRedInpExec.cmds[1] = argv_RedInp2[0];
        argvRedInpExec.cmds[2] = NULL;

   //     printf("argvRedInpExec.cmds[0]: %s\n",argvRedInpExec.cmds[0]);
   //     printf("argvRedInpExec.cmds[1]: %s\n",argvRedInpExec.cmds[1]);
   //     printf("argvRedInpExec.cmds[2]: %s\n",argvRedInpExec.cmds[2]);

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

        pthread_mutex_lock(&lock);
        execvpSeqRed(argvRedOutExec.cmds, argv_RedOut2[0]);
        pthread_mutex_unlock(&lock);
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

        pthread_mutex_lock(&lock); //começo o lock
        execvpPar_count++; //var estava dando prob pq era incrementada pelas threads que chegava antes, entao solução foi lockar para cada 1 acessar
        pthread_mutex_unlock(&lock);//termino o lock
        //return argv_p;
        // printf("argv_p NO PIPE: %s\n",argv_p[0]);
        execvpSeq(argv_p); //posso tentar execuatar por aqui mesmo! evitar algum prob
    }
}


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
                        retorno = execvpSeq(argv);
                    }else if(strcmp(style,"par")==0){
                        retorno = execvpSeq(argv); //arrumar isso!! para aceitar tbm argv no execvpPar
                    }
                    if(retorno>0){
                    }
                }else{
                    fprintf(stdout, "No commands\n");
                }
            }

            // printf("LETRA: [%d]",cmd[0]);

            //verificar mudança de estilo!
            styleCheck(cmd);

            //Splittar os Comandos pela ;
            char **cmdsArray = splitString(cmd, &cmd_count);

            /*  for (int i = 0; i <= cmd_total; ++i) {
                   printf("CMDSARRAY: %s\n",cmdsArray[i]);
                   if(strstr(cmdsArray[i],"<")!=NULL){
                       printf("TEM REDINP \n");
                   }else{
                       printf("TEM PIPE\n");
                   }
              }
            */

//Executar os comandos!!
            //printf("CMDTOTAL: %d\n",cmd_total);
            if(strcmp(style, "seq") == 0 && strcmp(cmd,"!!")) {
                for (int i = 0; i <= cmd_total; ++i) {
                    //para separar os args de cada cmd e depois executá-los! (SEQUENTIAL) - per space!!
                    for (int j = 0; j <= cmd_args_count; ++j) {
                        //if para verificar se tem | - pipe!!
                      //  printf("cmdsArray[i] %s\n",cmdsArray[i]);
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
                                lastCmd[0]=argv[0];
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

                        //    printf("argvRedInpExec.cmds[0]: %s\n",argvRedInpExec.cmds[0]);
                        //    printf("argvRedInpExec.cmds[1]: %s\n",argvRedInpExec.cmds[1]);
                        //    printf("argvRedInpExec.cmds[2]: %s\n",argvRedInpExec.cmds[2]);

                            //So falta executar o processo com o arq de input!!
                            execvpSeq(argvRedInpExec.cmds);

                        }else if(strstr(cmdsArray[i]," >> ")!=NULL){
                            char **cmdsArrayRedOut = splitStringRedOut(cmdsArray[i], &cmd_count_redOut);
                            for (int i = 0; i < cmd_count_redOut; ++i) {
                                 //  printf("cmdsArrayRedOut[i]: %s\n",cmdsArrayRedOut[i]);
                                //splitar o cmd dos args!!
                            }

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

                            // printf("argvRedOutExec.cmds[0]: %s\n",argvRedOutExec.cmds[0]);
                            // printf("argvRedOutExec.cmds[1]: %s\n",argvRedOutExec.cmds[1]);
                            // printf("argvRedOutExec.cmds[2]: %s\n",argvRedOutExec.cmds[2]);

                            execvpSeqRed(argvRedOutExec.cmds, argv_RedOut2[0]);

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
                  //  printf("\tparalelo -  argvPar->cmds - %s\n", argvPar.cmds[l]);
                }
                //printf("paralelo -  argvPar->size- %d\n", argvPar.size);
                //execução dos COMANDOS!! (Max. 2);
                pthread_t thread1[cmd_count];
                int  t1[cmd_count];

                //char *cmdArgvPar = execvpParSep(&argvPar);
            //    printf("cmdArgvPar: %s\n",argvPar.cmds[0]); //ver issooooooo!!

                //testando criacao das threads!!
                for (int i = 0; i < cmd_count; ++i) {
                    // printf("cmd_count: %d\n",cmd_count);
                    t1[i] = pthread_create(&thread1[i], NULL, (void *) execvpPar, (void *) &argvPar); //enviar para cada um uma especifica!!;
                    if(t1[i])
                    {
                        fprintf(stderr,"Error - pthread_create() return code: %d\n", t1[i]);
                        exit(EXIT_FAILURE);//eroor
                    }
                    // printf("pthread_create() for Thread %d returns: %d\n",i,t1[i]);
                    //      printf("t1[%d]: %d\n",i,t1[i]);
                }

                /* Wait till threads are complete before main continues. */
                //FOR usado para juntar todos as threads criadas, de modo a esperar o wait da main!
                for (int i = 0; i < cmd_count; ++i) {
                    if(pthread_join(thread1[i], NULL)!=0){
                        return 2;
                    }
                    //printf("Thread %d FINISHED\n",i);
                    if(execvpPar_count>i){ //zerar para não gerar prob com outros loops
                        execvpPar_count=0;
                    }
                }
            }
            cmd_args_count=0;
            cmd_total=0;
            histVerify=0;

            //printf("ultimo %s   \n",lastCmd[1]);
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
                 //   exit(0);
                }
                //cmdString[strlen(cmdString) - 1] = 0; // ISSO RESOLVEU (pq tira o ultimo!!) /MAS GERA PROB NA ULTIMA LINHA!!
                // printf("%ld\n", strlen(cmdString));
                //printf("> [%s]\n", cmdString);

     //ARRUMAR ESSE LIXOO!! gerado - PEGAR QTD LINHAS E DEPOIS TERMINAR!!
            //    printf("\tcmdString: %s\n",cmdString);

                cmdString[strlen(cmdString) -1] =0; //posso concatenar com exit! para resolver!

                //LIMPAR A STRING!!
                styleCheck(cmdString);

                char **cmdsArray = malloc(*cmdString * sizeof(char *));
                cmdsArray = splitString(cmdString, &cmd_count);

                for (int i = 0; i <= cmd_total ; ++i) {
                 //   printf("CMDSARRAY: %s\n",cmdsArray[i]);
                }

                //Corrigir erro quando não consegue abrir o arq!

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
                                    //    printf("argv_RedInp[l]: %s\n",argv_RedInp[l]);
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

                                 //   printf("argvRedInpExec.cmds[0]: %s\n",argvRedInpExec.cmds[0]);
                                 //   printf("argvRedInpExec.cmds[1]: %s\n",argvRedInpExec.cmds[1]);
                                 //   printf("argvRedInpExec.cmds[2]: %s\n",argvRedInpExec.cmds[2]);

                                    //So falta executar o processo com o arq de input!!
                                    execvpSeq(argvRedInpExec.cmds);
                            }else if(strstr(cmdsArray[i]," >> ") != NULL){
                                char **cmdsArrayRedOut = splitStringRedOut(cmdsArray[i], &cmd_count_redOut);
                                for (int i = 0; i < cmd_count_redOut; ++i) {
                                    //  printf("cmdsArrayRedOut[i]: %s\n",cmdsArrayRedOut[i]);
                                    //splitar o cmd dos args!!
                                }

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

                                // printf("argvRedOutExec.cmds[0]: %s\n",argvRedOutExec.cmds[0]);
                                // printf("argvRedOutExec.cmds[1]: %s\n",argvRedOutExec.cmds[1]);
                                // printf("argvRedOutExec.cmds[2]: %s\n",argvRedOutExec.cmds[2]);

                                execvpSeqRed(argvRedOutExec.cmds, argv_RedOut2[0]);

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
                                // printf("argv_pipe[0]: %s\n",argv_pipe[0]);
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
                    //fim sequencial
                    //executar paralelo
                else if (strcmp(style, "par") == 0 && strcmp(cmd,"!!")) {
                   //   printf("AGORA PARALLELO: %s\n",argv[0]);

                    //inicializando a struct para armazenar os dados! Conforme o tamanho

                    Argv_ParStruct argvPar = {cmd_total};

                    for (int l = 0; l < cmd_count; ++l) {
                        argvPar.cmds[l] = cmdsArray[l];
                    }

                    //for para analisar o struct criado!!
                    for (int l = 0; l < cmd_count; ++l) {
                        //     printf("\tparalelo -  argvPar->cmds - %s\n", argvPar.cmds[l]);
                    }
                    //printf("paralelo -  argvPar->size- %d\n", argvPar.size);
                    //execução dos COMANDOS!! (Max. 2);
                    pthread_t thread1[cmd_count];
                    int  t1[cmd_count];

                    //separar aqui e depois passar tudo certinho para as thread!! já dividido!!

                    // char *cmdArgvPar = execvpParSep(&argvPar);
                    // printf("cmdArgvPar: %s\n",argvPar.cmds[0]); //ver issooooooo!!

                    //testando criacao das threads!!
                    for (int i = 0; i < cmd_count; ++i) {
                        // printf("cmd_count: %d\n",cmd_count);
                        t1[i] = pthread_create(&thread1[i], NULL, (void *) execvpPar, (void *) &argvPar); //enviar para cada um uma especifica!!;
                        if(t1[i])
                        {
                            fprintf(stderr,"Error - pthread_create() return code: %d\n", t1[i]);
                            exit(EXIT_FAILURE);//eroor
                        }
                        // printf("pthread_create() for Thread %d returns: %d\n",i,t1[i]);
                    }
                    /* Wait till threads are complete before main continues. */
                    //FOR usado para juntar todos as threads criadas, de modo a esperar o wait da main!
                    for (int i = 0; i < cmd_count; ++i) {
                        if(pthread_join(thread1[i], NULL)!=0){
                            return 2;
                        }
                        //printf("Thread %d FINISHED\n",i);
                        if(execvpPar_count>i){ //zerar para não gerar prob com outros loops
                            execvpPar_count=0;
                        }
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
            //print shell encerrado!!
            break;
        }
        if(argc>=3 && strstr(argv[1],"<")==NULL){
            fprintf(stderr,"Too many arguments! Choose the correct file\n");
            exit(0);
        }
    }
    return 0;
}

//Verificar o !! (ainda com prob quando < 2)
    //FAZER MAKEFILE!!
//BACKGROUND &


// PRECISO COLOCAR UM CHECK DO STYLE ANTES DE CADA EXECUÃO!!

/*Para execução perfeita do batch, é preciso que tenha o exit no final ou, pelo menos, uma linha vazia no final (\n)
    assim, não foi verificado nenhum erro. na execução do batch file.*/