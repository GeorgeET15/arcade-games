/* =========================================================================
 * Flappy Bird Recreation - Documentation
 * =========================================================================
 * Author: GeorgeET15
 * Description:
 * A simplified Flappy Bird clone built using the Arcade Library. The player
 * controls an animated bird that navigates through pairs of moving pipes,
 * scoring points for each pair passed. The game features Start, Playing,
 * Paused, and GameOver states, animated sprites, dynamic pipe speed, audio
 * feedback, and a high score system. It is cross-platform, supporting Windows
 * (Win32) and Linux (X11).
 *
 * Category: Arcade/Endless Runner
 * - Emphasizes reflex-based gameplay, high-score chasing, and simple mechanics,
 *   typical of Flappy Bird and similar mobile arcade games.
 *
 * Controls:
 * - Space: Jump (Playing state), Start game (Start state)
 * - P: Pause/Unpause (Playing/Paused states)
 * - R: Restart game (GameOver state)
 * - ESC: Quit (closes the window)
 *
 * Compilation:
 * Linux:
 *   gcc -D_POSIX_C_SOURCE=199309L -o flappy flappybird.c arcade.c -lX11 -lm
 * Windows (MinGW):
 *   gcc -o flappy flappybird.c arcade.c -lgdi32 -lwinmm
 * Run:
 *   Linux: ./flappy
 *   Windows: flappy.exe
 * Ensure sprite files are in ./assets/sprites/ and audio files in ./assets/audio/.
 *
 * Sprite Files (relative to executable):
 * - background.png: ~800x600 PNG, game background
 * - bluebird.png: ~40x40 PNG, bird frame 1 (upflap)
 * - bluebird-midflap.png: ~40x40 PNG, bird frame 2 (midflap)
 * - bluebird-downflap.png: ~40x40 PNG, bird frame 3 (downflap)
 * - pipe-top.png: ~50xvariable PNG, top pipe
 * - pipe-bottom.png: ~50xvariable PNG, bottom pipe
 *
 * Audio Files (relative to executable):
 * - sfx_wing.wav: Sound for bird jump
 * - sfx_point.wav: Sound for scoring a point
 * - sfx_die.wav: Sound for collision or ground hit
 * - pause.wav: Sound for pause/unpause toggle
 *
 * Dependencies:
 * - Arcade Library (arcade.h, arcade.c)
 * - STB libraries (included via arcade.c for image loading/resizing)
 * - Linux: libX11, libm, aplay (alsa-utils for audio)
 * - Windows: gdi32, winmm
 *
 * Notes:
 * - Ensure WAV files are PCM, 16-bit, mono/stereo for compatibility.
 * - Sprite paths are hardcoded; ensure files exist in ./assets/sprites/.
 * - Uses arcade_delta_time for frame-rate-independent movement, scaled to 60 FPS.
 * - High score persists in memory during a session but resets on exit.
 * - arcade_sleep(16) targets ~60 FPS; consider removing for full frame-rate
 *   independence.
 * - Dynamic pipe speed increases difficulty; capped to maintain playability.
 * - Animation runs at ~6 FPS (10-frame interval) for smooth flapping.
 * ========================================================================= */

#define ARCADE_IMPLEMENTATION
#include "arcade.h"

/* =========================================================================
 * Game Constants
 * =========================================================================
 * Define core game parameters, balancing gameplay difficulty and visuals.
 * Adjust these to tweak the game’s feel (e.g., pipe gap, spawn frequency).
 */
#define MAX_PIPES 6      /* Maximum number of pipe pairs (top + bottom). Limits memory usage and rendering load. */
#define PIPE_WIDTH 50.0f /* Width of each pipe sprite (pixels). Matches sprite dimensions for accurate collisions. */
#define PIPE_GAP 135.0f  /* Vertical gap between top and bottom pipes (pixels). Adjust for difficulty. */
#define SPAWN_FRAMES 120 /* Frames between pipe spawns (~2 seconds at 60 FPS). Controls pipe frequency. */

/* =========================================================================
 * GameState Enum
 * =========================================================================
 * Defines the possible game states, used to manage different modes of
 * operation (e.g., rendering different UI, handling specific inputs).
 * - Start: Initial screen with instructions.
 * - Playing: Active gameplay with bird movement and pipe spawning.
 * - Paused: Frozen state with pause message.
 * - GameOver: End state with score and restart prompt.
 */
