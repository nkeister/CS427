#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <libgen.h>
#include <fcntl.h>
#include "encrpyt_decrypt.h"

#define MAX 256
#ifndef COLOR_H
#define COLOR_H

/* FANCY */
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"
#define BOLD "\033[1m"
#define R_BOLD "\x1B[31m" \
               "\033[1m"
#define B_BOLD "\033[0;30m" \
               "\033[1m"
#define BLACK "\033[0;30m"

#endif // COLOR_H

struct hostent *hp;
struct sockaddr_in server_addr;
char serstr[INET_ADDRSTRLEN];
char *paths[64];

int server_sock, r;
int SERVER_IP, SERVER_PORT;

// ------- Kerberos Struct ------------------
struct KerberosAuth
{
    int ticket_server;
    int key;
};
// ------------------------------------------

int get(void)
{
    // Client receives from server
    int i = 0, b = 0, fd, n;
    char *rec;
    char *token, hold[64], name[64], bytes[16];
    rec = (char *)malloc(sizeof(char) * MAX);
    n = read(server_sock, rec, MAX);
    if (!strcmp(rec, ""))
    {
        printf("get failed\n");
        return 0;
    }
    strcpy(hold, rec);
    token = strtok(hold, " ");
    strcpy(name, token);
    token = strtok(NULL, " ");
    strcpy(bytes, token);
    b = atoi(bytes);
    fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    free(rec);
    rec = (char *)malloc(sizeof(char) * b);
    n = read(server_sock, rec, b);
    write(fd, rec, b * sizeof(char));
    free(rec);
}

int put(void)
{
    // Client puts to server
    struct stat buf;
    char send[64], check[64];
    char *dname, *bname, *rec;
    int n, fd;
    dname = dirname(paths[1]);
    bname = basename(paths[1]);
    if (dname == NULL)
        dname = ".";
    if (stat(dname, &buf))
    {
        printf("put failed\n");
        return 0;
    }
    if (S_ISDIR(buf.st_mode))
    {
        DIR *dir = opendir(dname);
        struct dirent *file = readdir(dir);
        if (dname[strlen(dname) - 1] == '/')
            dname[strlen(dname) - 1] = '\0';
        while (file)
        {
            if (!strcmp(file->d_name, bname))
            {
                int size = buf.st_size;
                snprintf(check, sizeof(check), "%s/%s", dname, file->d_name);
                stat(check, &buf);
                snprintf(send, sizeof(send), "%s %d", file->d_name, buf.st_size);
                n = write(server_sock, send, MAX);
                snprintf(send, sizeof(send), "%s/%s", dname, bname);
                fd = open(send, O_RDONLY);
                rec = (char *)malloc(sizeof(char) * buf.st_size);
                read(fd, rec, buf.st_size);
                n = write(server_sock, rec, buf.st_size);
                free(rec);
                printf("File put\n");
                return 1;
            }
            file = readdir(dir);
        }
        n = write(server_sock, "", MAX);
        printf("put failed\n");
        return 0;
    }
}

void parse(char *input)
{
    int len, i = 0;
    char *token, *hold;
    token = strtok(input, " ");
    paths[i] = token;
    i++;
    while (token = strtok(NULL, " "))
    {
        len = strlen(token);
        hold = (char *)malloc(len * sizeof(char));
        strcpy(hold, token);
        paths[i] = hold;
        i++;
    }
    return paths;
}

int rm1(void)
{
    if (paths[1])
    {
        int i = 1;
        while (paths[i])
        {
            if (!unlink(paths[i]))
            {
                printf("rm %s successful\n", paths[i]);
            }
            else
            {
                printf("rm %s unsuccessful\n", paths[i]);
            }
            i++;
        }
    }
    else
    {
        printf("rm unsuccessful\n");
    }
}

int pwd1(void)
{
    char buf[MAX];
    getcwd(buf, MAX);
    printf("%s\n", buf);
}

int rmdir1(void)
{
    if (paths[1])
    {
        int i = 1;
        while (paths[i])
        {
            if (!rmdir(paths[i]))
            {
                printf("rmdir %s successful\n", paths[i]);
            }
            else
            {
                printf("rmdir %s unsuccessful\n", paths[i]);
            }
            i++;
        }
    }
    else
    {
        printf("rmdir unsuccessful\n");
    }
}

