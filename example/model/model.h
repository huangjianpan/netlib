#pragma once

#include <string>
#include <vector>

#include "json/json_gen.h"

namespace model {

class SudokuReq {
 public:
  // {"sudokus":[["53..7....","6..195...",".98....6.","8...6...3","4..8.3..1","7...2...6",".6....28.","...419..5","....8..79"],["..9748...","7........",".2.1.9...","..7...24.",".64.1.59.",".98...3..","...8.3.2.","........6","...2759.."]]}
  Attribute("json:sudokus") std::vector<std::vector<std::string>> sudokus;
};

class SudokuRsp {
 public:
  class Entry {
   public:
    Attribute("json:errcode") int errcode;
    Attribute("json:errmsg") std::string errmsg;
    Attribute("json:sudoku") std::vector<std::string> sudoku;
  };

  Attribute("json:sudokus") std::vector<Entry> sudokus;
};

}  // namespace model
