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
int cmd_count;
char *lastCmd[MAX_LINE];
char *cmd_clean[MAX_LINE];

char **splitString(char *string, int *cmdCount) {
    *cmdCount =1;
    char **array = malloc(*cmdCount * sizeof(char *));
    array[0] = &string[0];
    for (int i = 1; string[i] != 0; i++) {
        if (string[i] == ',') {
            (*cmdCount)++;
            array = realloc(array, sizeof(char *) * (*cmdCount));
            array[(*cmdCount) - 1] = &string[i + 1];
            string[i] = '\0';
        }
    }
    return array;
}

int main() {
    int should_run = 1;        /* flag to help exit program*/
    char lastCmd[MAX_LINE];
    should_run=1;
    while (should_run){
    printf("mprb %s> ", style);
    fflush(stdout);

    //pegar o input do usuario e dividir a string
    //user input Keyboard
    fgets(cmd, MAX_LINE, stdin);
    cmd[strlen(cmd) - 1] = 0;
    fflush(stdout);

    printf("%s\n",cmd);
    }
    return 0;
}