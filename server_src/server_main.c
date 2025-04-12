#include <helpers.h>
#include <ssh_server.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc > 2) {
    printf("Usage: [Optional: IP Address:Port]\n");
    exit(-1);
  }

  struct sockaddr_in addr;

  if (argc == 1) {
    if (parse_addr(&addr, "127.0.0.1:8080") < 0) {
      printf("Could not parse address\n");
      return -1;
    }
  } else if (argc == 2) {
    if (parse_addr(&addr, argv[1]) < 0) {
      printf("Could not parse address\n");
      return -1;
    }
  }

  if (run_server(addr) < 0) {
    printf("An error occurred while running the server");
    return -1;
  }

  return 0;
}
