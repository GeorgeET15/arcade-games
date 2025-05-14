# Arcade Games Repository

Welcome to the `arcade-games` repository! This collection includes a set of classic 2D games built using the Arcade Library, designed to run on both Linux and Windows platforms. Each game showcases different aspects of game development, such as sprite rendering, collision detection, game states, and input handling.

## Games Included

### 1. Asteroids

A classic space shooter where you control a spaceship, shoot asteroids, and avoid collisions. Features include:

- Player movement and rotation using arrow keys.
- Shooting mechanics with spacebar.
- Collision detection and scoring.
- Game states (Start, Playing, Game Over).

### 2. Paddleball

A simple breakout-style game where you bounce a ball with a paddle to break bricks. Features include:

- Paddle movement with arrow keys.
- Ball physics with bouncing mechanics.
- Brick-breaking and scoring.
- Game states (Start, Playing, Won, Lost).

### 3. Flappy Bird

A clone of the popular Flappy Bird game where you navigate a bird through a series of pipes. Features include:

- Bird movement with spacebar to flap.
- Randomly generated pipes for endless gameplay.
- Collision detection with pipes and ground.
- Scoring based on pipes passed.

### 4. Super Jump Adventure

A 2D platformer where you control a character to jump, shoot, and reach a flag while avoiding enemies. Features include:

- Player movement (left/right), jumping (double jump), and shooting with spacebar.
- Platform navigation with coyote time for better jump mechanics.
- Enemies that patrol platforms and can be defeated with bullets.
- Game states (Start, Playing, Won, Lost) with best-time tracking.
- Sprite animations for player and enemies.

## Getting Started

### Prerequisites

- **Operating System**: Linux (requires `libX11` and `libm`) or Windows (requires `gdi32` and `winmm` for MinGW).
- **Compiler**: GCC (or MinGW on Windows).
- **Dependencies**: The Arcade Library (`arcade.h` and `arcade.c`) is included in each game directory. It uses the STB library for image loading (bundled within `arcade.c`).
- **Assets**: Each game requires sprite assets located in the `./assets/sprites/` directory relative to the game’s source file. Ensure these assets are present:
  - Asteroids: `spaceship.png`, `asteroid.png`, `bullet.png`.
  - Paddleball: `paddle.png`, `ball.png`, `brick.png`.
  - Flappy Bird: `bird.png`, `pipe.png`, `background.png`.
  - Super Jump Adventure: `background.png`, `player-run-1.png` to `player-run-4.png`, `player-idle.png`, `platform.png`, `enemy-run-1.png` to `enemy-run-3.png`, `flag.png`, `bullet.png`.

### Directory Structure

```
arcade-games/
│
├── asteroids/
│   ├── asteroids.c
│   ├── arcade.h
│   ├── arcade.c
│   └── assets/
│       └── sprites/
│           ├── spaceship.png
│           ├── asteroid.png
│           └── bullet.png
│
├── paddleball/
│   ├── paddleball.c
│   ├── arcade.h
│   ├── arcade.c
│   └── assets/
│       └── sprites/
│           ├── paddle.png
│           ├── ball.png
│           └── brick.png
│
├── flappybird/
│   ├── flappybird.c
│   ├── arcade.h
│   ├── arcade.c
│   └── assets/
│       └── sprites/
│           ├── bird.png
│           ├── pipe.png
│           └── background.png
│
└── superjumpadventure/
    ├── super_jump_adventure.c
    ├── arcade.h
    ├── arcade.c
    └── assets/
        └── sprites/
            ├── background.png
            ├── player-run-1.png
            ├── player-run-2.png
            ├── player-run-3.png
            ├── player-run-4.png
            ├── player-idle.png
            ├── platform.png
            ├── enemy-run-1.png
            ├── enemy-run-2.png
            ├── enemy-run-3.png
            ├── flag.png
            └── bullet.png
```

### Compilation and Running

#### Linux

1. Navigate to the game directory (e.g., `cd asteroids`).
2. Compile the game:
   ```bash
   gcc -D_POSIX_C_SOURCE=199309L -o game game.c arcade.c -lX11 -lm
   ```
   Replace `game.c` with the specific game file (e.g., `asteroids.c`, `paddleball.c`, `flappybird.c`, or `super_jump_adventure.c`), and `game` with the desired executable name (e.g., `asteroids`, `paddleball`, `flappybird`, or `superjump`).
3. Run the game:
   ```bash
   ./game
   ```

#### Windows (MinGW)

1. Navigate to the game directory (e.g., `cd asteroids`).
2. Compile the game:
   ```bash
   gcc -o game game.c arcade.c -lgdi32 -lwinmm
   ```
   Replace `game.c` and `game` as described above.
3. Run the game:
   ```bash
   game.exe
   ```

### Controls

- **Asteroids**:
  - Left/Right Arrows: Rotate spaceship.
  - Up Arrow: Thrust forward.
  - Space: Shoot bullets.
  - ESC: Quit.
- **Paddleball**:
  - Left/Right Arrows: Move paddle.
  - Space (Start/Won/Lost): Start or restart game.
  - ESC: Quit.
- **Flappy Bird**:
  - Space: Flap wings to fly.
  - ESC: Quit.
- **Super Jump Adventure**:
  - Left/Right Arrows: Move player.
  - Up Arrow: Jump (double jump supported).
  - Space: Shoot bullets.
  - ESC: Quit.
  - Space (Start): Start game.
  - R (Won/Lost): Restart game.

## Notes

- Ensure all sprite assets are in the correct `assets/sprites/` directory for each game, or the game will fail to load.
- The games are designed to run at approximately 60 FPS, using `arcade_sleep(16)` to maintain consistent frame timing.
- Some games (e.g., Super Jump Adventure) include advanced features like frame-rate-independent movement using `arcade_delta_time()`. Others (e.g., Asteroids, Paddleball) may require updates for better performance on varying frame rates.
- Background music or sound effects may be present in some games (e.g., Super Jump Adventure expects `background_music.wav` in the `assets/` directory if added). Ensure these files exist if the game attempts to load them.

## Contributing

Feel free to fork this repository, add new games, or improve existing ones! If you encounter bugs or have suggestions, please open an issue or submit a pull request.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details (if applicable).