int mkdir1(void)
{
    if (paths[1])
    {
        int i = 1;
        while (paths[i])
        {
            if (!mkdir(paths[i], 0755))
            {
                printf("mkdir %s successful\n", paths[i]);
            }
            else
            {
                printf("mkdir %s unsuccessful\n", paths[i]);
            }
            i++;
        }
    }
    else
    {
        printf("mkdir unsuccessful\n");
    }
}

int cdir(void)
{
    int n = 0;
    char line[MAX];
    if (paths[1])
    {
        if (!chdir(paths[1]))
        {
            printf("chdir %s successful\n", paths[1]);
            n = write(server_sock, line, MAX); //write command from client to server
        }
        else
        {
            printf("chdir %s unsuccessful\n", paths[1]);
            n = write(server_sock, line, MAX); //write command from client to server
        }
    }
    else
    {
        chdir("/");
        printf("chdir / successful");
    }
}

void ls(struct dirent *file, char *pathname)
{
    char hold[64];
    char ret[128], perm[11] = {'\0'};
    struct stat buf;
    snprintf(hold, sizeof(hold), "%s/%s", pathname, file->d_name);
    if (!stat(hold, &buf))
    {
        time_t val = buf.st_mtime;
        char *mtime = ctime(&val);
        mtime[strlen(mtime) - 1] = '\0';
        strcat(perm, (S_ISDIR(buf.st_mode)) ? "d" : "-");
        strcat(perm, (buf.st_mode & S_IRUSR) ? "r" : "-");
        strcat(perm, (buf.st_mode & S_IWUSR) ? "w" : "-");
        strcat(perm, (buf.st_mode & S_IXUSR) ? "x" : "-");
        strcat(perm, (buf.st_mode & S_IRGRP) ? "r" : "-");
        strcat(perm, (buf.st_mode & S_IWGRP) ? "w" : "-");
        strcat(perm, (buf.st_mode & S_IXGRP) ? "x" : "-");
        strcat(perm, (buf.st_mode & S_IROTH) ? "r" : "-");
        strcat(perm, (buf.st_mode & S_IWOTH) ? "w" : "-");
        strcat(perm, (buf.st_mode & S_IXOTH) ? "x" : "-");
        snprintf(ret, sizeof(ret), "%s %d %d %d %s %s", perm, buf.st_nlink, buf.st_uid, buf.st_gid, mtime, file->d_name);
        printf("%s\n", ret);
    }
}

char *ls_dir(char *pathname)
{
    struct stat buf;
    char hold[64];
    char ret[2048];
    if (pathname == NULL)
        pathname = ".";
    if (stat(pathname, &buf))
    {
        strcpy(hold, "ls failed\n");
        return hold;
    }
    if (S_ISDIR(buf.st_mode))
    {
        DIR *dir = opendir(pathname);
        struct dirent *file = readdir(dir);
        if (pathname[strlen(pathname) - 1] == '/')
            pathname[strlen(pathname) - 1] = '\0';
        while (file)
        {
            ls(file, pathname);
            file = readdir(dir);
        }
    }
}
// clinet initialization code

int client_init(char *argv[])
{
    printf("======= client init ==========\n");

    printf("1 : get server info\n");
    hp = gethostbyname(argv[1]);
    if (hp == 0)
    {
        printf("unknown host %s\n", argv[1]);
        exit(1);
    }

    SERVER_IP = *(long *)hp->h_addr;
    SERVER_PORT = atoi(argv[2]);

    printf("2 : create a TCP socket\n");
    /*
        A server must create a socket and bind it with a sockaddr containing 
        the server’s IP address and port number. In order to communicate with 
        a server, a client must create a socket. For UPD sockets, binding the
        socket to a server address is optional. socket() system call, which 
        creates a socket and returns a file descriptor
    */
    server_sock = socket(AF_INET, SOCK_STREAM, 0); //create socket for sending/receiving
    if (server_sock < 0)
    {
        printf("socket call failed\n");
        exit(2);
    }

    printf("3 : fill server_addr with server's IP and PORT#\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = SERVER_IP;
    server_addr.sin_port = htons(SERVER_PORT);

    // Connect to server
    printf("4 : connecting to server ....\n");
    /*
        connect() system call connects the socket referred to by the file 
        descriptor sockfd to the address specified by addr, and the addrlen 
        argument specifies the size of addr
    */
    r = connect(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)); //connect to server
    if (r < 0)
    {
        printf("connect failed\n");
        exit(1);
    }

    printf("5 : connected OK to \007\n");
    printf("---------------------------------------------------------\n");
    inet_ntop(AF_INET, &SERVER_IP, serstr, INET_ADDRSTRLEN);
    printf(B_BOLD "hostname=%s  IP=%s  PORT=%d\n" RESET,
           hp->h_name, serstr, SERVER_PORT);
    printf("---------------------------------------------------------\n");

    printf("========= init done ==========\n");
}

