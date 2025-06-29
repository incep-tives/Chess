# Chess

An open-source chess application written in C++. This project features a classic chess game with a built-in Stockfish engine, supporting various modes and adjustable difficulty. It is designed for enthusiasts who want to play against a strong chess engine or experiment with different chess variants.

## Features

- **Classic Chess**: Standard chess rules and gameplay.
- **Chess960 (Fischer Random)**: Play Chess960 for added variety and challenge.
- **Player vs Player**: Play against another human locally.
- **Player vs Computer**: Challenge the Stockfish engine at various ELO levels.
- **Adjustable Difficulty**: Set engine skill or ELO for computer opponents.
- **Move Legality and Validation**: The engine checks for legal moves, including special rules like en passant, castling, and pawn promotion.
- **Check Detection**: Visual and logical indication when a king is in check.
- **Move History**: Track and review move history.
- **FEN Support**: Position serialization for saving/loading games and engine communication.
- **UCI Protocol Integration**: Communicates with Stockfish using the UCI protocol.
- **Promotion Dialog**: GUI for pawn promotion choices.
- **Customizable Board and Pieces**: Support for custom images for the board and pieces.
- **Syzygy Tablebase Support**: Endgame tablebase probing for perfect play in simplified positions.

## Getting Started

### Prerequisites

- C++17 or newer compiler (GCC, Clang, MSVC)
- CMake (for build configuration)
- Make (optional, for Unix-like systems)

### Building

```sh
git clone https://github.com/incep-tives/Chess.git
cd Chess
mkdir build
cd build
cmake ..
make
```

### Running

After building, run the executable generated in the `build` directory:

```sh
./Chess
```

### Configuration

- **Engine Strength**: Adjustable via dialogs or configuration settings.
- **Game Mode**: Choose between Player vs Player, Player vs Computer, or Chess960.
- **Color Selection**: Select to play as White or Black.

## Project Structure

- `/game/` – Core chess logic (board, move generation, game state)
- `/stockfish/` – Integrated Stockfish chess engine sources
- `main.cpp` – Application entry point and GUI logic

## License

This project incorporates Stockfish, which is licensed under the [GNU GPL v3](https://www.gnu.org/licenses/gpl-3.0.html). See `stockfish/` for details.

## Credits

- [Stockfish Developers](https://stockfishchess.org/)
- Chess application by [@incep-tives](https://github.com/incep-tives)

## Contributing

Pull requests and suggestions are welcome! Please open an issue or submit a PR.
