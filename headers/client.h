#ifndef CLIENT_H
#define CLIENT_H

int admin_portal_client(int sd);
int user_portal_client(int sd, struct User *user);


#endif // !CLIENT_H