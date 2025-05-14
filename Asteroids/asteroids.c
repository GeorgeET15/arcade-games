/* =========================================================================
 * Asteroids Recreation - Documentation
 * =========================================================================
 * Author: GeorgeET15
 * Description:
 * A simplified Asteroids-inspired game built using the Arcade Library. The
 * player controls a red square (ship) that moves left/right and shoots yellow
 * bullets to destroy gray asteroids falling from the top of the screen. The
 * goal is to score points by destroying asteroids while avoiding collisions.
 * The game features three states (Start, Playing, GameOver), a high score
 * system, and increasing difficulty (asteroid speed). All asteroids are
 * removed from the screen in GameOver state. It uses color-based sprites and
 * is cross-platform (Windows with Win32, Linux with X11).
 *
 * Controls:
 * - Left Arrow: Move ship left (Playing state)
 * - Right Arrow: Move ship right (Playing state)
 * - Space: Shoot bullet (Playing state, one bullet at a time), Start game
 *         (Start state)
 * - R: Restart game (GameOver state)
 * - ESC: Quit (closes the window)
 *
 * Compilation:
 * Linux:
 *   gcc -D_POSIX_C_SOURCE=199309L -o asteroids asteroids.c arcade.c -lX11 -lm
 * Windows (MinGW):
 *   gcc -o asteroids asteroids.c arcade.c -lgdi32 -lwinmm
 * Run:
 *   Linux: ./asteroids
 *   Windows: asteroids.exe
 *
 * Dependencies:
 * - Arcade Library (arcade.h, arcade.c)
 * - STB libraries (included via arcade.c, though not used here since no
 *   image sprites)
 * - Linux: libX11, libm
 * - Windows: gdi32, winmm
 *
 * Notes:
 * - Uses color-based sprites (`ArcadeSprite`) for simplicity; image sprites
 *   could be added for better visuals.
 * - Asteroid spawn rate (2% per frame) and speed increase (0.1 per asteroid
 *   destroyed, capped at 5.0) balance difficulty.
 * - High score persists in memory during a session but resets on exit.
 * - Uses arcade_delta_time for frame-rate-independent movement, scaled to
 *   60 FPS.
 * - No audio effects; consider adding sounds for shooting, collisions, etc.
 * - In GameOver state, all asteroids are deactivated to clear the screen.
 * ========================================================================= */

#define ARCADE_IMPLEMENTATION
#include "arcade.h"

/* =========================================================================
 * Game Constants
 * =========================================================================
 * Define core game parameters, controlling the play area and object limits.
 * Adjust these to tweak game size or difficulty.
 */
#define MAX_ASTEROIDS 5   /* Maximum number of active asteroids. Balances performance and challenge. */
#define WINDOW_WIDTH 400  /* Window width (pixels). Narrow for focused gameplay. */
#define WINDOW_HEIGHT 800 /* Window height (pixels). Tall to allow reaction time for falling asteroids. */

/* =========================================================================
 * GameState Enum
 * =========================================================================
 * Defines the possible game states, used to manage different modes of
 * operation (e.g., rendering UI, handling inputs).
 * - Start: Initial screen with instructions.
 * - Playing: Active gameplay with ship movement, shooting, and asteroids.
 * - GameOver: End state with score and restart prompt, no asteroids shown.
 */
typedef enum
{
    Start,   /* Start screen, waiting for Space to begin */
    Playing, /* Active gameplay, ship and asteroids move */
    GameOver /* Game over, shows score and restart option */
} GameState;

/* =========================================================================
 * Asteroid Structure
 * =========================================================================
 * Represents a single asteroid with its sprite properties.
 * - sprite: ArcadeSprite for position, size, velocity, color, and active state.
 * Note: Uses color-based sprites (gray); could use `ArcadeImageSprite` for
 * textured asteroids.
 */
typedef struct
{
    ArcadeSprite sprite; /* Asteroid’s sprite (position, velocity, color) */
} Asteroid;

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
 *       arcade_init(WINDOW_WIDTH, WINDOW_HEIGHT, "ARCADE: Asteroids", 0x000000);
 *       while (arcade_running() && arcade_update()) {
 *           float dt = arcade_delta_time();
 *           // Update and render game
 *       }
 *       arcade_quit();
 *       return 0;
 *   }
 * Notes:
 * - Initializes random seed for asteroid spawning.
 * - Uses arcade_sleep(16) for approximate 60 FPS; consider removing for full
 *   frame-rate independence.
 * - Cleans up sprite group and Arcade resources on exit.
 * - Prints final score and high score to console on exit.
 */
