#include "sudoku.h"

void Sudoku::handler(context::Context& ctx, model::SudokuReq& req,
                     model::SudokuRsp& rsp) {
  if (!check_param(req.sudoku)) {
    rsp.errcode = 1;
    rsp.errmsg = "check param failed";
    return;
  }
  solve(req.sudoku);
  rsp.errcode = 0;
  rsp.errmsg = "success";
  rsp.sudoku = std::move(req.sudoku);
  return;
}

bool Sudoku::check_param(const std::vector<std::string>& sudoku) {
  if (sudoku.size() != 9) {
    return false;
  }
  for (const auto& row : sudoku) {
    if (row.size() != 9) {
      return false;
    }
  }
  return true;
}

void Sudoku::solve(std::vector<std::string>& sudoku) {
  size_t state[9] = {};
  std::vector<size_t> blank;
  for (size_t i = 0; i < 9; ++i) {
    for (size_t j = 0; j < 9; ++j) {
      if (sudoku[i][j] == '.') {
        blank.emplace_back((i << 8) + j);
      } else {
        state[sudoku[i][j] - '1'] |=
            (1 << (i / 3 * 3 + j / 3)) | (1 << (i + 9)) | 1 << (j + 18);
      }
    }
  }
  auto f = [&](auto&& self, size_t id) -> bool {
    if (id == blank.size()) {
      return true;
    }
    size_t i = blank[id] >> 8;
    size_t j = blank[id] & 0xff;
    size_t new_state =
        (1 << (i / 3 * 3 + j / 3)) | (1 << (i + 9)) | 1 << (j + 18);
    for (char n = '1'; n <= '9'; ++n) {
      if (state[n - '1'] & new_state) continue;
      state[n - '1'] |= new_state;
      if (self(self, id + 1)) {
        sudoku[i][j] = n;
        return true;
      }
      state[n - '1'] &= ~new_state;
    }
    return false;
  };
  f(f, 0);
}