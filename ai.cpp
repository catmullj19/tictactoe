#include "ai.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>

namespace {

constexpr const char* kFileHeader = "tictactoe-scores 1";

}  // namespace

QLearningAgent::QLearningAgent(unsigned seed)
    : q_(kStateCount), rng_(seed) {
    for (auto& values : q_) {
        values.fill(0.0);
    }
}

void QLearningAgent::train(int episodes) {
    for (int episode = 0; episode < episodes; ++episode) {
        selfPlay(TicTacToe());
    }
}

void QLearningAgent::selfPlay(TicTacToe game) {
    std::uniform_real_distribution<double> chance(0.0, 1.0);

    while (!game.isGameOver()) {
        int move = 0;
        if (chance(rng_) < explorationRate_) {
            const auto moves = game.availableMoves();
            std::uniform_int_distribution<std::size_t> pick(0, moves.size() - 1);
            move = moves[pick(rng_)];
        } else {
            move = bestMove(game);
        }

        update(game, move, learningRate_);
        game.makeMove(move, game.currentPlayer());
    }
}

void QLearningAgent::learnFromGame(const std::vector<int>& moves) {
    // Learn from all 8 rotations and reflections of the game, since they are equally valid.
    for (int symmetry = 0; symmetry < 8; ++symmetry) {
        std::vector<TicTacToe> positions;
        std::vector<int> transformed;
        TicTacToe game;
        for (const int move : moves) {
            positions.push_back(game);
            transformed.push_back(transform(move, symmetry));
            if (!game.makeMove(transformed.back(), game.currentPlayer())) {
                return;
            }
        }

        // Study the game: play practice games from each position in it, so the agent
        // explores the alternatives it didn't try, such as the move that would have saved it.
        for (const auto& position : positions) {
            for (int i = 0; i < studyGamesPerPosition_; ++i) {
                selfPlay(position);
            }
        }

        // Then update from the final move backwards so the result reaches the opening moves in one pass.
        // A real game is worth remembering in full, so use a learning rate of 1.
        for (std::size_t i = transformed.size(); i-- > 0;) {
            update(positions[i], transformed[i], 1.0);
        }
    }
}

int QLearningAgent::transform(int cell, int symmetry) {
    int row = cell / 3;
    int col = cell % 3;
    if (symmetry & 4) {
        std::swap(row, col);
    }
    if (symmetry & 1) {
        row = 2 - row;
    }
    if (symmetry & 2) {
        col = 2 - col;
    }
    return row * 3 + col;
}

void QLearningAgent::update(const TicTacToe& before, int move, double learningRate) {
    const auto player = before.currentPlayer();
    TicTacToe after = before;
    after.makeMove(move, player);

    // The value of a move is the result if the game ended, otherwise the
    // negation of the opponent's best reply (what is good for them is bad for us).
    double target = 0.0;
    if (after.isWinner(player)) {
        target = 1.0;
    } else if (!after.isDraw()) {
        target = -bestValue(after);
    }

    double& value = q_[static_cast<std::size_t>(encode(before))][static_cast<std::size_t>(move)];
    value += learningRate * (target - value);
}

int QLearningAgent::chooseMove(const TicTacToe& game) {
    if (game.isGameOver()) {
        return TicTacToe::InvalidMove;
    }
    return bestMove(game);
}

bool QLearningAgent::save(const std::string& path) const {
    // Write to a temporary file and rename it, so a crash mid-save can't leave a half-written file.
    const std::string tempPath = path + ".tmp";
    {
        std::ofstream out(tempPath);
        if (!out) {
            return false;
        }

        // One "state move value" line per learned score; unlearned scores are zero and skipped.
        out << kFileHeader << "\n" << std::setprecision(17);
        for (std::size_t state = 0; state < q_.size(); ++state) {
            for (std::size_t move = 0; move < kCellCount; ++move) {
                if (q_[state][move] != 0.0) {
                    out << state << ' ' << move << ' ' << q_[state][move] << '\n';
                }
            }
        }

        if (!out) {
            return false;
        }
    }

    std::error_code error;
    std::filesystem::rename(tempPath, path, error);
    return !error;
}

bool QLearningAgent::load(const std::string& path) {
    std::ifstream in(path);
    std::string header;
    if (!in || !std::getline(in, header) || header != kFileHeader) {
        return false;
    }

    std::vector<std::array<double, kCellCount>> loaded(kStateCount);
    for (auto& values : loaded) {
        values.fill(0.0);
    }

    std::size_t state = 0;
    std::size_t move = 0;
    double value = 0.0;
    while (in >> state >> move >> value) {
        if (state >= loaded.size() || move >= kCellCount) {
            return false;
        }
        loaded[state][move] = value;
    }

    // Stopping anywhere other than the end of the file means a malformed line.
    if (!in.eof()) {
        return false;
    }

    q_ = std::move(loaded);
    return true;
}

int QLearningAgent::encode(const TicTacToe& game) {
    int code = 0;
    for (std::size_t i = 0; i < kCellCount; ++i) {
        code = code * 3 + static_cast<int>(game.boardAt(i));
    }
    return code * 2 + (game.currentPlayer() == TicTacToe::Player::O ? 1 : 0);
}

double QLearningAgent::bestValue(const TicTacToe& game) const {
    const auto& values = q_[static_cast<std::size_t>(encode(game))];
    double best = -std::numeric_limits<double>::infinity();
    for (const int move : game.availableMoves()) {
        best = std::max(best, values[static_cast<std::size_t>(move)]);
    }
    return best;
}

int QLearningAgent::bestMove(const TicTacToe& game) {
    const auto& values = q_[static_cast<std::size_t>(encode(game))];
    const double best = bestValue(game);

    // Break ties randomly so the agent doesn't always open the same way.
    std::vector<int> bestMoves;
    for (const int move : game.availableMoves()) {
        if (values[static_cast<std::size_t>(move)] >= best - 1e-9) {
            bestMoves.push_back(move);
        }
    }

    std::uniform_int_distribution<std::size_t> pick(0, bestMoves.size() - 1);
    return bestMoves[pick(rng_)];
}
