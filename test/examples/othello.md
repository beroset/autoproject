# [Othello (Reversi) in C++17 and CMake](https://codereview.stackexchange.com/questions/288575)
### tags: ['object-oriented', 'game', 'c++17', 'cmake']

[Othello][1] is a two-player strategy game typically played over the board.

I implemented Othello in C++17 with CMake as the build system.  I'm looking for feedback on cleanness, readability, and extensibility, in addition to the quality of the C++ (are there certain features I could have taken advantage of / are there features that I used inappropriately?).

Here is the project structure:

```
Othello
│
├── cmake-build-debug
│
├── src
│   ├── Controller
│   │   ├── Controller.cpp
│   │   └── Controller.h
│   ├── Model
│   │   ├── Model.cpp
│   │   ├── Model.h
│   │   ├── Player.cpp
│   │   ├── Player.h
│   │   └── Utility.h
│   └── View
│       ├── StandardView.cpp
│       └── StandardView.h
│
├── main.cpp
├── CMakeLists.txt
└── test
```
Here are the source files:
# main.cpp
```cpp
#include <iostream>
#include "Controller/Controller.h"

int main() {
    Othello::Controller othello;
    othello.SetUpGame();
    othello.PlayGame();
    return 0;
}
```
# Controller
## Controller.h
```cpp
#ifndef OTHELLO_CONTROLLER_H
#define OTHELLO_CONTROLLER_H

#include "../View/StandardView.h"
#include "../Model/Model.h"

namespace Othello {
    class Controller {
    public:
        Controller();
        void SetUpGame();
        void PlayGame();
    private:
        Model* model_;
        StandardView* view_;

        void EnactMove(Move move);
    };
}

#endif //OTHELLO_CONTROLLER_H
```
## Controller.cpp
```cpp
#include "Controller.h"


void Othello::Controller::SetUpGame() {
    // Get size of the board and player names
    int board_size = Othello::StandardView::GetBoardSize();
    auto [black_name, white_name] = Othello::StandardView::GetPlayerNames();

    // Create the board and players
    model_->Init(board_size, black_name, white_name);

    // Set up the board with the usual configuration
    int center_row = board_size / 2 - 1;
    int center_col = center_row;

    Coordinate top_left = {center_row, center_col};
    Coordinate top_right = {center_row, center_col + 1};
    Coordinate bottom_left = {center_row + 1, center_col};
    Coordinate bottom_right = {center_row + 1, center_col + 1};

    model_->AddPiece(top_left, WHITE);
    model_->AddPiece(top_right, BLACK);
    model_->AddPiece(bottom_left, BLACK);
    model_->AddPiece(bottom_right, WHITE);
    view_->PrintBoard(model_);
}

void Othello::Controller::PlayGame() {
    // Game ends when neither player can make a move
    while (!model_->IsGameOver()) {
        // Check if active player can make a move
        // If not, skip turn
        if (!model_->TurnValid()) {
            model_->ChangeTurn();
            continue;
        }
        // If active player cannot move, turn is skipped
        auto move = view_->GetMove(model_); // We need to use the model to check if move is valid
        this->EnactMove(move);
        model_->ChangeTurn();
    }
//    view_.DeclareWinner(model_.GetWinner());
}

void Othello::Controller::EnactMove(Move move) {
    auto [coordinate, color] = move;
    model_->AddPiece(coordinate, color);
    auto tiles_flipped = model_->GetTilesFlipped(move);
    for (auto& tile : tiles_flipped) {
        model_->InvertCell(tile);
    }
}

Othello::Controller::Controller() {
    model_ = new Model();
    view_ = new StandardView();
}
```
# Model
## Model.h
```cpp

#ifndef OTHELLO_MODEL_H
#define OTHELLO_MODEL_H


#include "Player.h"

namespace Othello {
    class Model {
    public:
        Model();
        void Init(int board_size, std::string black_name, std::string white_name);

        // Board editing utility
        void AddPiece(Coordinate coordinate, Color color);
        void InvertCell(Coordinate coordinate);


        // Getters & commonly used board state
        Color GetCellColor(Coordinate coordinate) const;
        int GetBoardSize() const;
        bool IsGameOver() const;
        bool CanMove(Player* player) const;
        bool IsValidMove(Move move) const;
        bool TurnValid() const;
        std::vector<Coordinate> GetTilesFlipped(Move move) const;

        // Active Player Methods
        Color GetActiveColor() const;
        std::string GetActiveName() const;
        void ChangeTurn();


    private:
        Player* black_;
        Player* white_;
        std::vector<std::vector<Cell>> board_;
        int board_size_;
        Player* active_player_;
        bool Inbounds(int coordinate) const;
    };
}



#endif //OTHELLO_MODEL_H

```

