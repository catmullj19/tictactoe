#include "ai.h"
#include "tic.h"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QGridLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include <array>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

class TicTacToeWindow : public QWidget {
public:
    TicTacToeWindow(bool startFresh, int trainingGames, std::string scoresPath, QWidget* parent = nullptr)
        : QWidget(parent), game_(), scoresPath_(std::move(scoresPath)) {
        if (startFresh || !agent_.load(scoresPath_)) {
            agent_.train(trainingGames);
            saveScores();
        }

        auto* mainLayout = new QVBoxLayout(this);
        auto* boardLayout = new QGridLayout();

        for (int i = 0; i < 9; ++i) {
            buttons_[i] = new QPushButton(this);
            buttons_[i]->setFixedSize(90, 90);
            buttons_[i]->setFont(QFont("Sans Serif", 24, QFont::Bold));
            boardLayout->addWidget(buttons_[i], i / 3, i % 3);

            connect(buttons_[i], &QPushButton::clicked, this, [this, i]() {
                handleMove(i);
            });
        }

        auto* resetButton = new QPushButton("New Game", this);
        connect(resetButton, &QPushButton::clicked, this, &TicTacToeWindow::resetGame);

        vsComputer_ = new QCheckBox("Play against computer (O)", this);
        vsComputer_->setChecked(true);
        connect(vsComputer_, &QCheckBox::toggled, this, [this]() {
            playComputerMove();
        });

        mainLayout->addLayout(boardLayout);
        mainLayout->addWidget(vsComputer_);
        mainLayout->addWidget(resetButton);

        updateBoard();
    }

private:
    void handleMove(int index) {
        if (!game_.isValidMove(index)) {
            return;
        }

        if (!makeMove(index)) {
            return;
        }

        playComputerMove();
    }

    void playComputerMove() {
        if (!vsComputer_->isChecked() || game_.isGameOver() ||
            game_.currentPlayer() != TicTacToe::Player::O) {
            return;
        }

        makeMove(agent_.chooseMove(game_));
    }

    bool makeMove(int index) {
        if (!game_.makeMove(index, game_.currentPlayer())) {
            return false;
        }

        moves_.push_back(index);
        updateBoard();
        showResultIfOver();
        return true;
    }

    void showResultIfOver() {
        if (game_.isGameOver()) {
            agent_.learnFromGame(moves_);
            ++gamesLearned_;

            const auto winner = game_.winner();
            QString message = (winner == TicTacToe::Player::None)
                ? "Draw!"
                : QString("Player %1 wins!").arg(game_.playerToString(winner).c_str());
            message += QString("\n\nThe computer has learned from %1 game(s) this session.").arg(gamesLearned_);
            if (!saveScores()) {
                message += "\nWarning: its scores couldn't be saved.";
            }

            QMessageBox::information(this, "Game Over", message);
        }
    }

    bool saveScores() {
        if (agent_.save(scoresPath_)) {
            return true;
        }
        qWarning("Couldn't save scores to %s", scoresPath_.c_str());
        return false;
    }

    void resetGame() {
        game_.reset();
        moves_.clear();
        updateBoard();
    }

    void updateBoard() {
        for (int i = 0; i < 9; ++i) {
            const auto cell = game_.boardAt(static_cast<std::size_t>(i));
            QPushButton* button = buttons_[i];

            if (cell == TicTacToe::Player::None) {
                button->setText("");
            } else if (cell == TicTacToe::Player::X) {
                button->setText("X");
            } else {
                button->setText("O");
            }

            button->setEnabled(cell == TicTacToe::Player::None && !game_.isGameOver());
        }
    }

    TicTacToe game_;
    std::string scoresPath_;
    QLearningAgent agent_;
    std::vector<int> moves_;
    int gamesLearned_ = 0;
    std::array<QPushButton*, 9> buttons_{};
    QCheckBox* vsComputer_ = nullptr;
};

// Usage: tic_gui [training-games]
// With no argument, the computer carries on with the scores saved next to the
// executable, or trains with 100000 self-play games if there are none.
// With training-games, it starts fresh with that many self-play games, replacing
// any saved scores. Use 0 to start untrained and learn only from you.
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    const bool startFresh = argc > 1;
    const int trainingGames = startFresh ? std::atoi(argv[1]) : 100000;
    const std::string scoresPath =
        QDir(QCoreApplication::applicationDirPath()).filePath("tictactoe_scores.txt").toStdString();
    TicTacToeWindow window(startFresh, trainingGames, scoresPath);
    window.resize(360, 420);
    window.setWindowTitle("Tic-Tac-Toe");
    window.show();
    return app.exec();
}
