#include <cstdio>

#include "http/http.h"

int main(int argc, char** argv) {
  http::HttpServer svr(8888);
  svr.start();
  return 0;
}