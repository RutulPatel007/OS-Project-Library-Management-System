#ifndef SERVER_H
#define SERVER_H

#include"utils.h"

int authentication(int nsd);
void *connection(void *args);

int admin_portal(int nsd);
void add_new_user(int nsd);
void add_book(int nsd);
void get_all_books(int fd);
void delete_book(int nsd);
void add_more_copies(int nsd);

int client_portal(int nsd);
void issue_book(int nsd);
void get_all_issue_entry();
void get_all_issue_entry_nsd(int nsd);
void return_book(int nsd);

#define MAX_BOOKS 10000 
#define max(a,b) a>b?a:b
#endif // !SERVER_H