## Model.cpp
```cpp
#include "Model.h"

#include <utility>



Othello::Model::Model() : board_(0)
{}

void Othello::Model::AddPiece(Coordinate coordinate, Color color) {
    auto [row, column] = coordinate;
    auto& cell = board_.at(row).at(column);
    if (cell.color != EMPTY) {
        throw std::logic_error("You are trying to add a piece to a square that already has one.");
    }
    cell.color = color;
}

void Othello::Model::InvertCell(Coordinate coordinate) {
    auto [row, column] = coordinate;
    auto& cell = board_.at(row).at(column);
    Color curr_color = cell.color;
    if (curr_color == EMPTY) {
        throw std::logic_error("You are trying to invert a cell that is empty.");
    }
    else {
        Color new_color = (curr_color == BLACK) ? WHITE : BLACK;
        cell.color = new_color;
    }
}

void Othello::Model::Init(int board_size, std::string black_name, std::string white_name) {
    black_ = new Player(std::move(black_name), BLACK);
    white_ = new Player(std::move(white_name), WHITE);
    board_ = std::vector<std::vector<Cell>>(board_size, std::vector<Cell>(board_size));
    board_size_ = board_size;
    active_player_ = black_; // BLACK goes first by default
}

Color Othello::Model::GetCellColor(Coordinate coordinate) const {
    auto [row, column] = coordinate;
    auto cell = board_.at(row).at(column);
    return cell.color;
}

int Othello::Model::GetBoardSize() const {
    return board_size_;
}

bool Othello::Model::IsGameOver() const {
    return !CanMove(black_) && !CanMove(white_);
}

bool Othello::Model::CanMove(Othello::Player *player) const {
    int size = (int)board_.size();
    for (int row = 0; row < size; row++) {
        for (int column = 0; column < size; column++) {
            // If the cell is occupied, skip
            if (GetCellColor({row, column}) != EMPTY) continue;
            // From this cell, check in all directions
            Move move = {row, column, player->GetPlayerColor()};
            if (!GetTilesFlipped(move).empty()) {
                return true;
            }
        }
    }
    return false;
}

bool Othello::Model::TurnValid() const{
    return CanMove(active_player_);
}

bool Othello::Model::IsValidMove(Move move) const {
    return !GetTilesFlipped(move).empty();
}

std::vector<Coordinate> Othello::Model::GetTilesFlipped(Move move) const {
    auto [coordinate, same_color] = move;
    auto [start_row, start_column] = coordinate;
    Color alt_color = (same_color == BLACK) ? WHITE : BLACK;
    std::vector<Coordinate> directions = {
            {0, 1}, // Horizontal
            {1, 0}, // Vertical
            {1, 1}, // Right Diagonal
            {1, -1} // Left Diagonal
    };

    // offset for forward and backwards pass
    std::vector<int> offsets = {
            -1,                 // Backward Pass
             1                  // Forward Pass
    };

    std::vector<Coordinate> tiles_flipped;
    for (auto& direction : directions) {
        // for each direction, we perform a forward and backward pass
        for (auto offset : offsets) {
            auto [row_activated, column_activated] = direction;

            // We collect all alternate-color pieces
            std::vector<Coordinate> potential_tiles_flipped;

            // neat trick for combining forward and backward passes into one loop
            int row_offset = offset * row_activated;
            int column_offset = offset * column_activated;
            int row = start_row + row_offset;
            int column = start_column + column_offset;

            // we add the potential tiles flipped once we encounter a tile of the same color
            bool completed_flip = false;

            // flip check loop
            while (Inbounds(row) && Inbounds(column)) {
                auto cell_color = GetCellColor({row, column});
                if (cell_color == alt_color) {
                    potential_tiles_flipped.push_back({row, column});
                } else if (cell_color == same_color) {
                    completed_flip = true;
                    break;
                } else {
                    completed_flip = false;
                    break;
                }

                // update
                offset += offset;
                row_offset = offset * row_activated;
                column_offset = offset * column_activated;
                row = start_row + row_offset;
                column = start_column + column_offset;
            }
            if (completed_flip) {
                // flip all intermediate tiles if flip is completed
                tiles_flipped.insert(tiles_flipped.end(),
                                     potential_tiles_flipped.begin(),
                                     potential_tiles_flipped.end());
            }
        }
    }
    return tiles_flipped;
}

void Othello::Model::ChangeTurn() {
    bool is_black = (active_player_ == black_);
    active_player_ = (is_black) ? white_ : black_;
}

bool Othello::Model::Inbounds(int coordinate) const {
    return (coordinate >= 0) && (coordinate < board_size_);
}

Color Othello::Model::GetActiveColor() const {
    return active_player_->GetPlayerColor();
}

std::string Othello::Model::GetActiveName() const {
    return active_player_->GetName();
}
```
## Player.h
```cpp

#ifndef OTHELLO_PLAYER_H
#define OTHELLO_PLAYER_H
#include <string>
#include "Utility.h"

namespace Othello {
    class Player {
    public:
        Player(std::string name, Color color);
        std::string GetName();
        Color GetPlayerColor();
    private:
        std::string name_;
        Color color_;
    };
}



#endif //OTHELLO_PLAYER_H

```
## Player.cpp
```cpp
#include "Player.h"

#include <utility>

using std::vector;

Othello::Player::Player(std::string name, Color color) : name_(std::move(name)), color_(color) {
}

std::string Othello::Player::GetName() {
    return name_;
}

Color Othello::Player::GetPlayerColor() {
    return color_;
}
```
## Utility.h
```cpp
#ifndef OTHELLO_UTILITY_H
#define OTHELLO_UTILITY_H

#include "Player.h"
enum Color { EMPTY, BLACK,  WHITE };

inline std::string ToString(Color color) {
    switch (color) {
        case EMPTY:
            return "empty";
        case BLACK:
            return "black";
        case WHITE:
            return "white";
    }
}

struct Coordinate {
    int row;
    int column;
};

struct Cell {
    Coordinate coordinate;
    Color color;
};

struct Move {
    Coordinate coordinate;
    Color color;
};


#endif //OTHELLO_UTILITY_H

```

