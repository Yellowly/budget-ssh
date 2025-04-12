#include <netinet/in.h>

/*!
 * Connect to the given address and run the client
 *
 * This should be called once in main.c
 *
 * @returns -1 if an error occurs while the client is running
 */
int run_client(struct sockaddr_in addr);
