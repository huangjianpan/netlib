#include "handler/handler.h"
#include "http/http.h"

void register_route(http::HttpServer& svr);

int main(int argc, char** argv) {
  http::HttpServer svr(8888);
  register_route(svr);
  svr.start();
  return 0;
}

void register_route(http::HttpServer& svr) { 
  svr.POST("/sudoku", Sudoku::handler);
}