# View
## StandardView.h
```cpp

#ifndef OTHELLO_STANDARDVIEW_H
#define OTHELLO_STANDARDVIEW_H


#include <utility>
#include <string>
#include "../Model/Model.h"

namespace Othello {
    class StandardView {
    public:
        StandardView() = default;

        // Input methods
        static int GetBoardSize();
        static std::pair<std::string, std::string> GetPlayerNames();

        // Output methods
        void PrintBoard(Model *model) ;

        Move GetMove(Othello::Model *model);
    };
}


#endif //OTHELLO_STANDARDVIEW_H

```
## StandardView.cpp
```cpp
#include "StandardView.h"
#include <iostream>

using std::cout, std::cin, std::string, std::pair;

int Othello::StandardView::GetBoardSize() {
    // TODO: Add validation for an even board size
    int board_size;
    std::cout << "Enter an even board size: \n";
    std::cin >> board_size;
    return board_size;
}

pair<std::string, std::string> Othello::StandardView::GetPlayerNames() {
    string black_name, white_name;
    cout << "Enter the name of player with the black pieces: \n";
    cin >> black_name;
    cout << "Enter the name of player with the white pieces: \n";
    cin >> white_name;
    return {black_name, white_name};
}

// Design taken from Code Review Stack Exchange
// https://codereview.stackexchange.com/questions/51716/shortest-possible-way-of-printing-a-specific-board
void Othello::StandardView::PrintBoard(Model *model) {
    int size = model->GetBoardSize();
    string letter_bar, decoration_bar;

    // We want to pad each line with a consistent amount of spaces
    int total_padding = (int)log10(size);
    // Construct letter bar
    for (int _ = 0; _ < total_padding; _++) {
        letter_bar += " ";
    }
    for (char letter = 'a'; (int)letter < 'a' + size; letter++) {
        letter_bar += letter;
        letter_bar += " ";
    }

    // Construct decoration bar
    for (int _ = 0; _ < total_padding; _++) {
        decoration_bar += " ";
    }
    decoration_bar += "+";
    for (int _ = 0; _ < size; _++) {
        decoration_bar += "--";
    }
    decoration_bar += "--+";

    // Print board
    cout << "   " << letter_bar << '\n';
    cout << " " << decoration_bar << '\n';
    for (int row = 0; row < size; row++) {
        int num_digits = (int)log10(row + 1);
        for (int _ = 0; _ < total_padding - num_digits; _++) {
            cout << " ";
        }
        cout << row + 1 << "| ";
        for (int column = 0; column < size; column++) {
            Color color = model->GetCellColor({row, column});
            switch (color) {
                case BLACK:
                    std::cout << 'B';
                    break;
                case WHITE:
                    std::cout << 'W';
                    break;
                case EMPTY:
                    std::cout  << ' ';
                    break;
                default:
                    throw std::logic_error("Cell has not been initialized.");
            }
            cout << " ";
        }
        std::cout << " |\n";
    }
    cout << " " << decoration_bar << '\n';
}

Move Othello::StandardView::GetMove(Othello::Model *model) {
    Move move{0};
    do {
        PrintBoard(model);
        auto color = model->GetActiveColor();
        auto name = model->GetActiveName();
        cout << "It's your move " << name << ".\n";
        cout << "You have the " << ToString(color) << " pieces.\n";
        cout << "Enter move as space separated row number and column character\n";
        cout << "Ex. 1 a\n";
        int raw_row; char raw_col;

        // add some validation here later
        cin >> raw_row >> raw_col;
        int row = raw_row - 1;
        int column = tolower(raw_col)-'a';
        move = {row, column, color};

    } while (!model->IsValidMove(move));
    return move;
}

```
# CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.25)
project(Othello)

set(CMAKE_CXX_STANDARD 17)

add_executable(Othello src/main.cpp src/Controller/Controller.cpp src/Controller/Controller.h src/View/StandardView.cpp src/View/StandardView.h src/Model/Player.cpp src/Model/Player.h src/Model/Model.cpp src/Model/Model.h src/Model/Utility.h)

```





  [1]: https://www.worldothello.org/about/about-othello/othello-rules/official-rules/english