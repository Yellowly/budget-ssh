#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

// ***************************
// Any functions used by both the server and client should be included in this
// directory
// ***************************

/*!
 * Finds the index of the first occurence of the char `find` in the provided
 * c string `str`
 *
 * @returns the index of the first occurence of `char find`, or -1 if it was not
 * found
 */
int index_of_char(const char *str, char find) {
  int i = 0;
  while (str[i] != '\0') {
    if (str[i] == find)
      return i;
    i++;
  }
  return -1;
}

/*!
 * Parses a string in the format "[ip]:[port]" to a `sockaddr_in` struct.
 *
 * @example
 * `struct sockaddr_in addr;`
 * `int success = parse_addr(&addr, "127.0.0.1:8080");`
 * `if (success == -1) exit(1);`
 *
 * @returns 0 if success, or -1 if parse error, -2 if address format error.
 */
int parse_addr(struct sockaddr_in *addr, const char *addr_str) {
  char ip[32];
  int port;
  int result = sscanf(addr_str, "%31[^:]:%d", ip, &port);
  if (result != 2)
    return -1;

  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  if (inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    return -2;

  return 0;
}

/*!
 * Given a `char**` where each `char*` is formatted as `[KEY]=[VALUE]`,
 * find the first index where KEY matches the given key.
 */
int get_env_idx(char **env, char *key) {
  for (int i = 0; env[i] != NULL; i++) {
    char *curr = env[i];
    char is_key = 1;
    int j;
    for (j = 0; key[j] != '\0'; j++) {
      if (curr[j] == '\0' || curr[j] != key[j]) {
        is_key = 0;
        break;
      }
    }
    if (is_key && curr[j] == '=')
      return i;
  }
  return -1;
}