int main(void)
{
    /* Seed random number generator for asteroid spawning and positioning */
    srand(time(NULL));

    /* Game parameters */
    float player_speed = 5.0f;       /* Ship’s horizontal speed (pixels/frame at 60 FPS) */
    float bullet_speed = 30.0f;      /* Bullet’s upward speed (pixels/frame at 60 FPS, negative = up) */
    float asteroid_speed = 2.0f;     /* Initial asteroid downward speed (pixels/frame at 60 FPS) */
    float asteroid_speed_max = 5.0f; /* Maximum asteroid speed for difficulty cap */
    float asteroid_speed_inc = 0.1f; /* Speed increase per asteroid destroyed */
    int score = 0;                   /* Current player score (asteroids destroyed) */
    int high_score = 0;              /* Highest score in session, persists across restarts */
    char text[64];                   /* Buffer for rendering score and messages */
    char textGameOver[64];           /* Buffer for game over message */
    char textHighScore[64];          /* Buffer for high score message */
    char textRestart[64];            /* Buffer for restart prompt */
    GameState state = Start;         /* Start in Start state (shows instructions) */

    /* Initialize player sprite (red square, starts near bottom-center) */
    ArcadeSprite player = {
        .x = WINDOW_WIDTH / 2 - 10.0f, /* Center horizontally */
        .y = WINDOW_HEIGHT - 50.0f,    /* Near bottom of screen */
        .width = 20.0f,
        .height = 20.0f, /* 20x20 pixel square */
        .vy = 0.0f,
        .vx = 0.0f,        /* No initial velocity */
        .color = 0xFF0000, /* Red */
        .active = 1        /* Visible and collidable */
    };

    /* Initialize bullet sprite (yellow square, inactive until shot) */
    ArcadeSprite bullet = {
        .x = player.x + 10.0f, /* Aligned with player’s center (updated on shoot) */
        .y = player.y,         /* Starts at player’s top */
        .width = 5.0f,
        .height = 5.0f, /* 5x5 pixel square */
        .vy = 0.0f,
        .vx = 0.0f,        /* No initial velocity */
        .color = 0xFFFF00, /* Yellow */
        .active = 0        /* Inactive until Space is pressed */
    };

    /* Initialize asteroids array (gray squares, initially inactive) */
    Asteroid asteroids[MAX_ASTEROIDS];
    for (int i = 0; i < MAX_ASTEROIDS; i++)
    {
        asteroids[i].sprite.x = rand() % (WINDOW_WIDTH - 30) + 15;            /* Random x within bounds */
        asteroids[i].sprite.y = rand() % (WINDOW_HEIGHT / 2) - WINDOW_HEIGHT; /* Off-screen above */
        asteroids[i].sprite.width = 30.0f;                                    /* 30x30 pixel square */
        asteroids[i].sprite.height = 30.0f;
        asteroids[i].sprite.vy = asteroid_speed; /* Initial downward speed */
        asteroids[i].sprite.vx = 0.0f;           /* No horizontal movement */
        asteroids[i].sprite.color = 0x808080;    /* Gray */
        asteroids[i].sprite.active = 0;          /* Inactive until spawned */
    }

    /* Initialize sprite group for rendering */
    SpriteGroup group;
    arcade_init_group(&group, MAX_ASTEROIDS + 2); /* Capacity for player, bullet, asteroids */

    /* Initialize Arcade environment (window, rendering, input) */
    if (arcade_init(WINDOW_WIDTH, WINDOW_HEIGHT, "ARCADE: Asteroids", 0x000000) != 0)
    {
        arcade_free_group(&group);
        fprintf(stderr, "Initialization failed\n");
        return 1; /* Exit if window creation fails */
    }

    /* Main game loop: runs until window is closed or ESC is pressed */
    while (arcade_running() && arcade_update())
    {
        /* Get delta time for frame-rate-independent movement */
        float delta_time = arcade_delta_time();
        float scale = delta_time * 60.0f; /* Normalize to 60 FPS */

        /* Update score display (rendered every frame) */
        snprintf(text, sizeof(text), "Score: %d", score);

        /* Reset sprite group to rebuild with active sprites */
        group.count = 0;

        /* Add active sprites to render group (player, bullet, asteroids) */
        if (player.active)
        {
            arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.sprite = player}, SPRITE_COLOR);
        }
        if (bullet.active)
        {
            arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.sprite = bullet}, SPRITE_COLOR);
        }
        for (int i = 0; i < MAX_ASTEROIDS; i++)
        {
            if (asteroids[i].sprite.active)
            {
                arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.sprite = asteroids[i].sprite}, SPRITE_COLOR);
            }
        }

        /* Render the scene (clears screen, draws sprites, updates window) */
        arcade_render_group(&group);
        arcade_render_text(text, 10.0f, 30.0f, 0xFFFFFF); /* Score in top-left (white) */

        /* Handle game logic based on current state */
        switch (state)
        {
        case Start:
            /* Show blinking start prompt and high score */
            arcade_render_text_centered_blink("Press Space to Start", WINDOW_HEIGHT / 2.0f, 0xFFFFFF, 30); /* Blinks every 0.5s */
            snprintf(text, sizeof(text), "High Score: %d", high_score);
            arcade_render_text_centered(text, WINDOW_HEIGHT / 2.0f + 50.0f, 0xFFFFFF); /* High score below prompt */
            if (arcade_key_pressed_once(a_space) == 2)
            {
                arcade_clear_keys(); /* Clear input to prevent immediate shoot */
                state = Playing;     /* Transition to gameplay */
            }
            break;

        case Playing:
            /* Handle player movement (left/right arrow keys) */
            if (arcade_key_pressed(a_right) == 2 && player.active)
            {
                player.vx = player_speed; /* Set rightward velocity */
            }
            else if (arcade_key_pressed(a_left) == 2 && player.active)
            {
                player.vx = -player_speed; /* Set leftward velocity */
            }
            else
            {
                player.vx = 0.0f; /* Stop movement */
            }

            /* Update player position and clamp to window bounds */
            if (player.active)
            {
                player.x += player.vx * scale; /* Scale movement by delta time */
                if (player.x < 0)
                {
                    player.x = 0; /* Prevent moving off left edge */
                }
                else if (player.x + player.width > WINDOW_WIDTH)
                {
                    player.x = WINDOW_WIDTH - player.width; /* Prevent moving off right edge */
                }
            }

            /* Handle shooting (Space key, one bullet at a time) */
            if (arcade_key_pressed_once(a_space) == 2 && player.active && !bullet.active)
            {
                bullet.x = player.x + player.width / 2 - bullet.width / 2; /* Center bullet on player */
                bullet.y = player.y;                                       /* Start at player’s top */
                bullet.vy = -bullet_speed;                                 /* Move upward */
                bullet.active = 1;                                         /* Activate bullet */
                /* Note: Could add shooting sound here (e.g., arcade_play_sound("shoot.wav")) */
            }

            /* Update bullet position */
            if (bullet.active)
            {
                bullet.y += bullet.vy * scale; /* Scale movement by delta time */
                if (bullet.y < 0)
                {
                    bullet.active = 0; /* Deactivate when off-screen */
                }
            }

            /* Update and spawn asteroids */
            for (int i = 0; i < MAX_ASTEROIDS; i++)
            {
                if (!asteroids[i].sprite.active && rand() % 100 < 2)
                {                                                              /* 2% spawn chance per frame */
                    asteroids[i].sprite.x = rand() % (WINDOW_WIDTH - 30) + 15; /* Random x within bounds */
                    asteroids[i].sprite.y = -30.0f;                            /* Start above screen */
                    asteroids[i].sprite.vy = asteroid_speed;                   /* Current downward speed */
                    asteroids[i].sprite.active = 1;                            /* Activate asteroid */
                }

                if (asteroids[i].sprite.active)
                {
                    asteroids[i].sprite.y += asteroids[i].sprite.vy * scale; /* Scale movement by delta time */
                    if (asteroids[i].sprite.y > WINDOW_HEIGHT)
                    {
                        asteroids[i].sprite.active = 0; /* Deactivate when off-screen */
                    }
                }
            }

            /* Collision detection: Bullet vs. Asteroids */
            if (bullet.active)
            {
                for (int i = 0; i < MAX_ASTEROIDS; i++)
                {
                    if (arcade_check_collision(&bullet, &asteroids[i].sprite))
                    {
                        asteroids[i].sprite.active = 0; /* Destroy asteroid */
                        bullet.active = 0;              /* Destroy bullet */
                        score++;                        /* Increment score */
                        if (score > high_score)
                            high_score = score;               /* Update high score */
                        asteroid_speed += asteroid_speed_inc; /* Increase difficulty */
                        if (asteroid_speed > asteroid_speed_max)
                        {
                            asteroid_speed = asteroid_speed_max; /* Cap speed */
                        }
                        /* Note: Could add explosion sound here (e.g., arcade_play_sound("explode.wav")) */
                        break; /* Stop checking after first hit */
                    }
                }
            }

            /* Collision detection: Player vs. Asteroids */
            for (int i = 0; i < MAX_ASTEROIDS; i++)
            {
                if (arcade_check_collision(&player, &asteroids[i].sprite))
                {
                    player.active = 0; /* Disable player */
                    state = GameOver;  /* End game */
                    /* Note: Could add crash sound here (e.g., arcade_play_sound("crash.wav")) */
                    break; /* Stop checking after first hit */
                }
            }
            break;

        case GameOver:
            /* Deactivate all asteroids to clear the screen */
            for (int i = 0; i < MAX_ASTEROIDS; i++)
            {
                asteroids[i].sprite.active = 0; /* Remove asteroid from rendering and updates */
            }

            /* Show game over message with current score, high score, and restart prompt */
            snprintf(textGameOver, sizeof(textGameOver), "Game Over! Score: %d", score);
            snprintf(textHighScore, sizeof(textHighScore), "High Score: %d", high_score);
            snprintf(textRestart, sizeof(textRestart), "Press R to restart");
            arcade_render_text_centered(textGameOver, WINDOW_HEIGHT / 2.7f, 0xFFFFFF);  /* Slightly higher for spacing */
            arcade_render_text_centered(textHighScore, WINDOW_HEIGHT / 2.2f, 0xFFFFFF); /* Adjusted for even spacing */
            arcade_render_text_centered(textRestart, WINDOW_HEIGHT / 1.7f, 0xFFFFFF);   /* Lower for better separation */

            /* Handle restart input */
            if (arcade_key_pressed_once(a_r) == 2)
            {
                arcade_clear_keys(); /* Clear input to prevent immediate actions */

                /* Reset player */
                player.x = WINDOW_WIDTH / 2 - 10.0f; /* Center horizontally */
                player.y = WINDOW_HEIGHT - 50.0f;    /* Near bottom */
                player.vx = 0.0f;
                player.vy = 0.0f;
                player.active = 1; /* Re-enable player */

                /* Reset bullet */
                bullet.active = 0;
                bullet.vy = 0.0f;

                /* Reset asteroids */
                for (int i = 0; i < MAX_ASTEROIDS; i++)
                {
                    asteroids[i].sprite.x = rand() % (WINDOW_WIDTH - 30) + 15;
                    asteroids[i].sprite.y = rand() % (WINDOW_HEIGHT / 2) - WINDOW_HEIGHT; /* Off-screen above */
                    asteroids[i].sprite.vy = asteroid_speed;
                    asteroids[i].sprite.active = 0; /* Inactive until spawned */
                }

                /* Reset game state */
                score = 0;             /* Reset score (high_score persists) */
                asteroid_speed = 2.0f; /* Reset difficulty */
                state = Playing;       /* Restart gameplay */
            }
            break;
        }

        /* Sleep for ~16ms to target 60 FPS (optional) */
        arcade_sleep(16);
    }

    /* Clean up resources before exit */
    arcade_free_group(&group); /* Free sprite group */
    arcade_quit();             /* Close window and release Arcade resources */

    /* Print final score and high score to console */
    printf("Game Over! Final Score: %d, High Score: %d\n", score, high_score);
    return 0;
}