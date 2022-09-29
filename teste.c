#include <stdio.h>
#include <string.h>

int main()
{
    char Src[120]= "educative";
    char Dest[120] = "";
    printf("Before copying\n");
    printf("Source string: %s \n", Src);
    printf("Destination string: %s \n\n", Dest);

    strcpy(Dest, Src);   // calling strcpy function
    printf("After copying\n");
    printf("Source string: %s \n", Src);
    printf("Destination string: %s \n", Dest);
    printf("---------------------------------\n");
    Src[120]= "educasade";

    printf("Before copying\n");
    printf("Source string: %s \n", Src);
    printf("Destination string: %s \n\n", Dest);

    return 0;
}