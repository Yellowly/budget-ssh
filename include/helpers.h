#include <netinet/in.h>

/*!
 * Finds the index of the first occurence of the char `find` in the provided
 * c string `str`
 *
 * @returns the index of the first occurence of `char find`, or -1 if it was not
 * found
 */
int index_of_char(const char *str, char find);

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
int parse_addr(struct sockaddr_in *addr, const char *addr_str);

/*!
 * Given a `char**` where each `char*` is formatted as `[KEY]=[VALUE]`,
 * find the first index where KEY matches the given key.
 */
int get_env_idx(char **env, char *key);
