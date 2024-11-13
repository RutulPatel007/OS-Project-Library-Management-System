#include"../headers/utils.h"
#include"../headers/server.h"

sem_t mutex_book[MAX_BOOKS];
sem_t mutex_book_whole;
sem_t mutex_user;
sem_t mutex_usr_book;

int authentication(int nsd)
{
    struct User temp;
    read(nsd,&temp,sizeof(struct User));
    struct flock lock;
    lock.l_type=F_RDLCK;
    lock.l_whence=SEEK_SET;
    lock.l_start=0;
    lock.l_len=0;
    lock.l_pid=getpid();
    int fd=open("db/users.txt",O_RDONLY|O_CREAT,0666);
    chk(fd,"open");
    int sz=sizeof(struct User);
    chk(fcntl(fd,F_SETLKW,&lock),"fcntl");
    printf("Username: %s\n",temp.username);
    printf("Password: %s\n",temp.password);
    struct User user;
    int flag=0;
    while(read(fd,&user,sz))
    {
        if(strcmp(user.username,temp.username)==0 && strcmp(user.password,temp.password)==0)
        {
            flag=1;
            break;
        }
    }
    lock.l_type=F_UNLCK;
    chk(fcntl(fd,F_SETLKW,&lock),"fcntl");
    close(fd);
    if(flag==1)
    {   
        int authMode=user.isAdmin?ADMIN:USER;
        write(nsd,&authMode,sizeof(int));
        return authMode;
    }
    else
    {   
        int authMode=UNAUTHORISED;
        write(nsd,&authMode,sizeof(int));
        return authMode;
    }
}

void add_new_user(int nsd)
{
    sem_wait(&mutex_user);
    struct User temp;
    read(nsd,&temp,sizeof(struct User));
    int fd=open("db/users.txt",O_RDWR|O_CREAT,0666);
    struct User user;
    while(read(fd,&user,sizeof(struct User)))
    {
        if(strcmp(user.username,temp.username)==0)
        {
            int flag=DUPLICATE;
            write(nsd,&flag,sizeof(int));
            sem_post(&mutex_user);
            return;
        }
    }
    lseek(fd,0,SEEK_END);
    temp.isAdmin=0;
    write(fd,&temp,sizeof(struct User));
    sem_post(&mutex_user);
    close(fd);
    int flag=OK;
    write(nsd,&flag,sizeof(int));
    return;
}

void print_book(struct Book *b)
{
    printf("ID: %d\n",b->id);
    printf("Title: %s\n",b->title);
    printf("Author: %s\n",b->author);
    printf("Copies: %d\n",b->copies);
    printf("Valid: %d\n",b->valid);
    printf("\n");
    printf("\n");
}

void add_book(int nsd)
{
    struct Book temp;
    bzero(&temp,sizeof(struct Book));
    read(nsd,&temp,sizeof(struct Book));
    int fd=open("db/books.txt",O_RDWR|O_CREAT,0666);
    chk(fd,"open");
    int sz=sizeof(struct Book);
    int offset=lseek(fd,0,SEEK_END);
    if(offset<=0) offset=0;
    int id=(offset/sz+1);
    chk(sem_wait(&mutex_book[id]),"sem_wait");
    temp.id=id;
    temp.valid=1;
    write(fd,&temp,sizeof(struct Book));
    print_book(&temp);
    get_all_books(fd);  
    chk(sem_post(&mutex_book[id]),"sem_post");
    write(nsd,&id,sizeof(int));
    close(fd);
}

void delete_book(int nsd)
{
    int id=-1;
    read(nsd,&id,sizeof(int));
    int fd=open("db/books.txt",O_RDWR|O_CREAT,0666);
    chk(fd,"open");
    int offset=lseek(fd,0,SEEK_END);
    if(offset<=0) offset=0;
    int sz=sizeof(struct Book);
    int maxid=(offset/sz);
    lseek(fd,0,SEEK_SET);
    if(id>maxid) 
    {
        int done=BAD_REQUEST;
        write(nsd,&done,sizeof(int));
    }
    chk(sem_wait(&mutex_book[id]),"sem_wait");
    struct Book book;
    lseek(fd,(id-1)*sizeof(struct Book),SEEK_SET);
    read(fd,&book,sizeof(struct Book));
    book.valid=0;
    book.copies=0;
    lseek(fd,(id-1)*sizeof(struct Book),SEEK_SET);
    write(fd,&book,sizeof(struct Book));
    chk(sem_post(&mutex_book[id]),"sem_post");
    get_all_books(fd);
    close(fd);
    int done=OK;
    write(nsd,&done,sizeof(int));
}

