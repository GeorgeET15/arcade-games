/* =========================================================================
 * Paddle Ball - Documentation
 * =========================================================================
 * Author: GeorgeET15
 * Description:
 * A Breakout-style arcade game built using the Arcade Library. The player
 * controls a paddle to bounce a ball, breaking a 5x10 grid of colored bricks.
 * Each brick awards 10 points, and the game ends when all lives are lost (ball
 * falls off the bottom) or all bricks are destroyed (win). The game features
 * three states (Start, Playing, GameOver), a high score system, and a lives
 * system. It uses color-based sprites and is cross-platform (Windows with
 * Win32, Linux with X11).
 *
 * Category: Arcade/Score-Attack
 * - Emphasizes simple, addictive mechanics, high-score chasing, and reflex-based
 *   gameplay, typical of arcade games like Breakout or Arkanoid.
 *
 * Controls:
 * - Left Arrow: Move paddle left (Playing state)
 * - Right Arrow: Move paddle right (Playing state)
 * - Space: Start game (Start state), Release ball (Playing state, if stuck)
 * - R: Restart game (GameOver state)
 * - ESC: Quit (closes the window)
 *
 * Compilation:
 * Linux:
 *   gcc -D_POSIX_C_SOURCE=199309L -o paddleball paddleball.c arcade.c -lX11 -lm
 * Windows (MinGW):
 *   gcc -o paddleball paddleball.c arcade.c -lgdi32 -lwinmm
 * Run:
 *   Linux: ./paddleball
 *   Windows: paddleball.exe
 *
 * Optional Assets:
 * - Audio files (relative to executable, PCM 16-bit WAV):
 *   - hit.wav: Sound for paddle/ball or wall/ball collision
 *   - break.wav: Sound for breaking a brick
 * - Sprite files (for image-based visuals, PNG):
 *   - paddle.png: ~100x20 paddle sprite
 *   - ball.png: ~10x10 ball sprite
 *   - brick.png: ~50x20 brick sprite
 *
 * Dependencies:
 * - Arcade Library (arcade.h, arcade.c)
 * - STB libraries (included via arcade.c, used only if image sprites are added)
 * - Linux: libX11, libm, aplay (for audio, if used)
 * - Windows: gdi32, winmm
 *
 * Notes:
 * - Uses color-based sprites (`ArcadeSprite`) for simplicity; image sprites can
 *   be added with `ArcadeImageSprite`.
 * - Lives system (3 lives) balances difficulty; game ends on 0 lives or all
 *   bricks destroyed.
 * - High score persists in memory during a session but resets on exit.
 * - Uses arcade_delta_time for frame-rate-independent movement, scaled to 60 FPS.
 * - Optional audio support for hit and break sounds; assets not required.
 * - Ball physics use simple vector reflection; could add spin or variable speed.
 * - arcade_sleep(16) targets ~60 FPS; consider removing for full frame-rate
 *   independence.
 * ========================================================================= */

#define ARCADE_IMPLEMENTATION
#include "arcade.h"
#include <math.h>

/* =========================================================================
 * Game Constants
 * =========================================================================
 * Define core game parameters, controlling the play area, objects, and
 * difficulty. Adjust these to tweak gameplay feel or challenge.
 */
#define WINDOW_WIDTH 800       /* Window width (pixels). Wide for paddle movement and brick grid. */
#define WINDOW_HEIGHT 600      /* Window height (pixels). Tall for brick grid and play area. */
#define MAX_BRICKS 50          /* Maximum number of bricks (5 rows x 10 columns). */
#define PADDLE_WIDTH 100.0f    /* Paddle width (pixels). Wide for easier catching. */
#define PADDLE_HEIGHT 20.0f    /* Paddle height (pixels). Thin for aesthetics. */
#define BALL_SIZE 10.0f        /* Ball width/height (pixels). Small for precision. */
#define BRICK_WIDTH 76.0f      /* Brick width (pixels). Fits 10 per row with spacing. */
#define BRICK_HEIGHT 20.0f     /* Brick height (pixels). Short for compact grid. */

/* =========================================================================
 * GameState Enum
 * =========================================================================
 * Defines the possible game states, used to manage different modes of
 * operation (e.g., rendering UI, handling inputs).
 * - Start: Initial screen with instructions.
 * - Playing: Active gameplay with paddle, ball, and bricks.
 * - GameOver: End state with score and restart prompt.
 */