typedef enum
{
    Start,   /* Start screen, waiting for player to press Space */
    Playing, /* Active gameplay, bird and pipes move */
    Paused,  /* Paused gameplay, no movement */
    GameOver /* Game over, shows score and restart option */
} GameState;

/* =========================================================================
 * Pipe Structure
 * =========================================================================
 * Represents a single pipe (top or bottom) with its sprite and scoring state.
 * - sprite: ArcadeImageSprite for rendering and movement.
 * - scored: Flag (0 or 1) to track if the pipe pair has been scored, preventing
 *           multiple scores for the same pair.
 */
typedef struct
{
    ArcadeImageSprite sprite; /* Pipe’s sprite (position, velocity, pixels) */
    int scored;              /* 1 if scored, 0 otherwise */
} Pipe;

/* =========================================================================
 * add_pipe_pair Function
 * =========================================================================
 * Creates a pair of pipes (top and bottom) with a random vertical gap position
 * and adds them to the pipes array. Ensures proper cleanup if sprite loading
 * fails.
 * Parameters:
 * - pipes: Array of Pipe structs to store the new pipes.
 * - pipe_count: Pointer to the current number of pipes (updated on addition).
 * - window_width: Window width (pixels), used for spawn position.
 * - window_height: Window height (pixels), used for pipe sizing.
 * - speed: Horizontal velocity (negative for leftward movement, pixels/frame at 60 FPS).
 * Returns: None.
 * Example:
 *   add_pipe_pair(pipes, &pipe_count, 800, 600, -3.0f); // Spawn pipe pair moving left
 * Notes:
 * - Gap’s y-position is randomized between 200 and 350 pixels for variability.
 * - If either sprite fails to load, both are discarded to maintain pair integrity.
 * - Pipes spawn just off-screen (x = window_width) for smooth entry.
 * - Speed is scaled by delta time in the game loop for frame-rate independence.
 */
void add_pipe_pair(Pipe *pipes, int *pipe_count, float window_width, float window_height, float speed)
{
    if (*pipe_count >= MAX_PIPES - 1) return; /* Prevent array overflow; reserve space for top and bottom pipes */
    float gap_y = 200.0f + (rand() % 150);   /* Random gap y-position (200–350 pixels) for variability in pipe placement */

    /* Create top pipe, extending from y=0 to gap_y */
    ArcadeImageSprite top = arcade_create_image_sprite(window_width, 0.0f, PIPE_WIDTH, gap_y, "./assets/sprites/pipe-top.png");
    if (!top.pixels) return; /* Skip if image fails to load (e.g., file missing or invalid format) */
    top.vx = speed;          /* Set leftward velocity for pipe movement */
    pipes[*pipe_count].sprite = top; /* Store top pipe in array */
    pipes[*pipe_count].scored = 0;   /* Initialize as not scored */
    (*pipe_count)++;                 /* Increment pipe count */

    /* Create bottom pipe, starting at gap_y + PIPE_GAP to the window bottom */
    ArcadeImageSprite bottom = arcade_create_image_sprite(window_width, gap_y + PIPE_GAP, PIPE_WIDTH, window_height - gap_y - PIPE_GAP, "./assets/sprites/pipe-bottom.png");
    if (!bottom.pixels)
    { /* If bottom fails, clean up top to maintain pair integrity */
        arcade_free_image_sprite(&pipes[--(*pipe_count)].sprite); /* Free top pipe */
        return;
    }
    bottom.vx = speed;               /* Set same leftward velocity as top pipe */
    pipes[*pipe_count].sprite = bottom; /* Store bottom pipe in array */
    pipes[*pipe_count].scored = 0;   /* Initialize as not scored (shared with top for scoring) */
    (*pipe_count)++;                 /* Increment pipe count */
}

