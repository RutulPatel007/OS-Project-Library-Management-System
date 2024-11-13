#include"../headers/utils.h"
#include "../headers/client.h"

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

int admin_portal_client(int sd)
{
    printf("Choose options from below:\n");
    printf("1. Add new user\n");
    printf("2. Add new book\n");
    printf("3. Delete a book\n");
    printf("4. Get list of all books\n");
    printf("5. Add more copies of a book\n");
    printf("6. Remove copies of a book\n");
    printf("7. Get list of all users with books issued\n");
    printf("8. Exit\n");
    printf("=>\n");
    int choice;
    scanf("%d",&choice);
    write(sd,&choice,sizeof(int));

    if(choice==1)
    {
        struct User newUser;
        memz(newUser.username);
        memz(newUser.password);
        pt("Enter the username of the new user:\n=>");
        scanf("%s",newUser.username);
        pt("Enter the password of the new user:\n=>");
        scanf("%s",newUser.password);
        write(sd,&newUser,sizeof(struct User));
        int flag;
        read(sd,&flag,sizeof(int));
        if(flag==DUPLICATE)
        {
            pt("Username already exists\n");
        }
        else if(flag==OK)
        {
            pt("User added successfully\n");
        }
        return 1;
    }
    else if(choice==2)
    {
        struct Book b;
        bzero(&b,sizeof(struct Book));
        pt("Enter the title of the book:\n=>");
        scanf("%s",b.title);
        pt("Enter the author of the book:\n=>");
        scanf("%s",b.author);
        pt("Enter the number of copies of the book:\n=>");
        scanf("%d",&b.copies);
        write(sd,&b,sizeof(struct Book));
        b.valid=0;
        int id;
        read(sd,&id,sizeof(int));
        printf("Book added with book id: %d\n",id);
        return 1;
    }
    else if(choice==3)
    {
        printf("Enter the book id to delete:\n=>");
        int id;
        scanf("%d",&id);
        write(sd,&id,sizeof(int));
        int flag;
        read(sd,&flag,sizeof(int));
        if(flag==OK)
        {
            printf("Book deleted successfully\n");
        }
        else
        {
            printf("Book not found\n");
        }
        return 1;
    }
    else if(choice==4)
    {
        int max_books;
        read(sd,&max_books,sizeof(int));
        for(int i=0;i<max_books;i++)
        {
            struct Book b;
            read(sd,&b,sizeof(struct Book));
            if(b.valid==1) print_book(&b);
        }
        return 1;
    }
    else if(choice==5)
    {
        int id;
        int copies;
        printf("Enter the book id to add more copies:\n=>");
        scanf("%d",&id);
        printf("Enter the number of copies to add:\n=>");
        scanf("%d",&copies);
        if(copies<0)
        {
            printf("Invalid number of copies\n");
            return 1;
        }
        write(sd,&id,sizeof(int));
        write(sd,&copies,sizeof(int));
        int flag;
        read(sd,&flag,sizeof(int));
        if(flag==OK)
        {
            printf("Copies added successfully\n");
        }
        else
        {
            printf("Book not found\n");
        }
        return 1;
    }
    else if(choice==6)
    {
        int id;
        int copies;
        printf("Enter the book id to remove some of its copies:\n=>");
        scanf("%d",&id);
        printf("Enter the number of copies to remove:\n=>");
        scanf("%d",&copies);
        if(copies<0)
        {
            printf("Invalid number of copies\n");
            return 1;
        }
        write(sd,&id,sizeof(int));
        write(sd,&copies,sizeof(int));
        int flag;
        read(sd,&flag,sizeof(int));
        if(flag==OK)
        {
            printf("Copies removed successfully\n");
        }
        else if(flag==NOT_ALLOWED)
        {
            printf("Not enough copies to remove\n");
        }
        else
        {
            printf("Book not found\n");
        }
        return 1;
    }
    else if(choice==7)
    {
        int max_books;
        read(sd,&max_books,sizeof(int));
        for(int i=0;i<max_books;i++)
        {
            struct user_book ub;
            read(sd,&ub,sizeof(struct user_book));
            struct Book b;
            read(sd,&b,sizeof(struct Book));
            printf("Username: %s\n",ub.username);
            printf("Book id: %d\n",ub.id);
            print_book(&b);
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

int user_portal_client(int sd, struct User *user)
{
    printf("Choose options from below:\n");
    printf("1. Get list of all books in the library\n");
    printf("2. Issue a book under your id\n");
    printf("3. Return a book\n");
    printf("4. View all books issued to you\n");
    int choice;
    scanf("%d",&choice);
    write(sd,&choice,sizeof(int));
    if(choice==1)
    {
        int max_books;
        read(sd,&max_books,sizeof(int));
        for(int i=0;i<max_books;i++)
        {
            struct Book b;
            read(sd,&b,sizeof(struct Book));
            if(b.valid==1) print_book(&b);
        }
        return 1;
    }
    else if(choice==2)
    {
        int id;
        printf("Enter the book id to issue:\n=>");
        struct user_book ub;
        scanf("%d",&id);
        ub.id=id;
        memz(ub.username);
        strcpy(ub.username,user->username);
        write(sd,&ub,sizeof(struct user_book));
        int flag;
        read(sd,&flag,sizeof(int));
        if(flag==OK)
        {
            printf("Book issued successfully\n");
        }
        else if(flag==DUPLICATE)
        {
            printf("Book already issued\n");
        }
        else
        {
            printf("Book not found or copies not available\n");
        }
        return 1;
    }
    else if(choice==3)
    {
        int id;
        printf("Enter the book id to return:\n=>");
        scanf("%d",&id);
        struct user_book ub;
        ub.id=id;
        memz(ub.username);
        strcpy(ub.username,user->username);
        write(sd,&ub,sizeof(struct user_book));
        int flag;
        read(sd,&flag,sizeof(int));
        if(flag==OK)
        {
            printf("Book returned successfully\n");
        }
        else
        {
            printf("Bad Request\n");
        }
        return 1;
    }
    else if(choice==4)
    {
        int max_books;
        read(sd,&max_books,sizeof(int));
        for(int i=0;i<max_books;i++)
        {
            struct user_book ub;
            read(sd,&ub,sizeof(struct user_book));
            struct Book b;
            read(sd,&b,sizeof(struct Book));
            if(strcmp(ub.username,user->username)==0) 
            {
                printf("Book id: %d\n",ub.id);
                print_book(&b);
            }
        }
        return 1;
    }
    else
    {
        return 0;
    }
}

int main()
{
    struct sockaddr_in serv;
    int sd=socket(AF_INET,SOCK_STREAM,0);
    chk(sd,"socket");
    bzero(&serv,sizeof(serv));
    serv.sin_family=AF_INET;
    serv.sin_port=htons(PORT);
    serv.sin_addr.s_addr=inet_addr("127.0.0.1");
    chk(connect(sd,(struct sockaddr*)&serv,sizeof(serv)),"connect");
    struct User login;
    memz(login.username);
    memz(login.password);
    pt("Enter the username:\n=>");
    scanf("%s",login.username);
    pt("Enter the password:\n=>");
    scanf("%s",login.password);
    write(sd,&login,sizeof(struct User));
    int authMode;
    read(sd,&authMode,sizeof(int));
    if(authMode==ADMIN)
    {
        pt("Admin\n");
        int aa=1;
        while(aa)
        {
            aa=admin_portal_client(sd);
        }
    }
    else if(authMode==USER)
    {
        pt("User\n");
        int aa=1;
        while(aa)
        {
            aa=user_portal_client(sd,&login);
        }
    }
    else
    {
        pt("Unauthorised\n");
    }

    // read(sd,buf,BUF);
    // pt(buf);
    close(sd);
}