#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX2 128

/*          COLORS           */
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define RESET "\x1B[0m"
#define R_BOLD "\x1B[31m" \
               "\033[1m"
#define B_BOLD "\033[0;30m" \
               "\033[1m"
/*                           */

int string2chars(char temp[MAX2]);
int stringbreak(char temp2[MAX2]);
int hashfunction(char temp3[MAX2]);
int token_authorization(char token2[MAX2]);

int string2chars(char temp[MAX2])
{
    int i = 0;
    int lower = 10000, upper = 99999, count = 1;
    char usertemp[MAX2];
    char s[MAX2];
    //printf(".h FILE: %s\n", temp);

    stringbreak(temp); //break string down into chars
    hashfunction(temp);

    /* ------ Generate token --------------- */
    char token[MAX2] = "1234";
    printf("USER -- %s -- connected successfully\n", usertemp);
    printf("Ticket Request Sent to Server\n");

    // --------- 2. request token and enter in ----------
    //srand(time(0));                              //use time for random num gen
    //token2 = random_num(lower, upper, count);    //get random token number
    printf(R_BOLD "token = %s\n" RESET, token); //print token number
    token_authorization(token);                 //FIX THIS

    //printf(R_BOLD "token = %d, %d\n" RESET, SERVER_PORT, hp->h_addrtype);
    printf("DEBUG 4\n");
}

int stringbreak(char temp2[MAX2])
{
    char stringofchar[128];
    char *token;
    int i = 0;

    while (temp2[i] != '\0')
    {
        stringofchar[i] = temp2[i];
        //printf("temp2: %c = %c\n", temp2[l], stringofchar[l]);
        //printf("temp2[%d]: %c\n", i, stringofchar[i]);
        i++;
    }
    return *stringofchar;
}

int hashfunction(char temp3[MAX2])
{
    int i = 0, size = 0, ascii_sum = 0;
    int ascii_dummy, salt_count, ascii_temp, n;
    char encryped[24];
    char hexencrypt[16];
    char hexencrypt2[16];
    int binary32[32];
    int binary_num[31];

    while (temp3[i] != '\0')
    {
        size++; //size of array
        i++;
    }

    //printf("sizeof: %d\n", size); //printf size of array
    if (size % 2 == 0)
    {
        printf("salt_count (EVEN): 0\n");
        salt_count = 0; //set count to 0 if password is even
    }
    else
    {
        printf("salt_count (ODD): 1\n");
        salt_count = 1; //set count to 1 if password is odd
    }
    i = 0; //reset i to 0

    while (temp3[i] != '\0')
    {
        ascii_dummy = temp3[i] + 73; //set ascii dummy number offset to 73
        printf("ASCII[%d]: %d : ASCII_dummy: %d\n", i, temp3[i], ascii_dummy);
        ascii_sum = ascii_sum + (11023 * temp3[i]); //create a ascii dummy sum digest
        i++;
    }
    ascii_sum = ascii_sum + (5 * temp3[2] - 7493);
    i = 0;

    if (ascii_sum > 4294967296)
    {
        ascii_sum = ascii_sum - 1357531; //get a new 16-bit number if too high
    }

    printf("ASCII_sum: %d\n", ascii_sum); //printf ascii value created

    /*               Convert decimal to Hex number               */
    int quotient = ascii_sum;
    int remainder, remainder2;
    int k = 0;

    while (quotient != 0)
    {
        remainder = quotient % 16;
        if (remainder < 10)
            hexencrypt[k++] = 48 + remainder;
        else
            hexencrypt[k++] = 55 + remainder;
        quotient = quotient / 16;
    }

    printf(R_BOLD "hashed password: ");
    for (int e = k - 1; e >= 0; e--)
    {
        printf("%c", hexencrypt[e]); //printf hex form of hash from [0-5]
    }

    exportpassword(hexencrypt);
    printf(RESET "\n");
    read_file(hexencrypt);
}

int token_authorization(char token2[MAX2])
{
    char token_temp[MAX2];
    char line5[MAX2];

    printf("Enter token: ");
    fgets(line5, 128, stdin);
    line5[strlen(line5) - 1] = 0;
    sscanf(line5, "%s", token_temp);

    printf("\ntoken_temp = %s   token = %s\n", token2, token_temp);
    if (strcmp(token2, line5) == 0)
    {
        printf("DEBUG 3\n");
        printf("Granted Access to Server. Token delting in\n");
        printf(R_BOLD "3...\n");
        delay();
        printf("2...\n");
        delay();
        printf("1...\n");
        delay();
        printf("Connected to Server!!\n" RESET);
        //token_temp = NULL;
    }
    else if (strcmp(token2, line5) != 0)
    {
        printf(R_BOLD "\nIncorrrect token. Shutting Down Client. \n" RESET);
        exit(0);
    }
    else
    {
        printf("\nidk\n");
    }
}

void exportpassword(char count[16])
{
    FILE *outfile = fopen("export2.txt", "w+");

    if (outfile == NULL)
    {
        printf("Unable to open file for write\n");
        exit(1);
    }

    for (int i = 0; i < sizeof(count) / sizeof(count[0]) + 2; i++)
    {
        fprintf(outfile, "%c", count[i]); //data to be exported
    }

    fclose(outfile); //close file
}

void read_file(char count2[16])
{
    FILE *infile = fopen("export2.txt", "r");
    FILE *outfile = fopen("export.txt", "r");
    char buf[16];
    char buf2[16];

    fscanf(infile, "%s", buf); //read string from file
    //printf("%s", buf);

    fscanf(outfile, "%s", buf2); //read string from file
    //printf("\n%s\n", buf2);

    if (strcmp(buf, buf2) == 0) //compare if files are the same
    {
        printf(B_BOLD "ACCESS GRANTED\n" RESET);
    }
    else if (strcmp(buf, buf2) != 0) //compare if files are different
    {
        printf(R_BOLD "INCORRECT PASSWORD! EXITING NOW\n" RESET);
        exit(0);
    }
    else
    {
        printf("no idea\n");
    }

    fclose(outfile);
    fclose(infile);
}