/* =========================================================================
 * main Function
 * =========================================================================
 * Entry point for the game. Initializes the Arcade environment, sets up
 * sprites, manages the game loop, and handles cleanup. The game loop processes
 * input, updates game state, and renders the scene at ~60 FPS using
 * arcade_delta_time for frame-rate-independent movement.
 * Parameters: None.
 * Returns:
 * - 0 on successful exit.
 * - 1 if initialization fails (e.g., window creation, sprite loading).
 * Example:
 *   int main(void) {
 *       arcade_init(800, 600, "Flappy Bird", 0x00B7EB);
 *       while (arcade_running() && arcade_update()) {
 *           float dt = arcade_delta_time();
 *           arcade_move_animated_sprite(&player, gravity * dt * 60.0f, window_height); // Apply gravity
 *           arcade_render_group(&group);
 *       }
 *       arcade_quit();
 *       return 0;
 *   }
 * Notes:
 * - Initializes random seed for pipe gap randomization.
 * - Uses arcade_sleep(16) for approximate 60 FPS; consider removing for full
 *   frame-rate independence.
 * - Cleans up all sprites and Arcade resources on exit.
 * - Dynamic pipe speed increases with score, capped for balance.
 * - Animation runs at ~6 FPS (10-frame interval) for smooth flapping.
 * - Prints score updates and final score to console for debugging.
 */
