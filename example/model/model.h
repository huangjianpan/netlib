#pragma once

#include <string>
#include <vector>

#include "json/json_gen.h"

namespace model {

class SudokuReq {
 public:
  // {"sudoku":["53..7....","6..195...",".98....6.","8...6...3","4..8.3..1","7...2...6",".6....28.","...419..5","....8..79"]}
  Attribute("json:sudoku") std::vector<std::string> sudoku;
};

class SudokuRsp {
 public:
  Attribute("json:errcode") int errcode;
  Attribute("json:errmsg") std::string errmsg;
  Attribute("json:sudoku") std::vector<std::string> sudoku;
};

}  // namespace model