void get_all_books(int fd)
{
    struct Book book;
    lseek(fd,0,SEEK_SET);
    sem_wait(&mutex_book_whole);
    while(read(fd,&book,sizeof(struct Book)))
    {
        print_book(&book);
    }
    sem_post(&mutex_book_whole);
}

void get_all_books_nsd(int nsd)
{
    chk(sem_wait(&mutex_book_whole),"sem_wait");
    int fd=open("db/books.txt",O_RDONLY|O_CREAT,0666);
    int offset=lseek(fd,0,SEEK_END);
    if(offset<=0) offset=0;
    int sz=sizeof(struct Book);
    int maxid=(offset/sz);
    printf("Total books: %d\n",maxid);
    write(nsd,&maxid,sizeof(int));
    lseek(fd,0,SEEK_SET);
    struct Book temp;
    if(maxid==0) return;
    int i=0;
    while(read(fd,&temp,sizeof(struct Book)) && i<maxid)
    {
        write(nsd,&temp,sizeof(struct Book));
        i++;
    }
    chk(sem_post(&mutex_book_whole),"sem_post");
    close(fd);
}

void add_more_copies(int nsd)
{
    int id;
    read(nsd,&id,sizeof(int));
    int fd=open("db/books.txt",O_RDWR|O_CREAT,0666);
    chk(fd,"open");
    struct Book book;
    int offset=lseek(fd,0,SEEK_END);
    if(offset<=0) offset=0;
    int sz=sizeof(struct Book);
    int maxid=(offset/sz);
    if(id>maxid) 
    {
        int done=BAD_REQUEST;
        write(nsd,&done,sizeof(int));
        return;
    }
    sem_wait(&mutex_book[id]);
    lseek(fd,(id-1)*sizeof(struct Book),SEEK_SET);
    read(fd,&book,sizeof(struct Book));
    int copies;
    read(nsd,&copies,sizeof(int));
    book.copies+=copies;
    lseek(fd,(id-1)*sizeof(struct Book),SEEK_SET);
    write(fd,&book,sizeof(struct Book));
    sem_post(&mutex_book[id]);
    close(fd);
    int done=OK;
    write(nsd,&done,sizeof(int));
    return;
}

void remove_more_copies(int nsd)
{
    int id;
    read(nsd,&id,sizeof(int));
    int fd=open("db/books.txt",O_RDWR|O_CREAT,0666);
    chk(fd,"open");
    struct Book book;
    int offset=lseek(fd,0,SEEK_END);
    if(offset<=0) offset=0;
    int sz=sizeof(struct Book);
    int maxid=(offset/sz);
    if(id>maxid) 
    {
        int done=BAD_REQUEST;
        write(nsd,&done,sizeof(int));
        return;
    }
    sem_wait(&mutex_book[id]);
    lseek(fd,(id-1)*sizeof(struct Book),SEEK_SET);
    read(fd,&book,sizeof(struct Book));
    int copies;
    read(nsd,&copies,sizeof(int));
    if(copies>book.copies)
    {
        int done=NOT_ALLOWED;
        write(nsd,&done,sizeof(int));
        return;
    }
    book.copies-=copies;
    lseek(fd,(id-1)*sizeof(struct Book),SEEK_SET);
    write(fd,&book,sizeof(struct Book));
    sem_post(&mutex_book[id]);
    close(fd);
    int done=OK;
    write(nsd,&done,sizeof(int));
    return;
}

int admin_portal(int nsd)
{
    while (1)
    {
        int choice;
        read(nsd,&choice,sizeof(int));
        printf("Choice: %d\n",choice);
        if(choice==1)
        {
            add_new_user(nsd);
        }
        else if(choice==2)
        {
            add_book(nsd);
        }
        else if(choice==3)
        {
            delete_book(nsd);
        }
        else if(choice==4)
        {
            get_all_books_nsd(nsd);
        }
        else if(choice==5)
        {
            add_more_copies(nsd);
        }
        else if(choice==6)
        {
            remove_more_copies(nsd);
        }
        else if(choice==7)
        {
            get_all_issue_entry_nsd(nsd);
        }
        else
        {
            return 0 ;
        }
    }
    
}

