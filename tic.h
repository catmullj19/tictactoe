#ifndef TIC_TAC_TOE_H
#define TIC_TAC_TOE_H

#include <array>
#include <cstddef>
#include <string>
#include <vector>

class TicTacToe {
public:
    enum class Player : int {
        None = 0,
        X = 1,
        O = 2
    };

    static constexpr std::size_t BoardSize = 3;
    static constexpr int InvalidMove = -1;

    TicTacToe();

    bool isValidMove(int position) const;
    bool makeMove(int position, Player player);
    bool isWinner(Player player) const;
    bool isDraw() const;
    bool isGameOver() const;

    void reset();
    std::vector<int> availableMoves() const;
    std::string toString() const;
    std::string playerToString(Player player) const;

    Player winner() const;
    Player currentPlayer() const;
    Player boardAt(std::size_t index) const;
    void setCurrentPlayer(Player player);

private:
    static constexpr int kCellCount = BoardSize * BoardSize;

    std::array<Player, kCellCount> board_{};
    Player currentPlayer_ = Player::X;
    Player winner_ = Player::None;

    static int indexFromRowCol(int row, int col);
    static int positionFromRowCol(int row, int col);
    static bool isInsideBoard(int row, int col);
};

#endif
