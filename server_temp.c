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
#include "admin_create_pass.h"

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

// Define variables:
struct sockaddr_in server_addr, client_addr, name_addr;
struct hostent *hp;

char serstr[INET_ADDRSTRLEN];

char *paths[64];

int mysock, client_sock; // socket descriptors
int serverPort;          // server port number
int r, length, n;        // help variables

// ------- Kerberos Struct ------------------
struct KerberosAuth
{
    int ticket_server;
    int key;
};
// ------------------------------------------

int put(void)
{
    // server receives from client
    int i = 0, b = 0, fd;
    char *rec;
    char *token, hold[64], name[64], bytes[16];
    rec = (char *)malloc(sizeof(char) * MAX);
    n = read(client_sock, rec, MAX);
    if (!strcmp(rec, ""))
    {
        printf("put failed\n");
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
    n = read(client_sock, rec, b);
    write(fd, rec, b * sizeof(char));
    free(rec);
}

int get(void)
{
    // server puts to client
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
        printf("get failed\n");
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
                n = write(client_sock, send, MAX);
                snprintf(send, sizeof(send), "%s/%s", dname, bname);
                fd = open(send, O_RDONLY);
                rec = (char *)malloc(sizeof(char) * buf.st_size);
                read(fd, rec, buf.st_size);
                n = write(client_sock, rec, buf.st_size);
                free(rec);
                printf("File get\n");
                return 1;
            }
            file = readdir(dir);
        }
        n = write(client_sock, "", MAX);
        printf("get failed\n");
        return 0;
    }
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
                n = write(client_sock, "rm successful", MAX);
            }
            else
            {
                printf("rm %s unsuccessful\n", paths[i]);
                n = write(client_sock, "rm unsuccessful", MAX);
            }
            i++;
        }
    }
    else
    {
        printf("rm unsuccessful\n");
        n = write(client_sock, "rm unsuccessful", MAX);
    }
}

int pwd1(void)
{
    char buf[MAX];
    getcwd(buf, MAX);
    printf("%s\n", buf);
    n = write(client_sock, buf, MAX);
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
                n = write(client_sock, "rmdir successful", MAX);
            }
            else
            {
                printf("rmdir %s unsuccessful\n", paths[i]);
                n = write(client_sock, "rmdir unsuccessful", MAX);
            }
            i++;
        }
    }
    else
    {
        printf("rmdir unsuccessful\n");
        n = write(client_sock, "rmdir unsuccessful", MAX);
    }
}

int mdir1(void)
{
    if (paths[1])
    {
        int i = 1;
        while (paths[i])
        {
            if (!mkdir(paths[i], 0755))
            {
                printf("mkdir %s successful\n", paths[i]);
                n = write(client_sock, "mkdir successful", MAX);
            }
            else
            {
                printf("mkdir %s unsuccessful\n", paths[i]);
                n = write(client_sock, "mkdir unsuccessful", MAX);
            }
            i++;
        }
    }
    else
    {
        printf("mkdir unsuccessful\n");
        n = write(client_sock, "mkdir unsuccessful", MAX);
    }
}

int chdir1(void)
{
    if (paths[1])
    {
        if (!chdir(paths[1]))
        {
            printf("chdir %s successful\n", paths[1]);
            n = write(client_sock, "chdir successful", MAX);
        }
        else
        {
            printf("chdir %s unsuccessful\n", paths[1]);
            n = write(client_sock, "chdir unsuccessful", MAX);
        }
    }
    else
    {
        chdir("/");
        printf("chdir / successful");
        n = write(client_sock, "chdir successful", MAX);
    }
}

