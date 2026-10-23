#ifndef TIC_TAC_TOE_AI_H
#define TIC_TAC_TOE_AI_H

#include "tic.h"

#include <array>
#include <random>
#include <string>
#include <vector>

// Tabular Q-learning agent that learns by playing games against itself.
//
// Every (board, player to move) state has a value for each of the nine cells:
// the expected outcome of playing there, from +1 (win) through 0 (draw) to -1 (loss).
class QLearningAgent {
public:
    explicit QLearningAgent(unsigned seed = std::random_device{}());

    // Plays `episodes` self-play games, updating the Q-table after every move.
    void train(int episodes);

    // Learns from a finished game, given every move in order starting with X.
    // Games containing an invalid move are ignored.
    void learnFromGame(const std::vector<int>& moves);

    // Returns the best learned move for the current player, or InvalidMove if the game is over.
    int chooseMove(const TicTacToe& game);

    // Saves the learned scores to a text file. Returns false if the file can't be written.
    bool save(const std::string& path) const;

    // Loads scores written by save(). Returns false, leaving the agent unchanged,
    // if the file is missing or invalid.
    bool load(const std::string& path);

private:
    static constexpr int kCellCount = TicTacToe::BoardSize * TicTacToe::BoardSize;
    static constexpr int kStateCount = 19683 * 2;  // 3^9 boards x 2 players to move

    static int encode(const TicTacToe& game);
    static int transform(int cell, int symmetry);
    void selfPlay(TicTacToe game);
    void update(const TicTacToe& before, int move, double learningRate);
    double bestValue(const TicTacToe& game) const;
    int bestMove(const TicTacToe& game);

    std::vector<std::array<double, kCellCount>> q_;
    std::mt19937 rng_;
    double learningRate_ = 0.5;
    double explorationRate_ = 0.3;
    int studyGamesPerPosition_ = 20;
};

#endif
