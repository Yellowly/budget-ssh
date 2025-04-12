#include <helpers.h>
#include <ssh_client.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc > 2) {
    printf("Usage: [Optional: IP Address:Port]\n");
    exit(-1);
  }

  // see main.c in server_src for implementation strategy
  // use run_client() (from ssh_client.c) instead of run_server()

  return 0;
}
