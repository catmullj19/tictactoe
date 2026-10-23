#include "ai.h"
#include "tic.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <vector>

// Usage: tic [training-games]
// With no argument, the computer carries on with the scores saved next to the
// executable, or trains with 100000 self-play games if there are none.
// With training-games, it starts fresh with that many self-play games, replacing
// any saved scores. Use 0 to start untrained and learn only from you.
int main(int argc, char* argv[]) {
    const bool startFresh = argc > 1;
    const int trainingGames = startFresh ? std::atoi(argv[1]) : 100000;
    const std::string scoresPath =
        (std::filesystem::path(argv[0]).parent_path() / "tictactoe_scores.txt").string();

    std::cout << "Modern Tic-Tac-Toe\n";
    std::cout << "Play against the computer? (y/n): ";

    char answer = 'n';
    std::cin >> answer;
    const bool vsComputer = (answer == 'y' || answer == 'Y');

    QLearningAgent agent;
    if (vsComputer) {
        if (!startFresh && agent.load(scoresPath)) {
            std::cout << "Loaded the computer's saved scores from " << scoresPath << "\n";
        } else {
            std::cout << "Training the computer with " << trainingGames << " self-play games...\n";
            agent.train(trainingGames);
            if (!agent.save(scoresPath)) {
                std::cout << "Warning: couldn't save scores to " << scoresPath << "\n";
            }
        }
        std::cout << "You are X, the computer is O.\n";
    }

    bool playAgain = true;
    while (playAgain) {
        TicTacToe game;
        std::vector<int> moves;

        std::cout << "Choose a cell from 1 to 9.\n";
        std::cout << game.toString() << "\n";

        while (!game.isGameOver()) {
            const auto current = game.currentPlayer();

            if (vsComputer && current == TicTacToe::Player::O) {
                const int move = agent.chooseMove(game);
                game.makeMove(move, current);
                moves.push_back(move);
                std::cout << "Computer plays " << move + 1 << "\n";
                std::cout << game.toString() << "\n";
                continue;
            }

            std::cout << "Player " << game.playerToString(current) << " move: ";

            int move = 0;
            std::cin >> move;

            if (move < 1 || move > 9) {
                std::cout << "Invalid move. Please enter a number from 1 to 9.\n";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }

            const bool ok = game.makeMove(move - 1, current);
            if (!ok) {
                std::cout << "That square is already taken. Try again.\n";
                continue;
            }
            moves.push_back(move - 1);

            std::cout << game.toString() << "\n";
        }

        if (game.winner() == TicTacToe::Player::None) {
            std::cout << "Draw!\n";
        } else {
            std::cout << "Player " << game.playerToString(game.winner()) << " wins!\n";
        }

        if (vsComputer) {
            agent.learnFromGame(moves);
            if (agent.save(scoresPath)) {
                std::cout << "The computer has learned from this game.\n";
            } else {
                std::cout << "The computer has learned from this game, but couldn't save to "
                          << scoresPath << "\n";
            }
        }

        std::cout << "Play again? (y/n): ";
        answer = 'n';
        std::cin >> answer;
        playAgain = (answer == 'y' || answer == 'Y');
    }

    return 0;
}
