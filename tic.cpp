#include "tic.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {

constexpr std::size_t kRowCount = TicTacToe::BoardSize;
constexpr std::size_t kColCount = TicTacToe::BoardSize;

bool isWinningLine(const std::array<TicTacToe::Player, 9>& board,
                   std::size_t a,
                   std::size_t b,
                   std::size_t c,
                   TicTacToe::Player player) {
    return board[a] == player && board[b] == player && board[c] == player;
}

}  // namespace

TicTacToe::TicTacToe() {
    reset();
}

void TicTacToe::reset() {
    board_.fill(Player::None);
    currentPlayer_ = Player::X;
    winner_ = Player::None;
}

bool TicTacToe::isValidMove(int position) const {
    if (position < 0 || position >= static_cast<int>(board_.size())) {
        return false;
    }

    return board_[static_cast<std::size_t>(position)] == Player::None && winner_ == Player::None;
}

bool TicTacToe::makeMove(int position, Player player) {
    if (player != currentPlayer_ || !isValidMove(position)) {
        return false;
    }

    board_[static_cast<std::size_t>(position)] = player;

    if (isWinner(player)) {
        winner_ = player;
        return true;
    }

    if (isDraw()) {
        winner_ = Player::None;
        return true;
    }

    currentPlayer_ = (player == Player::X) ? Player::O : Player::X;
    return true;
}

bool TicTacToe::isWinner(Player player) const {
    const std::array<std::array<std::size_t, 3>, 8> winPatterns = {{
        {{0, 1, 2}},
        {{3, 4, 5}},
        {{6, 7, 8}},
        {{0, 3, 6}},
        {{1, 4, 7}},
        {{2, 5, 8}},
        {{0, 4, 8}},
        {{2, 4, 6}}
    }};

    for (const auto& pattern : winPatterns) {
        if (isWinningLine(board_, pattern[0], pattern[1], pattern[2], player)) {
            return true;
        }
    }

    return false;
}

bool TicTacToe::isDraw() const {
    if (winner_ != Player::None) {
        return false;
    }

    for (const auto cell : board_) {
        if (cell == Player::None) {
            return false;
        }
    }

    return true;
}

bool TicTacToe::isGameOver() const {
    return winner_ != Player::None || isDraw();
}

std::vector<int> TicTacToe::availableMoves() const {
    std::vector<int> moves;
    for (std::size_t i = 0; i < board_.size(); ++i) {
        if (board_[i] == Player::None) {
            moves.push_back(static_cast<int>(i));
        }
    }
    return moves;
}

std::string TicTacToe::toString() const {
    std::ostringstream out;
    for (std::size_t row = 0; row < kRowCount; ++row) {
        for (std::size_t col = 0; col < kColCount; ++col) {
            const auto index = indexFromRowCol(static_cast<int>(row), static_cast<int>(col));
            const auto cell = board_[static_cast<std::size_t>(index)];
            out << " " << ((cell == Player::None) ? std::to_string(index + 1) : playerToString(cell)) << " ";
            if (col < kColCount - 1) {
                out << "|";
            }
        }
        if (row < kRowCount - 1) {
            out << "\n-----------\n";
        }
    }
    return out.str();
}

std::string TicTacToe::playerToString(Player player) const {
    switch (player) {
        case Player::X: return "X";
        case Player::O: return "O";
        case Player::None: return " ";
        default: return "?";
    }
}

TicTacToe::Player TicTacToe::winner() const {
    return winner_;
}

TicTacToe::Player TicTacToe::currentPlayer() const {
    return currentPlayer_;
}

TicTacToe::Player TicTacToe::boardAt(std::size_t index) const {
    if (index >= board_.size()) {
        throw std::out_of_range("board index out of range");
    }
    return board_[index];
}

void TicTacToe::setCurrentPlayer(Player player) {
    if (player == Player::X || player == Player::O) {
        currentPlayer_ = player;
    }
}

int TicTacToe::indexFromRowCol(int row, int col) {
    return row * BoardSize + col;
}

int TicTacToe::positionFromRowCol(int row, int col) {
    return indexFromRowCol(row, col);
}

bool TicTacToe::isInsideBoard(int row, int col) {
    return row >= 0 && row < static_cast<int>(BoardSize) &&
           col >= 0 && col < static_cast<int>(BoardSize);
}

