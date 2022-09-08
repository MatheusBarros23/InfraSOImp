#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define MAX_LINE 80 /* 80 chars per line, per command */

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
    char str[] = "one,two,three,quatro";
    int size=4;
    char **array = splitString(str, &size);
    for (int i=0; i < size; i++) {
        printf("%s\n", array[i]);
    }
    return 0;
}