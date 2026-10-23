# tictactoe

Simple RL noughts and crosses: play tic-tac-toe against a computer opponent that
learns by reinforcement learning, first by playing against itself and then from
every game it plays against you.

There is a command-line version (`tic`) and an optional Qt desktop version (`tic_gui`).

## Requirements

- A C++17 compiler (GCC, Clang or MSVC)
- [CMake](https://cmake.org/) 3.16 or newer, and a build tool such as [Ninja](https://ninja-build.org/)
- Optional, for the GUI: [Qt 6](https://www.qt.io/download-qt-installer) with the Widgets
  module. Qt 6 needs CMake 3.25 or newer.

If Qt isn't found, CMake skips the GUI and builds the command-line version only.

## Building

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

If Qt is installed somewhere CMake doesn't search, tell it where with `CMAKE_PREFIX_PATH`,
pointing at the folder for your compiler inside the Qt installation:

```sh
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/mingw_64
```

Use a compiler that matches your Qt build. For example, the Qt installer's `mingw_64` build
should be compiled with the MinGW that comes with it (in `C:/Qt/Tools/`), and needs
`-DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe`.

For faster startup, build with optimisation: add `-DCMAKE_BUILD_TYPE=Release` when configuring.

## Running

### Command line

```sh
build/tic
```

Answer `y` to play against the computer: you are X and move first, the computer is O.
Answer `n` for a two-player game. Choose cells by number:

```
 1 | 2 | 3
-----------
 4 | 5 | 6
-----------
 7 | 8 | 9
```

### Desktop (Qt)

```sh
build/tic_gui
```

Click a square to move. Untick **Play against computer (O)** for a two-player game, and
click **New Game** to start again.

On Windows, `tic_gui` needs the Qt DLLs to start. Either add Qt's `bin` folder
(for example `C:\Qt\6.12.0\mingw_64\bin`) to your `PATH`, or copy the DLLs next to the
executable once with:

```sh
C:\Qt\6.12.0\mingw_64\bin\windeployqt.exe build\tic_gui.exe
```

### Training and saved scores

The computer saves what it has learned to `tictactoe_scores.txt`, next to the executables,
after every game. `tic` and `tic_gui` share the same file.

| Command | What the computer does |
|---|---|
| `tic` | Carries on from its saved scores. If there are none, it first trains by playing 100,000 games against itself, which makes it unbeatable. |
| `tic 0` | Starts again with no training, replacing the saved scores, and learns only from playing you. |
| `tic 5000` | Starts again with 5,000 self-play games, replacing the saved scores. It plays reasonably but has weaknesses you can find. |

`tic_gui` takes the same argument. To watch the computer learn, start it with `tic 0` and
play some games, then keep running `tic` without an argument: it should stop losing to you
within about 15 to 25 games. Deleting `tictactoe_scores.txt` also resets it.

## Running the tests

```sh
ctest --test-dir build
```

`--test-dir` needs CMake 3.20 or newer. With an older CMake, run `ctest` from inside the
`build` folder instead.

## How the computer learns

The computer uses tabular Q-learning (see `ai.h` and `ai.cpp`). It keeps a score for every
move in every board position, from +1 (this move tends to win) through 0 (draw) to -1
(loss).

- **Self-play:** it plays games against itself, nudging each move's score towards the
  result. 30% of its moves are random so it explores positions it would otherwise never reach.
- **Learning from your games:** after each game, it plays 20 short practice games from every
  position in that game, including all 8 rotations and reflections, then records the actual
  result. This focuses its learning on the positions where you found a weakness.
- **Playing:** it picks the move with the highest score, breaking ties randomly.