typedef enum
{
    Start,   /* Start screen, waiting for Space to begin */
    Playing, /* Active gameplay, paddle and ball move */
    GameOver /* Game over, shows score and restart option */
} GameState;

/* =========================================================================
 * Brick Structure
 * =========================================================================
 * Represents a single brick with its sprite properties.
 * - sprite: ArcadeSprite for position, size, color, and active state.
 * Note: Uses color-based sprites (varied colors per row); could use
 * `ArcadeImageSprite` for textured bricks.
 */
typedef struct
{
    ArcadeSprite sprite; /* Brick’s sprite (position, size, color) */
} Brick;

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
 * - 1 if initialization fails (e.g., window creation).
 * Example:
 *   int main(void) {
 *       arcade_init(WINDOW_WIDTH, WINDOW_HEIGHT, "Paddle Ball", 0x000000);
 *       while (arcade_running() && arcade_update()) {
 *           float dt = arcade_delta_time();
 *           paddle.x += paddle.vx * dt * 60.0f; // Move paddle
 *           arcade_render_group(&group);
 *       }
 *       arcade_quit();
 *       return 0;
 *   }
 * Notes:
 * - Initializes random seed for ball’s initial direction.
 * - Uses arcade_sleep(16) for approximate 60 FPS; consider removing for full
 *   frame-rate independence.
 * - Cleans up sprite group and Arcade resources on exit.
 * - Prints final score and high score to console on exit.
 * - Ball physics are simplified (reflection-based); could add spin or speed
 *   variation.
 * - Optional audio assets enhance feedback but are not required.
 */
