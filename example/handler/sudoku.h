#pragma once

#include "http/http.h"
#include "model/model.h"

class Sudoku {
 public:
  static void handler(context::Context& ctx, model::SudokuReq& req, model::SudokuRsp& rsp);

  static bool check_param(const std::vector<std::string>& sudoku);

  static void solve(std::vector<std::string>& sodoku);
};