void issue_book(int nsd)
{   
    struct user_book temp;
    read(nsd,&temp,sizeof(struct user_book));
    int fd1=open("db/books.txt",O_RDWR|O_CREAT,0666);
    chk(fd1,"open");
    int offset=lseek(fd1,0,SEEK_END);
    if(offset<=0) offset=0;
    int sz=sizeof(struct Book);
    int maxid=(offset/sz);
    if(temp.id>maxid) 
    {
        int flag=BAD_REQUEST;
        write(nsd,&flag,sizeof(int));
        return;
    }
    lseek(fd1,0,SEEK_SET);
    sem_wait(&mutex_book[temp.id]);
    struct Book book;
    lseek(fd1,(temp.id-1)*sizeof(struct Book),SEEK_SET);
    read(fd1,&book,sizeof(struct Book));
    if(book.copies<=0 || book.valid==0)
    {
        int flag=BAD_REQUEST;
        write(nsd,&flag,sizeof(int));
        return;
    }
    book.copies--;
    lseek(fd1,(temp.id-1)*sizeof(struct Book),SEEK_SET);
    write(fd1,&book,sizeof(struct Book));
    sem_post(&mutex_book[temp.id]);
    chk(sem_wait(&mutex_usr_book),"sem_wait");
    int fd=open("db/user_books.txt",O_RDWR|O_CREAT,0666);
    chk(fd,"open");
    struct user_book ub;
    while(read(fd,&ub,sizeof(struct user_book)))
    {
        if(strcmp(ub.username,temp.username)==0 && ub.id==temp.id)
        {
            int flag=DUPLICATE;
            write(nsd,&flag,sizeof(int));
            chk(sem_post(&mutex_usr_book),"sem_post");
            return;
        }
    }
    lseek(fd,0,SEEK_END);
    write(fd,&temp,sizeof(struct user_book));
    close(fd);
    close(fd1);
    int flag=OK;
    write(nsd,&flag,sizeof(int));
    chk(sem_post(&mutex_usr_book),"sem_post");
    get_all_issue_entry();
}       

void get_all_issue_entry()
{
    int fd=open("db/user_books.txt",O_RDONLY|O_CREAT,0666);
    chk(fd,"open");
    struct user_book ub;
    sem_wait(&mutex_usr_book);
    while(read(fd,&ub,sizeof(struct user_book)))
    {
        printf("ID: %d\n",ub.id);
        printf("Username: %s\n",ub.username);
        printf("\n");
    }
    sem_post(&mutex_usr_book);
    close(fd);

}

void return_book(int nsd)
{
    struct user_book temp;
    read(nsd,&temp,sizeof(struct user_book));
    sem_wait(&mutex_usr_book);
    int fd=open("db/user_books.txt",O_RDWR|O_CREAT,0666);
    chk(fd,"open");
    struct user_book temp1;
    struct user_book temp2;
    bzero(&temp1, sizeof(struct user_book));
    bzero(&temp2, sizeof(struct user_book));
    int flag=BAD_REQUEST;
    lseek(fd,-sizeof(struct user_book),SEEK_END);
    read(fd,&temp1,sizeof(struct user_book));
    lseek(fd,0,SEEK_SET);
    while(read(fd,&temp2,sizeof(struct user_book)))
    {
        if(strcmp(temp2.username,temp.username)==0 && temp2.id==temp.id)
        {
            flag=OK;
            lseek(fd,-sizeof(struct user_book),SEEK_CUR);
            write(fd,&temp1,sizeof(struct user_book));
            break;
        }
    }
    if(flag==BAD_REQUEST)
    {
        write(nsd,&flag,sizeof(int));
        chk(sem_post(&mutex_usr_book),"sem_post");
        return;
    }
    ftruncate(fd,lseek(fd,0,SEEK_END)-sizeof(struct user_book));
    close(fd);
    sem_post(&mutex_usr_book);
    int fd1=open("db/books.txt",O_RDWR|O_CREAT,0666);
    chk(fd1,"open");
    struct Book book;
    int id=temp.id;
    sem_wait(&mutex_book[id]);
    lseek(fd1,(id-1)*sizeof(struct Book),SEEK_SET);
    read(fd1,&book,sizeof(struct Book));
    book.copies++;
    lseek(fd1,(id-1)*sizeof(struct Book),SEEK_SET);
    write(fd1,&book,sizeof(struct Book));
    sem_post(&mutex_book[id]);
    close(fd1);
    write(nsd,&flag,sizeof(int));
    get_all_issue_entry();
    return;

}

