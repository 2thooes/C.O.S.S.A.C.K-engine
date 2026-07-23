# Custom Chess Engine

A lightweight, high-performance chess engine written in C using bitboards for efficient board representation and move generation. The engine is fully compatible with the **UCI (Universal Chess Interface)** protocol, allowing it to easily connect to popular graphical user interfaces (GUIs) like Cutechess, Arena, or tournament managers.

After testing with stockfish, the estimated power level is: 1450 elo.

## Architectural Features

*   **Bitboards:** Complete 64-bit (`uint64_t`) representation of the chess board for lightning-fast move generation and slider piece evaluation.
*   **Move Validation:** Correct handling of complex chess rules, including dynamic updates of castling rights (via context-aware bitwise clearing when kings/rooks move or are captured) and en passant captures.
*   **Deterministic Evaluation:** Single-threaded execution ensures 100% predictable, easily debugged, and reproducible search trees.

## Installing

Run this command to install engine:

git clone https://github.com/2thooes/C.O.S.S.A.C.K-engine

## Building and Compiling

The project is optimized for compilation on **Fedora Linux**. 

To compile the engine with maximum compiler optimizations (`-O3`), run the following command in your terminal:

gcc src/main.c src/board.c src/legalMoves.c src/evaluate.c src/findBest.c -o main

## Testing and Playing

You can install Cutechess or any other graphical interface to play against bot:

git clone https://github.com/cutechess/cutechess