int main(void)
{
    /* Seed random number generator for pipe gap randomization */
    srand(time(NULL));

    /* Window and physics parameters */
    int window_width = 800;      /* Window width (pixels), matches background sprite dimensions */
    int window_height = 600;     /* Window height (pixels), defines play area for bird and pipes */
    float gravity = 0.2f;        /* Gravity acceleration (pixels/frame^2 at 60 FPS), pulls bird downward */
    float jump_vy = -6.0f;       /* Upward velocity on jump (pixels/frame at 60 FPS, negative = up) */

    /* Game state variables */
    GameState state = Start;     /* Start in Start state (shows instructions and waits for input) */
    int score = 0;               /* Current player score (number of pipe pairs passed) */
    int high_score = 0;          /* Highest score in session, persists across restarts */
    int pipe_count = 0;          /* Number of active pipes (top + bottom, stored in pairs) */
    int next_pipe = 60;          /* Frames until next pipe spawn (initial delay, ~1s at 60 FPS) */
    char text[64];               /* Buffer for rendering score and game messages */

    /* Initialize background sprite (static, covers entire window) */
    ArcadeImageSprite background = arcade_create_image_sprite(0.0f, 0.0f, window_width, window_height, "./assets/sprites/background.png");

    /* Initialize animated bird sprite with three frames for flapping animation */
    const char *bird_frames[] = {
        "./assets/sprites/bluebird.png",          /* Frame 1: Upflap */
        "./assets/sprites/bluebird-midflap.png",  /* Frame 2: Midflap */
        "./assets/sprites/bluebird-downflap.png"  /* Frame 3: Downflap */
    };
    ArcadeAnimatedSprite player = arcade_create_animated_sprite(
        100.0f, 300.0f, 40.0f, 40.0f, bird_frames, 3, 10
    ); /* x=100 (left side), y=300 (vertical center), 40x40 pixels, 3 frames, 10-frame interval (~6 FPS animation) */

    /* Initialize pipe array and sprite group for rendering */
    Pipe pipes[MAX_PIPES] = {0}; /* Array to store up to MAX_PIPES pipes (top and bottom) */
    SpriteGroup group;
    arcade_init_group(&group, MAX_PIPES + 2); /* Capacity for background, player, and all pipes */

    /* Initialize Arcade environment (window, rendering, input) */
    if (!background.pixels || !player.frames || arcade_init(window_width, window_height, "Flappy Bird", 0x00B7EB))
    {
        fprintf(stderr, "Initialization failed: background=%p, player.frames=%p\n", background.pixels, player.frames);
        arcade_free_image_sprite(&background); /* Free background if initialization fails */
        arcade_free_animated_sprite(&player);  /* Free bird animation if initialization fails */
        arcade_free_group(&group);             /* Free sprite group */
        return 1; /* Exit if window creation or sprite loading fails */
    }

    /* Main game loop: runs until window is closed or ESC is pressed */
    while (arcade_running() && arcade_update())
    {
        /* Get delta time for frame-rate-independent movement */
        float delta_time = arcade_delta_time();
        float scale = delta_time * 60.0f; /* Normalize to 60 FPS for consistent speed */

        /* Update score display (rendered every frame) */
        snprintf(text, sizeof(text), "Score: %d", score);

        /* Reset sprite group to rebuild with current sprites (needed for animated player) */
        group.count = 0; /* Clear previous frame’s sprites for fresh rendering */

        /* Add background, player, and pipes to render group */
        arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.image_sprite = background}, SPRITE_IMAGE); /* Static background */
        arcade_add_animated_to_group(&group, &player); /* Adds current bird frame based on animation state */
        for (int i = 0; i < pipe_count; i++)
        {
            arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.image_sprite = pipes[i].sprite}, SPRITE_IMAGE); /* Add active pipes */
        }

        /* Render the scene (clears screen, draws sprites, updates window) */
        arcade_render_group(&group);
        arcade_render_text(text, 10.0f, 30.0f, 0xFFFFFF); /* Score in top-left (white) */

        /* Calculate dynamic pipe speed (increases with score, capped for balance) */
        float pipe_speed = -3.0f - (score / 10) * 0.5f; /* Base -3.0, increases by 0.5 per 10 points, pixels/frame at 60 FPS */
        if (pipe_speed < -6.0f) pipe_speed = -6.0f;     /* Cap at -6.0 to prevent unplayable difficulty */

        /* Handle game logic based on current state */
        switch (state)
        {
        case Start:
            /* Show blinking start prompt and high score */
            arcade_render_text_centered_blink("Press Space to Start", 300.0f, 0xFFFFFF, 30); /* Blinks every 0.5s at 60 FPS */
            snprintf(text, sizeof(text), "High Score: %d", high_score);
            arcade_render_text_centered(text, 350.0f, 0xFFFFFF); /* High score below prompt */
            if (arcade_key_pressed_once(a_space) == 2)
            {
                arcade_clear_keys(); /* Clear input to prevent immediate jump in Playing state */
                state = Playing;     /* Transition to gameplay */
            }
            break;

        case Playing:
            /* Handle pause toggle */
            if (arcade_key_pressed_once(a_p) == 2)
            {
                arcade_play_sound("./assets/audio/pause.wav"); /* Play pause sound effect */
                state = Paused; /* Freeze gameplay, no movement or updates */
            }

            /* Handle jump input (Space key) */
            if (arcade_key_pressed_once(a_space) == 2)
            {
                player.frames[player.current_frame].vy = jump_vy; /* Apply upward velocity to current frame */
                arcade_play_sound("./assets/audio/sfx_wing.wav"); /* Play wing flap sound */
            }

            /* Update player position (applies gravity, clamps to window) and animation */
            arcade_move_animated_sprite(&player, gravity * scale, window_height); /* Apply gravity scaled by delta time, clamp to window height */

            /* Update pipes and check scoring/collisions */
            for (int i = 0; i < pipe_count; i++)
            {
                pipes[i].sprite.vx = pipe_speed; /* Update velocity to current dynamic speed */
                arcade_move_image_sprite(&pipes[i].sprite, 0.0f, window_height); /* Move pipe left, scaled by delta time inside arcade_move_image_sprite */

                /* Score when bird passes bottom pipe (odd index, i % 2 == 1) */
                if (i % 2 == 1 && !pipes[i].scored && pipes[i].sprite.x + PIPE_WIDTH < player.frames[0].x)
                {
                    score++; /* Increment score for passing pipe pair */
                    if (score > high_score) high_score = score; /* Update high score if current score exceeds it */
                    pipes[i].scored = 1;     /* Mark bottom pipe as scored */
                    pipes[i - 1].scored = 1; /* Mark corresponding top pipe as scored */
                    arcade_play_sound("./assets/audio/sfx_point.wav"); /* Play score sound effect */
                    printf("Score incremented: %d\n", score); /* Debug output to console */
                }

                /* Check collision with active pipes */
                if (pipes[i].sprite.active && arcade_check_animated_collision(&player, &pipes[i].sprite))
                {
                    arcade_play_sound("./assets/audio/sfx_die.wav"); /* Play crash sound effect */
                    state = GameOver;                        /* Transition to GameOver state */
                    player.frames[0].active = 0;             /* Disable player rendering */
                }
            }

            /* Check for ground collision (hardcoded ground at window_height) */
            if (player.frames[0].y + player.frames[0].height >= window_height)
            {
                arcade_play_sound("./assets/audio/sfx_die.wav"); /* Play crash sound effect */
                state = GameOver; /* Transition to GameOver state */
                player.frames[0].active = 0; /* Disable player rendering */
            }

            /* Spawn new pipe pair after countdown (adjusted for delta time) */
            next_pipe -= scale; /* Decrease spawn timer, scaled by delta time */
            if (next_pipe <= 0)
            {
                add_pipe_pair(pipes, &pipe_count, window_width, window_height, pipe_speed); /* Spawn new pipe pair */
                next_pipe = SPAWN_FRAMES; /* Reset spawn timer (~2s at 60 FPS) */
            }

            /* Remove off-screen pipes to free memory and make room for new ones */
            if (pipe_count && pipes[0].sprite.x + PIPE_WIDTH < 0)
            {
                arcade_free_image_sprite(&pipes[0].sprite); /* Free top pipe’s sprite memory */
                arcade_free_image_sprite(&pipes[1].sprite); /* Free bottom pipe’s sprite memory */
                for (int i = 0; i < pipe_count - 2; i++) pipes[i] = pipes[i + 2]; /* Shift array to remove first pair */
                pipe_count -= 2; /* Reduce count by 2 (one pipe pair) */
            }
            break;

        case Paused:
            /* Show pause message (no movement or spawning occurs) */
            arcade_render_text_centered("Paused - Press P", 300.0f, 0xFFFFFF); /* Display pause prompt */
            if (arcade_key_pressed_once(a_p) == 2)
            {
                arcade_play_sound("./assets/audio/pause.wav"); /* Play unpause sound effect */
                state = Playing; /* Resume gameplay */
            }
            break;

        case GameOver:
            /* Show game over message with current score and high score */
            snprintf(text, sizeof(text), "Game Over! Score: %d. High Score: %d. Press R", score, high_score);
            arcade_render_text_centered(text, 300.0f, 0xFFFFFF); /* Display game over message */

            /* Handle restart input */
            if (arcade_key_pressed_once(a_r) == 2)
            {
                arcade_clear_keys(); /* Clear input to prevent immediate actions in Playing state */

                /* Reset player state */
                player.frames[0].x = 100.0f; /* Reset to starting x-position (left side) */
                player.frames[0].y = 300.0f; /* Reset to starting y-position (vertical center) */
                player.frames[0].vy = 0.0f;  /* Clear vertical velocity */
                player.frames[0].active = 1;  /* Re-enable player rendering */
                player.current_frame = 0;     /* Reset animation to first frame (upflap) */
                player.frame_counter = 0;     /* Reset animation timer */

                /* Sync other animation frames to first frame’s position and state */
                for (int i = 1; i < player.frame_count; i++)
                {
                    player.frames[i].x = 100.0f; /* Match first frame’s x */
                    player.frames[i].y = 300.0f; /* Match first frame’s y */
                    player.frames[i].vy = 0.0f;  /* Clear velocity */
                }

                /* Clear all pipes */
                for (int i = 0; i < pipe_count; i++)
                {
                    arcade_free_image_sprite(&pipes[i].sprite); /* Free each pipe’s sprite memory */
                }
                pipe_count = 0; /* Reset pipe count to 0 */

                /* Reset game state */
                score = 0;        /* Reset score (high_score persists) */
                next_pipe = 60;   /* Reset initial delay for first pipe (~1s at 60 FPS) */
                state = Playing;  /* Restart gameplay */
            }
            break;
        }

        /* Sleep for ~16ms to target 60 FPS (optional) */
        arcade_sleep(16);
    }

    /* Clean up all resources before exit */
    arcade_free_image_sprite(&background); /* Free background sprite memory */
    arcade_free_animated_sprite(&player);  /* Free bird animation frames */
    for (int i = 0; i < pipe_count; i++)
    {
        arcade_free_image_sprite(&pipes[i].sprite); /* Free any remaining pipe sprites */
    }
    arcade_free_group(&group); /* Free sprite group memory */
    arcade_quit();            /* Close window and release Arcade resources */

    /* Print final score and high score to console */
    printf("Game Over! Final Score: %d, High Score: %d\n", score, high_score);
    return 0;
}