
#define MAX_LINE 200 /* 80 chars per line, per command */

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

char **splitString(char *string, int *cmdCount);
char **splitStringPipe(char *string, int *cmdCountPipe);
char **splitStringRed(char *string, int *cmdCountRed);
char **splitStringRedInp(char *string, int *cmdCountRedInp);
char **splitStringRedOut(char *string, int *cmdCountRedOut);
int *styleCheck(char *input);
int execvpSeq(char *cmds[]);
int execvpSeqPipe(char *cmds[], char *cmds2[]);
int execvpSeqRed(char *cmds[],char *arq);
char execvpPar2(char *Argv_par);