int main(void)
{
    /* Seed random number generator for ball’s initial direction */
    srand(time(NULL));

    /* Game parameters */
    float paddle_speed = 8.0f;       /* Paddle’s horizontal speed (pixels/frame at 60 FPS) */
    float ball_speed = 6.0f;         /* Ball’s total speed (pixels/frame at 60 FPS, split into vx/vy) */
    int lives = 3;                   /* Starting lives; lose one per ball drop */
    int score = 0;                   /* Current player score (bricks broken, 10 points each) */
    int high_score = 0;              /* Highest score in session, persists across restarts */
    char text[64];                   /* Buffer for rendering score and lives */
    char textGameOver[64];           /* Buffer for game over message */
    char textHighScore[64];          /* Buffer for high score message */
    char textRestart[64];            /* Buffer for restart prompt */
    GameState state = Start;         /* Start in Start state (shows instructions) */
    int ball_stuck = 1;              /* 1 if ball is stuck to paddle, 0 if moving */

    /* Initialize paddle sprite (blue rectangle, near bottom-center) */
    ArcadeSprite paddle = {
        .x = WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, /* Center horizontally */
        .y = WINDOW_HEIGHT - 50.0f,               /* Near bottom of screen */
        .width = PADDLE_WIDTH,
        .height = PADDLE_HEIGHT, /* 100x20 rectangle */
        .vx = 0.0f,
        .vy = 0.0f,              /* No initial velocity */
        .color = 0x0000FF,       /* Blue */
        .active = 1              /* Visible and collidable */
    };
    /* Note: For image sprite, could use: ArcadeImageSprite paddle = arcade_create_image_sprite(x, y, PADDLE_WIDTH, PADDLE_HEIGHT, "./assets/paddle.png"); */

    /* Initialize ball sprite (white square, starts on paddle) */
    ArcadeSprite ball = {
        .x = paddle.x + PADDLE_WIDTH / 2 - BALL_SIZE / 2, /* Center on paddle */
        .y = paddle.y - BALL_SIZE,                        /* Just above paddle */
        .width = BALL_SIZE,
        .height = BALL_SIZE, /* 10x10 square */
        .vx = 0.0f,
        .vy = 0.0f,          /* No initial velocity (stuck to paddle) */
        .color = 0xFFFFFF,   /* White */
        .active = 1          /* Visible and collidable */
    };
    /* Note: For image sprite, could use: ArcadeImageSprite ball = arcade_create_image_sprite(x, y, BALL_SIZE, BALL_SIZE, "./assets/ball.png"); */

    /* Initialize bricks array (colored rectangles, 5 rows x 10 columns) */
    Brick bricks[MAX_BRICKS];
    int brick_count = 0;
    unsigned int row_colors[] = {0xFF0000, 0xFF9900, 0xFFFF00, 0x00FF00, 0x00FFFF}; /* Red, Orange, Yellow, Green, Cyan */
    for (int row = 0; row < 5; row++)
    {
        for (int col = 0; col < 10; col++)
        {
            bricks[brick_count].sprite.x = col * (BRICK_WIDTH + 4.0f) + 20.0f; /* 4-pixel gap between bricks, 20-pixel left margin */
            bricks[brick_count].sprite.y = row * (BRICK_HEIGHT + 4.0f) + 50.0f; /* 4-pixel gap between rows, 50-pixel top margin */
            bricks[brick_count].sprite.width = BRICK_WIDTH;   /* 76x20 rectangle */
            bricks[brick_count].sprite.height = BRICK_HEIGHT;
            bricks[brick_count].sprite.vx = 0.0f;            /* Static, no movement */
            bricks[brick_count].sprite.vy = 0.0f;
            bricks[brick_count].sprite.color = row_colors[row]; /* Assign color based on row */
            bricks[brick_count].sprite.active = 1;            /* Visible and collidable */
            brick_count++;
        }
    }
    /* Note: For image sprites, could use: ArcadeImageSprite brick = arcade_create_image_sprite(x, y, BRICK_WIDTH, BRICK_HEIGHT, "./assets/brick.png"); */

    /* Initialize sprite group for rendering */
    SpriteGroup group;
    arcade_init_group(&group, MAX_BRICKS + 2); /* Capacity for paddle, ball, and all bricks */

    /* Initialize Arcade environment (window, rendering, input) */
    if (arcade_init(WINDOW_WIDTH, WINDOW_HEIGHT, "Paddle Ball", 0x000000) != 0)
    {
        arcade_free_group(&group); /* Free sprite group if initialization fails */
        fprintf(stderr, "Initialization failed\n");
        return 1; /* Exit if window creation fails */
    }

    /* Main game loop: runs until window is closed or ESC is pressed */
    while (arcade_running() && arcade_update())
    {
        /* Get delta time for frame-rate-independent movement */
        float delta_time = arcade_delta_time();
        float scale = delta_time * 60.0f; /* Normalize to 60 FPS for consistent speed */

        /* Update score and lives display (rendered every frame) */
        snprintf(text, sizeof(text), "Score: %d  Lives: %d", score, lives);

        /* Reset sprite group to rebuild with active sprites */
        group.count = 0; /* Clear previous frame’s sprites for fresh rendering */

        /* Add active sprites to render group (paddle, ball, bricks) */
        if (paddle.active)
        {
            arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.sprite = paddle}, SPRITE_COLOR); /* Add paddle if active */
        }
        if (ball.active)
        {
            arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.sprite = ball}, SPRITE_COLOR); /* Add ball if active */
        }
        for (int i = 0; i < brick_count; i++)
        {
            if (bricks[i].sprite.active)
            {
                arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.sprite = bricks[i].sprite}, SPRITE_COLOR); /* Add active bricks */
            }
        }

        /* Render the scene (clears screen, draws sprites, updates window) */
        arcade_render_group(&group);
        arcade_render_text(text, 10.0f, 30.0f, 0xFFFFFF); /* Score and lives in top-left (white) */

        /* Handle game logic based on current state */
        switch (state)
        {
        case Start:
            /* Show blinking start prompt and high score */
            arcade_render_text_centered_blink("Press Space to Start", WINDOW_HEIGHT / 2.0f, 0xFFFFFF, 30); /* Blinks every 0.5s at 60 FPS */
            snprintf(text, sizeof(text), "High Score: %d", high_score);
            arcade_render_text_centered(text, WINDOW_HEIGHT / 2.0f + 50.0f, 0xFFFFFF); /* High score below prompt */
            if (arcade_key_pressed_once(a_space) == 2)
            {
                arcade_clear_keys(); /* Clear input to prevent immediate ball release in Playing state */
                state = Playing;     /* Transition to gameplay */
            }
            break;

        case Playing:
            /* Handle paddle movement (left/right arrow keys) */
            if (arcade_key_pressed(a_right) == 2 && paddle.active)
            {
                paddle.vx = paddle_speed; /* Set rightward velocity */
            }
            else if (arcade_key_pressed(a_left) == 2 && paddle.active)
            {
                paddle.vx = -paddle_speed; /* Set leftward velocity */
            }
            else
            {
                paddle.vx = 0.0f; /* Stop movement if no keys pressed */
            }

            /* Update paddle position and clamp to window bounds */
            if (paddle.active)
            {
                paddle.x += paddle.vx * scale; /* Scale movement by delta time */
                if (paddle.x < 0)
                {
                    paddle.x = 0; /* Prevent moving off left edge */
                }
                else if (paddle.x + paddle.width > WINDOW_WIDTH)
                {
                    paddle.x = WINDOW_WIDTH - paddle.width; /* Prevent moving off right edge */
                }
            }

            /* Handle ball release (Space key, if stuck) */
            if (arcade_key_pressed_once(a_space) == 2 && ball_stuck)
            {
                ball_stuck = 0; /* Release ball from paddle */
                /* Set initial velocity with random horizontal direction (60–120 degrees) */
                float angle = (rand() % 60 + 60) * 3.14159f / 180.0f; /* Convert degrees to radians */
                ball.vx = ball_speed * cosf(angle); /* Horizontal component */
                ball.vy = -ball_speed * sinf(angle); /* Vertical component (upward) */
                arcade_play_sound("./assets/hit.wav"); /* Play optional launch sound */
            }

            /* Update ball position */
            if (!ball_stuck)
            {
                ball.x += ball.vx * scale; /* Scale horizontal movement by delta time */
                ball.y += ball.vy * scale; /* Scale vertical movement by delta time */

                /* Handle wall collisions */
                if (ball.x <= 0)
                { /* Left wall collision */
                    ball.x = 0; /* Clamp to edge */
                    ball.vx = -ball.vx; /* Reflect horizontally */
                    arcade_play_sound("./assets/hit.wav"); /* Play optional collision sound */
                }
                else if (ball.x + ball.width >= WINDOW_WIDTH)
                { /* Right wall collision */
                    ball.x = WINDOW_WIDTH - ball.width; /* Clamp to edge */
                    ball.vx = -ball.vx; /* Reflect horizontally */
                    arcade_play_sound("./assets/hit.wav");
                }
                if (ball.y <= 0)
                { /* Top wall collision */
                    ball.y = 0; /* Clamp to edge */
                    ball.vy = -ball.vy; /* Reflect vertically */
                    arcade_play_sound("./assets/hit.wav");
                }

                /* Handle paddle collision */
                if (arcade_check_collision(&ball, &paddle))
                {
                    ball.y = paddle.y - ball.height; /* Move ball above paddle to prevent sticking */
                    ball.vy = -ball.vy; /* Reflect vertically */
                    /* Adjust horizontal velocity based on hit position on paddle */
                    float hit_pos = (ball.x + ball.width / 2 - paddle.x) / paddle.width; /* 0 to 1, normalized hit position */
                    ball.vx = ball_speed * (hit_pos - 0.5f) * 2.0f; /* Scale from -ball_speed to +ball_speed */
                    arcade_play_sound("./assets/hit.wav"); /* Play optional collision sound */
                }

                /* Handle brick collisions */
                for (int i = 0; i < brick_count; i++)
                {
                    if (bricks[i].sprite.active && arcade_check_collision(&ball, &bricks[i].sprite))
                    {
                        bricks[i].sprite.active = 0; /* Destroy brick */
                        score += 10; /* Award 10 points per brick */
                        if (score > high_score)
                            high_score = score; /* Update high score if current score exceeds it */
                        /* Simple reflection: reverse vertical velocity (assumes top/bottom hit) */
                        ball.vy = -ball.vy;
                        arcade_play_sound("./assets/break.wav"); /* Play optional brick break sound */
                        break; /* Handle one collision per frame to avoid multiple hits */
                    }
                }

                /* Check if ball falls off bottom */
                if (ball.y + ball.height > WINDOW_HEIGHT)
                {
                    lives--; /* Lose one life */
                    if (lives <= 0)
                    {
                        state = GameOver; /* End game if no lives remain */
                        paddle.active = 0; /* Hide paddle */
                        ball.active = 0; /* Hide ball */
                    }
                    else
                    {
                        /* Reset ball to paddle for next attempt */
                        ball_stuck = 1;
                        ball.x = paddle.x + PADDLE_WIDTH / 2 - BALL_SIZE / 2; /* Center on paddle */
                        ball.y = paddle.y - BALL_SIZE; /* Position above paddle */
                        ball.vx = 0.0f; /* Reset velocity */
                        ball.vy = 0.0f;
                    }
                }
            }
            else
            {
                /* Keep ball stuck to paddle, updating position to follow paddle */
                ball.x = paddle.x + PADDLE_WIDTH / 2 - BALL_SIZE / 2;
                ball.y = paddle.y - BALL_SIZE;
            }

            /* Check for win condition (all bricks destroyed) */
            int active_bricks = 0;
            for (int i = 0; i < brick_count; i++)
            {
                if (bricks[i].sprite.active)
                    active_bricks++; /* Count remaining bricks */
            }
            if (active_bricks == 0)
            {
                state = GameOver; /* End game (win condition) */
                paddle.active = 0; /* Hide paddle */
                ball.active = 0; /* Hide ball */
            }
            break;

        case GameOver:
            /* Show game over message with score, high score, and restart prompt */
            snprintf(textGameOver, sizeof(textGameOver), "Game Over! Score: %d", score);
            snprintf(textHighScore, sizeof(textHighScore), "High Score: %d", high_score);
            snprintf(textRestart, sizeof(textRestart), "Press R to restart");
            arcade_render_text_centered(textGameOver, WINDOW_HEIGHT / 2.7f, 0xFFFFFF);  /* Positioned higher for spacing */
            arcade_render_text_centered(textHighScore, WINDOW_HEIGHT / 2.2f, 0xFFFFFF); /* Even spacing for high score */
            arcade_render_text_centered(textRestart, WINDOW_HEIGHT / 1.7f, 0xFFFFFF);   /* Lower for restart prompt */

            /* Handle restart input */
            if (arcade_key_pressed_once(a_r) == 2)
            {
                arcade_clear_keys(); /* Clear input to prevent immediate actions in Playing state */

                /* Reset paddle */
                paddle.x = WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2; /* Center horizontally */
                paddle.y = WINDOW_HEIGHT - 50.0f;               /* Near bottom */
                paddle.vx = 0.0f;
                paddle.vy = 0.0f;
                paddle.active = 1; /* Re-enable paddle */

                /* Reset ball */
                ball.x = paddle.x + PADDLE_WIDTH / 2 - BALL_SIZE / 2; /* Center on paddle */
                ball.y = paddle.y - BALL_SIZE;                        /* Above paddle */
                ball.vx = 0.0f;
                ball.vy = 0.0f;
                ball.active = 1; /* Re-enable ball */
                ball_stuck = 1;  /* Ball starts stuck to paddle */

                /* Reset bricks */
                brick_count = 0;
                for (int row = 0; row < 5; row++)
                {
                    for (int col = 0; col < 10; col++)
                    {
                        bricks[brick_count].sprite.x = col * (BRICK_WIDTH + 4.0f) + 20.0f; /* Rebuild grid */
                        bricks[brick_count].sprite.y = row * (BRICK_HEIGHT + 4.0f) + 50.0f;
                        bricks[brick_count].sprite.width = BRICK_WIDTH;
                        bricks[brick_count].sprite.height = BRICK_HEIGHT;
                        bricks[brick_count].sprite.vx = 0.0f;
                        bricks[brick_count].sprite.vy = 0.0f;
                        bricks[brick_count].sprite.color = row_colors[row]; /* Reassign row colors */
                        bricks[brick_count].sprite.active = 1; /* Reactivate all bricks */
                        brick_count++;
                    }
                }

                /* Reset game state */
                score = 0;        /* Reset score (high_score persists) */
                lives = 3;        /* Reset to 3 lives */
                state = Playing;  /* Restart gameplay */
            }
            break;
        }

        /* Sleep for ~16ms to target 60 FPS (optional) */
        arcade_sleep(16);
    }

    /* Clean up resources before exit */
    arcade_free_group(&group); /* Free sprite group memory */
    arcade_quit();            /* Close window and release Arcade resources */

    /* Print final score and high score to console */
    printf("Game Over! Final Score: %d, High Score: %d\n", score, high_score);
    return 0;
}