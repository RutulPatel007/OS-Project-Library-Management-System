#ifndef UTILS_H
#define UTILS_H
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<sys/wait.h>
#include<sys/time.h>
#include<sys/resource.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<errno.h>
#include<string.h>
#include<signal.h>
#include<time.h>
#include<sys/select.h>
#include<strings.h>
#include<pthread.h>
#include<semaphore.h>
#include<sys/sem.h>
#include<stdbool.h>


#define MAX_CLIENTS 100
#define PORT 6000
#define BUF BUFSIZ
#define UBUF 100
#define memz(a) memset(a,0,sizeof(a))  // memz is a macro to zero out memory
#define pr perror
#define getl(a) scanf("%[^\n]",a)
#define pt(a) printf("%s",a)
#define chk(x,msg) if(x<0){pr(msg);exit(1);}
#define ADMIN 1
#define USER 2
#define UNAUTHORISED 401
#define DUPLICATE 402
#define BAD_REQUEST 400
#define OK 200
#define NOT_ALLOWED 403
struct User
{
    char username[100];
    char password[100];
    bool isAdmin;
};

struct Book
{
    int id;
    int copies;
    char title[100];
    char author[100];
    bool valid;
};

struct user_book
{
    int id;
    char username[100];
};

#endif