//generate random 5 digit token number and return
int random_num(int lower, int upper, int count)
{
    int num;
    for (int i = 0; i < count; i++)
    {
        num = (rand() % (upper - lower + 1)) + lower;
    }
    return num;
}

int delay()
{
    int countdelay = 199999999;
    for (int i = 0; i < countdelay; i++)
    {
        ;
    }
}

main(int argc, char *argv[])
{
    int n, i = 0;
    char line[MAX], ans[MAX], hold[MAX];
    char checkuser[MAX], line2[MAX];
    char *hostname;

    /* ------------ Confirm Users password ------------ */
    char line9[MAX];
    char userPassword9[MAX];

    printf("Password: ");

    fgets(line9, 128, stdin);
    line9[strlen(line9) - 1] = 0;
    sscanf(line9, "%s", userPassword9);

    printf(R_BOLD "MAIN: Password Entered: %s\n" RESET, userPassword9);
    string2chars(userPassword9);
    /* ----------------------------------------------- */

    if (argc < 2)
        hostname = "localhost";
    else
        hostname = argv[1];

    if (argc < 3)
    {
        printf("Usage : client ServerName localhost\n");
        exit(1);
    }

    client_init(argv);

    //printf("Enter Password: ");
    // ---------------------------------------------------------------
    // sock <---> server
    while (1)
    {
        printf("*********************************************************\n");
        printf("*\tget\tls\tmkdir\trmdir\tcd\tpwd\t*\n*\tput\tlls\tlmkdir\tlrmdir\tlcd\tlpwd\t*\n");
        printf("*********************************************************\n");
        printf("DEBUG 4.5 ---- %x\n", line);
        printf(BLACK "input a line : " RESET);
        bzero(line, MAX);        // zero out line[ ]
        fgets(line, MAX, stdin); // get a line (end with \n) from stdin

        printf("DEBUD 5 ---- %s\n", line);

        line[strlen(line) - 1] = 0; // kill \n at end
        if (line[0] == 0)           // exit if NULL line
            exit(0);
        printf("DEBUG 6\n");
        strcpy(hold, line); //copy string to hold[] destination
        // Send ENTIRE line to server
        n = write(server_sock, line, MAX); //write command from client to server
        printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

        // Read a line from sock and show it
        while (1)
        {
            n = read(server_sock, ans, MAX); //read server socket
            printf("%s\n", ans);
            if (!strcmp(ans, ""))
            {
                printf("End of command\n");
                break;
            }
            else if (!strcmp(ans, "put") || !strcmp(ans, "get") || !strcmp(ans, "lls") || !strcmp(ans, "lmkdir") || !strcmp(ans, "lpwd") || !strcmp(ans, "lrm") || !strcmp(ans, "lrmdir") || !strcmp(ans, "lcd"))
            {
                parse(hold);
                break;
            }
        }
        if (!strcmp(ans, "lls"))
            ls_dir(paths[1]);
        else if (!strcmp(ans, "lmkdir"))
            mkdir1();
        else if (!strcmp(ans, "lcd"))
            cdir();
        else if (!strcmp(ans, "lrmdir"))
            rmdir1();
        else if (!strcmp(ans, "lpwd"))
            pwd1();
        else if (!strcmp(ans, "lrm"))
            rm1();
        else if (!strcmp(ans, "get"))
            get();
        else if (!strcmp(ans, "put"))
            put();
        i = 0;
        while (paths[i])
        {
            paths[i] = NULL;
            i++;
        }
    }
}