void get_all_issue_entry_nsd(int nsd)
{
    sem_wait(&mutex_usr_book);
    int fd=open("db/user_books.txt",O_RDONLY|O_CREAT,0666);
    int fd1=open("db/books.txt",O_RDONLY|O_CREAT,0666);
    chk(fd,"open");
    int offset=lseek(fd,0,SEEK_END);
    if(offset<=0) offset=0;
    int sz=sizeof(struct user_book);
    int maxid=(offset/sz);
    write(nsd,&maxid,sizeof(int));
    lseek(fd,0,SEEK_SET);
    struct user_book temp;
    if(maxid==0) return;
    int i=0;
    while(read(fd,&temp,sizeof(struct user_book)) && i<maxid)
    {
        write(nsd,&temp,sizeof(struct user_book));
        sem_wait(&mutex_book[temp.id]);
        struct Book book;
        lseek(fd1,(temp.id-1)*sizeof(struct Book),SEEK_SET);
        read(fd1,&book,sizeof(struct Book));
        write(nsd,&book,sizeof(struct Book));
        sem_post(&mutex_book[temp.id]);
        i++;
    }
    sem_post(&mutex_usr_book);
    close(fd);

}



int client_portal(int nsd)
{
    while(1)
    {
        int choice;
        read(nsd,&choice,sizeof(int));
        printf("Choice: %d\n",choice);
        if(choice==1)   
        {
            get_all_books_nsd(nsd);
        }
        else if(choice==2)
        {
            issue_book(nsd);
        }
        else if(choice==3)
        {
            return_book(nsd);
        }
        else if(choice==4)
        {
            get_all_issue_entry_nsd(nsd);
        }
        else
        {
            return 0;
        }
    }
}


void *connection(void *args)
{
    int nsd=*(int*)args;
    int authMode=authentication(nsd);
    if(authMode==UNAUTHORISED)
    {
        close(nsd);
        return NULL;
    }
    if(authMode==ADMIN)
    {
        admin_portal(nsd);
    }
    else if(authMode==USER)
    {
        client_portal(nsd);
    }
    return NULL;

}


int main()
{   
    for(int i=0;i<MAX_BOOKS;i++)
    {
        sem_init(&mutex_book[i],0,1);
    }
    sem_init(&mutex_book_whole,0,1);
    sem_init(&mutex_user,0,1);
    sem_init(&mutex_usr_book,0,1);

    int sd,nsd;
    struct sockaddr_in serv, client;
    sd=socket(AF_INET,SOCK_STREAM,0);
    chk(sd,"socket");
    bzero(&serv,sizeof(serv));
    serv.sin_family=AF_INET;
    serv.sin_port=htons(PORT);
    serv.sin_addr.s_addr=htonl(INADDR_ANY);

    chk(bind(sd,(struct sockaddr*)&serv,sizeof(serv)),"bind");
    chk(listen(sd,MAX_CLIENTS),"listen");
    printf("Server listening on port %d\n",PORT);
    while(1)
    {
        int len=sizeof(client);
        nsd=accept(sd,(struct sockaddr*)&client,&len);
        chk(nsd,"accept");
        pthread_t tid;
        pthread_create(&tid,NULL,connection,(void*)&nsd);
    }
    close(sd);
    close(nsd);
    for(int i=0;i<MAX_BOOKS;i++)
    {
        sem_destroy(&mutex_book[i]);
    }
    sem_destroy(&mutex_user);
    sem_destroy(&mutex_book_whole);
    sem_destroy(&mutex_usr_book);
    return 0;
}