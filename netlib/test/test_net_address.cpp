#include <cstdio>

#include "net/address.h"

int main(int argc, char** argv) {
  net::Address addr("127.0.0.1", 8888);
  printf("%s:%d\n", addr.ip(), addr.port());
  return 0;
}