void ls(struct dirent *file, char *pathname)
{
    char hold[64];
    char ret[128], perm[11] = {'\0'};
    struct stat buf;
    snprintf(hold, sizeof(hold), "%s/%s", pathname, file->d_name);
    printf("ls %s\n", hold);
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
        n = write(client_sock, ret, MAX);
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

// Server initialization code:

int server_init(char *name)
{
    //%%%%%%%%%%%%%% get DOT name and IP address of this host %%%%%%%%%%%%%%%%%
    printf("==================== server init ======================\n");
    printf("1 : get and show server host info\n");
    hp = gethostbyname(name); //access system hostname
    if (hp == 0)
    {
        printf("unknown host\n");
        exit(1);
    }
    inet_ntop(AF_INET, hp->h_addr, serstr, INET_ADDRSTRLEN);             // convert IP binary to text form
    printf(B_BOLD "    hostname=%s  IP=%s\n" RESET, hp->h_name, serstr); //print hostname and IP
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //%%%%%%%%%%%% create a TCP socket by socket() syscall %%%%%%%%%%%%%%%%%%%%
    printf("2 : create a socket\n");
    mysock = socket(AF_INET, SOCK_STREAM, 0); //create an endpoint for communication
    if (mysock < 0)
    {
        printf("socket call failed\n");
        exit(2);
    }
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //%%%%%%%%%%%%%%%% initialize the server_addr structure %%%%%%%%%%%%%%%%%%%
    printf("3 : fill server_addr with host IP and PORT# info\n");
    server_addr.sin_family = AF_INET;                // for TCP/IP
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // THIS HOST IP address
    server_addr.sin_port = 0;                        // let kernel assign port
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //%%%%%%%%%%%%%%%% bind syscall: bind the socket to server_addr info %%%%%%
    printf("4 : bind socket to host info\n");
    /*
        bind() system call assigns the address specified by addr to the socket 
        referred to by the filedescriptor sockfd, addrlen specifies the size in 
        bytes of the address structure pointed to by addr.
    */
    r = bind(mysock, (struct sockaddr *)&server_addr, sizeof(server_addr)); //bind name to socket
    if (r < 0)
    {
        printf("bind failed\n");
        exit(3);
    }
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //%%%%%%%%%%%%%%%% find out socket port number (assigned by kernel) %%%%%%%
    printf("5 : find out Kernel assigned PORT# and show it\n");
    length = sizeof(name_addr); //get sizeof name_addr
    r = getsockname(mysock, (struct sockaddr *)&name_addr, &length);
    if (r < 0)
    {
        printf("get socketname error\n");
        exit(4);
    }

    serverPort = ntohs(name_addr.sin_port);           // convert to host ushort
    printf(B_BOLD "    Port=%d\n" RESET, serverPort); //print port number for connection
    printf("5 : server is listening ....\n");
    /*
        listen() marks the socket referred to by sockfd as a socket that will
        be used to accept incoming connection The backlog argument defines 
        the maximum queue length of pending connections.
    */
    listen(mysock, 5); //listen for clients to connect with max of 5 - TCP
    printf("===================== init done =======================\n");
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
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

main(int argc, char *argv[])
{
    char *hostname;
    char line[MAX];
    int len = 0, check = 1;
    int i = 0;

    /*      Create Temporary User Password    */ 
    char line9[MAX];
    char userPassword9[MAX];

    printf("Admin temporary Client Password: ");

    fgets(line9, 128, stdin);
    line9[strlen(line9) - 1] = 0;
    sscanf(line9, "%s", userPassword9);

    printf(R_BOLD"MAIN: Password Entered: %s\n"RESET, userPassword9);
    string2chars(userPassword9);
    /*                                  */

    if (argc < 2)
        hostname = "localhost"; //default hostname
    else
        hostname = argv[1];

    server_init(hostname); //initialied server with hostname as parameter

    while (1) // wait for client to connect
    {
        printf("server: accepting new connection ....\n");
        length = sizeof(client_addr); //sizeof client_addr
        /*
            It extracts the first connection request on the queue of pending
            connections for the listening socket, sockfd, creates a new connected
            socket, and returns a new file descriptor referring to that
            socket., which is connected with the client host.
        */
        client_sock = accept(mysock, (struct sockaddr *)&client_addr, &length); //accept client
        if (client_sock < 0)
        {
            printf("server: accept error\n");
            exit(1);
        }
        printf("server: accepted a client connection from\n");
        printf("-----------------------------------------------\n");
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, serstr, INET_ADDRSTRLEN); // convert IP binary to test form
        //printf(B_BOLD "        IP=%s  port=%d\n" RESET, serstr, ntohs(client_addr.sin_port));
        printf("-----------------------------------------------\n");

        // Processing loop: newsock <----> client
        while (1)
        {
            n = read(client_sock, line, MAX);
            if (n == 0)
            {
                printf("server: client died, server loops\n");
                close(client_sock);
                break;
            }

            // show the line string
            printf("server: read  n=%d bytes; line=[%s]\n", n, line);

            parse(line);

            if (!strcmp(paths[0], "ls"))
            {
                ls_dir(paths[1]);
                n = write(client_sock, "", MAX);
            }
            else if (!strcmp(paths[0], "mkdir"))
            {
                mdir1();
                n = write(client_sock, "", MAX);
            }
            else if (!strcmp(paths[0], "cd"))
            {
                chdir1();
                n = write(client_sock, "", MAX);
            }
            else if (!strcmp(paths[0], "rmdir"))
            {
                rmdir1();
                n = write(client_sock, "", MAX);
            }
            else if (!strcmp(paths[0], "pwd"))
            {
                pwd1();
                n = write(client_sock, "", MAX);
            }
            else if (!strcmp(paths[0], "rm"))
            {
                rm1();
                n = write(client_sock, "", MAX);
            }
            else if (!strcmp(paths[0], "lls"))
                n = write(client_sock, "lls", MAX);
            else if (!strcmp(paths[0], "lmkdir"))
                n = write(client_sock, "lmkdir", MAX);
            else if (!strcmp(paths[0], "lcd"))
                n = write(client_sock, "lcd", MAX);
            else if (!strcmp(paths[0], "lrmdir"))
                n = write(client_sock, "lrmdir", MAX);
            else if (!strcmp(paths[0], "lpwd"))
                n = write(client_sock, "lpwd", MAX);
            else if (!strcmp(paths[0], "lrm"))
                n = write(client_sock, "lrm", MAX);
            else if (!strcmp(paths[0], "get"))
            {
                n = write(client_sock, "get", MAX);
                get();
            }
            else if (!strcmp(paths[0], "put"))
            {
                n = write(client_sock, "put", MAX);
                put();
            }
            printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
            printf(BLACK "server: ready for next request\n" RESET);
            i = 0;
            while (paths[i])
            {
                paths[i] = NULL;
                i++;
            }
        }
    }
}
