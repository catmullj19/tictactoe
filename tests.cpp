#include "ai.h"
#include "tic.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <string>
#include <vector>

int main() {
    {
        TicTacToe game;
        assert(game.availableMoves().size() == 9);
        assert(game.currentPlayer() == TicTacToe::Player::X);
    }

    {
        TicTacToe game;
        assert(game.makeMove(0, TicTacToe::Player::X));
        assert(game.boardAt(0) == TicTacToe::Player::X);
        assert(game.currentPlayer() == TicTacToe::Player::O);
        assert(game.isValidMove(0) == false);
    }

    {
        TicTacToe game;
        for (int pos : {0, 1, 3, 4, 6}) {
            game.makeMove(pos, TicTacToe::Player::X);
            if (pos != 6) {
                game.makeMove(pos + 1, TicTacToe::Player::O);
            }
        }
        assert(game.isWinner(TicTacToe::Player::X));
        assert(game.isGameOver());
    }

    {
        TicTacToe game;
        game.makeMove(0, TicTacToe::Player::X);
        game.makeMove(3, TicTacToe::Player::O);
        game.makeMove(1, TicTacToe::Player::X);
        game.makeMove(4, TicTacToe::Player::O);
        game.makeMove(2, TicTacToe::Player::X);
        assert(game.isWinner(TicTacToe::Player::X));
    }

    {
        TicTacToe game;
        std::vector<int> moves = {0, 1, 2, 4, 3, 5, 7, 6, 8};
        for (int move : moves) {
            game.makeMove(move, game.currentPlayer());
        }
        assert(game.isDraw());
    }

    {
        // An untrained agent learns from one game where X wins along the top row.
        QLearningAgent learner(1);
        learner.learnFromGame({0, 3, 1, 4, 2});

        // Playing X, it repeats the winning move.
        TicTacToe game;
        for (int move : {0, 3, 1, 4}) {
            game.makeMove(move, game.currentPlayer());
        }
        assert(learner.chooseMove(game) == 2);

        // Playing O, it avoids the move that lost.
        TicTacToe lost;
        for (int move : {0, 3, 1}) {
            lost.makeMove(move, lost.currentPlayer());
        }
        for (int i = 0; i < 100; ++i) {
            assert(learner.chooseMove(lost) != 4);
        }
    }

    {
        // Games with invalid moves are ignored rather than half-learned.
        QLearningAgent learner(1);
        learner.learnFromGame({0, 0, 1});
        TicTacToe game;
        game.makeMove(0, TicTacToe::Player::X);
        game.makeMove(3, TicTacToe::Player::O);
        game.makeMove(1, TicTacToe::Player::X);
        game.makeMove(4, TicTacToe::Player::O);
        bool sawOtherMove = false;
        for (int i = 0; i < 100; ++i) {
            sawOtherMove = sawOtherMove || learner.chooseMove(game) != 2;
        }
        assert(sawOtherMove);
    }

    {
        // Saving and loading preserves every score exactly: re-saving gives an identical file.
        const auto dir = std::filesystem::temp_directory_path();
        const std::string first = (dir / "tictactoe_test_scores_1.txt").string();
        const std::string second = (dir / "tictactoe_test_scores_2.txt").string();

        QLearningAgent original(3);
        original.train(1000);
        original.learnFromGame({0, 3, 1, 4, 2});
        assert(original.save(first));

        QLearningAgent loaded(4);
        assert(loaded.load(first));
        assert(loaded.save(second));

        std::ifstream a(first);
        std::ifstream b(second);
        const std::string aText((std::istreambuf_iterator<char>(a)), std::istreambuf_iterator<char>());
        const std::string bText((std::istreambuf_iterator<char>(b)), std::istreambuf_iterator<char>());
        assert(!aText.empty() && aText == bText);
        a.close();
        b.close();

        // Missing and invalid files are rejected and leave the agent unchanged.
        const std::string bad = (dir / "tictactoe_test_scores_bad.txt").string();
        std::ofstream(bad) << "tictactoe-scores 1\n0 99 1.0\n";
        assert(!loaded.load(bad));
        std::ofstream(bad) << "not a scores file\n";
        assert(!loaded.load(bad));
        assert(!loaded.load((dir / "tictactoe_no_such_file.txt").string()));
        assert(loaded.save(second));
        std::ifstream c(second);
        const std::string cText((std::istreambuf_iterator<char>(c)), std::istreambuf_iterator<char>());
        assert(cText == aText);
        c.close();

        std::filesystem::remove(first);
        std::filesystem::remove(second);
        std::filesystem::remove(bad);
    }

    QLearningAgent agent(42);
    agent.train(100000);

    {
        // Takes a winning move: X has 0 and 1, so X should play 2.
        TicTacToe game;
        for (int move : {0, 3, 1, 4}) {
            game.makeMove(move, game.currentPlayer());
        }
        assert(agent.chooseMove(game) == 2);
    }

    {
        // Blocks a losing move: X threatens 0-1-2, so O should play 2.
        TicTacToe game;
        for (int move : {0, 4, 1}) {
            game.makeMove(move, game.currentPlayer());
        }
        assert(agent.chooseMove(game) == 2);
    }

    {
        // Never loses to a random opponent, playing either side.
        std::mt19937 rng(7);
        for (auto agentPlayer : {TicTacToe::Player::X, TicTacToe::Player::O}) {
            for (int i = 0; i < 1000; ++i) {
                TicTacToe game;
                while (!game.isGameOver()) {
                    int move = 0;
                    if (game.currentPlayer() == agentPlayer) {
                        move = agent.chooseMove(game);
                    } else {
                        const auto moves = game.availableMoves();
                        move = moves[std::uniform_int_distribution<std::size_t>(0, moves.size() - 1)(rng)];
                    }
                    game.makeMove(move, game.currentPlayer());
                }
                assert(game.winner() == agentPlayer || game.winner() == TicTacToe::Player::None);
            }
        }
    }

    {
        // Two trained agents always draw.
        TicTacToe game;
        while (!game.isGameOver()) {
            game.makeMove(agent.chooseMove(game), game.currentPlayer());
        }
        assert(game.isDraw());
    }

    return 0;
}
