#include <netinet/in.h>

/*!
 * Runs a server on the given address
 *
 * This should be called once in main.c
 *
 * @returns -1 if an error occurs while the server is running
 */
int run_server(struct sockaddr_in addr);
