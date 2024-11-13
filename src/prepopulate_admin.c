#include"../headers/utils.h"

int main()
{
    struct User admin;
    memz(admin.username);
    memz(admin.password);
    pt("Enter the username of the admin:\n=>");
    scanf("%s",admin.username);
    pt("Enter the password of the admin:\n=>");
    scanf("%s",admin.password);
    struct flock lock;
    admin.isAdmin=1;
    lock.l_type=F_RDLCK;
    lock.l_whence=SEEK_SET;
    lock.l_start=0;
    lock.l_len=0;
    lock.l_pid=getpid();
    int fd=open("db/users.txt",O_RDWR|O_CREAT,0666);
    chk(fd,"open");
    int sz=sizeof(struct User);
    chk(fcntl(fd,F_SETLKW,&lock),"fcntl");
    struct User temp;
    while(read(fd,&temp,sz))
    {
        if(strcmp(temp.username,admin.username)==0)
        {
            pt("Admin already exists\n");
            exit(0);
        }
    }
    lock.l_type=F_UNLCK;
    chk(fcntl(fd,F_SETLKW,&lock),"fcntl");
    lseek(fd,0,SEEK_END);
    lock.l_type=F_WRLCK;
    lock.l_start=lseek(fd,0,SEEK_END);
    lock.l_len=sz;
    chk(fcntl(fd,F_SETLKW,&lock),"fcntl");
    write(fd,&admin,sz);
    lock.l_type=F_UNLCK;
    chk(fcntl(fd,F_SETLKW,&lock),"fcntl");
    close(fd);
}