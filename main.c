#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "encrpyt_decrypt.h"

#define MAX 128

int string2chars(char temp[MAX]);

int main(int argc, char *argv[])
{
    char line[MAX];
    char userPassword[MAX];

    printf("Password: ");

    fgets(line, 128, stdin);
    line[strlen(line) - 1] = 0;
    sscanf(line, "%s", userPassword);

    printf("MAIN: Password Entered: %s\n", userPassword);
    string2chars(userPassword);
}