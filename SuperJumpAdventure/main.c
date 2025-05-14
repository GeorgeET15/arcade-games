/* =========================================================================
 * Super Jump Adventure - 2D Platformer
 * =========================================================================
 * Author: GeorgeET15
 * Description:
 * A 2D platformer using Arcade Library. Control an animated character to jump,
 * shoot, and reach a flag while avoiding enemies. Features Start, Playing, Won,
 * Lost states, best-time in memory, and smooth collision handling.
 * Cross-platform: Windows (Win32), Linux (X11).
 * Updates:
 * - Added intermediate platform.
 * - Timer at bottom-left.
 * - Precomputed flipped sprites for enemies.
 * - Improved collision detection.
 * - 3-frame enemy animation (enemy-run-1.png to enemy-run-3.png).
 * - Jump sprite (player-run-2.png).
 * - Best time in variable, restart with 'R'.
 * - Fixed jittering with smooth collisions.
 * - Optimized code for brevity.
 * - Fixed shooting with simplified cooldown.
 * - Fixed bullet rendering by switching to SPRITE_IMAGE.
 *
 * Category: Platformer/Arcade
 * Controls:
 * - Left/Right Arrow: Move (Playing)
 * - Up Arrow: Jump (Playing, double-jump)
 * - Space: Shoot (Playing, cooldown)
 * - ESC: Quit
 * - Space (Start): Start game
 * - R (Won/Lost): Restart
 *
 * Compilation:
 * Linux: gcc -D_POSIX_C_SOURCE=199309L -o superjump super_jump_adventure.c arcade.c -lX11 -lm
 * Windows (MinGW): gcc -o superjump super_jump_adventure.c arcade.c -lgdi32 -lwinmm
 * Run: Linux (./superjump), Windows (superjump.exe)
 * Sprites in ./assets/sprites/: background.png, player-run-1.png to player-run-4.png,
 * player-idle.png, platform.png, enemy-run-1.png to enemy-run-3.png, flag.png, bullet.png
 *
 * Dependencies: Arcade Library, STB (via arcade.c), Linux (libX11, libm), Windows (gdi32, winmm)
 * Notes:
 * - Uses ArcadeImageSprite for platforms/flag/bullets, ArcadeAnimatedSprite for player/enemies.
 * - Best time persists in session.
 * - Frame-rate-independent movement with arcade_delta_time.
 * - Coyote time, double-jump for better platforming.
 * - Bullets now use SPRITE_IMAGE to avoid rendering issues.
 * ========================================================================= */

/* Include the Arcade Library implementation */
#define ARCADE_IMPLEMENTATION
#include "arcade.h"
#include <stdlib.h>  /* For memory management (malloc, free) and random number generation (rand) */
#include <time.h>    /* For seeding the random number generator with the current time */
#include <string.h>  /* For string operations like strdup and strncpy */
#include <stdio.h>   /* For debug output using printf */

/* Game Constants - Define core game parameters */
#define WINDOW_WIDTH 800        /* Width of the game window in pixels */
#define WINDOW_HEIGHT 600       /* Height of the game window in pixels */
#define PLAYER_SIZE 40.0f       /* Width and height of the player sprite in pixels */
#define BG_COLOR 0x7092BE       /* Background color in ARGB format (light blue) */
#define PLAYER_SPEED 4.0f       /* Player horizontal movement speed (pixels per frame at 60 FPS) */
#define BULLET_SIZE 10.0f       /* Width and height of the bullet sprite in pixels */
#define BULLET_SPEED 10.0f      /* Bullet horizontal speed (pixels per frame at 60 FPS) */
#define MAX_BULLETS 10          /* Maximum number of bullets that can be active at once */
#define BULLET_COOLDOWN 10      /* Frames between bullet shots (~0.17 seconds at 60 FPS) */
#define GRAVITY 0.5f            /* Gravity acceleration (pixels per frame^2 at 60 FPS) */
#define JUMP_VELOCITY -10.0f    /* Initial upward velocity when jumping (pixels per frame at 60 FPS) */
#define COYOTE_FRAMES 6         /* Frames for "coyote time" (grace period to jump after leaving a platform) */
#define MAX_JUMPS 2             /* Maximum number of jumps allowed (double jump) */
#define ENEMY_SPEED 2.0f        /* Enemy horizontal movement speed (pixels per frame at 60 FPS) */
#define OVERLAY_COLOR 0x00000080 /* Overlay color for UI screens (semi-transparent black in ARGB) */

/* Game States - Enum to track the current state of the game */
typedef enum { Start, Playing, Won, Lost } GameState;

/* Helper Function to Free Flipped Sprites
 * Frees dynamically allocated memory for flipped sprite paths to prevent memory leaks.
 * Parameters:
 * - flipped_run: Array of flipped player run sprite paths
 * - flipped_idle: Flipped player idle sprite path
 * - flipped_jump: Flipped player jump sprite path
 * - flipped_enemy: Array of flipped enemy sprite paths
 * - flipped_flag: Flipped flag sprite path
 * - run_count: Number of player run frames
 * - enemy_count: Number of enemy animation frames
 */
void free_flipped_sprites(char **flipped_run, char *flipped_idle, char *flipped_jump, char **flipped_enemy, char *flipped_flag, int run_count, int enemy_count) {
    for (int i = 0; i < run_count; i++) if (flipped_run[i]) free(flipped_run[i]);      /* Free each flipped player run sprite path */
    for (int i = 0; i < enemy_count; i++) if (flipped_enemy[i]) free(flipped_enemy[i]); /* Free each flipped enemy sprite path */
    if (flipped_idle) free(flipped_idle);                                               /* Free flipped idle sprite path */
    if (flipped_jump) free(flipped_jump);                                               /* Free flipped jump sprite path */
    if (flipped_flag) free(flipped_flag);                                               /* Free flipped flag sprite path */
}

int main(void) {
    /* Seed the random number generator with the current time for random enemy behavior */
    srand(time(NULL));

    /* Sprite Paths - Define file paths for all sprite assets */
    const char *run_frames[] = {
        "./assets/sprites/player-run-1.png", "./assets/sprites/player-run-2.png",
        "./assets/sprites/player-run-1.png", "./assets/sprites/player-idle.png",
        "./assets/sprites/player-run-3.png", "./assets/sprites/player-run-4.png",
        "./assets/sprites/player-run-3.png", "./assets/sprites/player-idle.png"
    }; /* Array of player running animation frames (8 frames for smooth animation) */
    const char *idle_sprite = "./assets/sprites/player-idle.png"; /* Player idle sprite path */
    const char *jump_sprite = "./assets/sprites/player-run-2.png"; /* Player jump sprite path (reuses run frame 2) */
    const char *platform_sprite = "./assets/sprites/platform.png"; /* Platform sprite path */
    const char *enemy_frames[] = {
        "./assets/sprites/enemy-run-1.png", "./assets/sprites/enemy-run-2.png",
        "./assets/sprites/enemy-run-3.png"
    }; /* Array of enemy running animation frames (3 frames) */
    const char *flag_sprite = "./assets/sprites/flag.png"; /* Flag sprite path (win condition) */
    const char *bullet_sprite = "./assets/sprites/bullet.png"; /* Bullet sprite path (small red square) */

    /* Flipped Sprites - Precompute flipped versions of sprites for left-facing movement */
    char *flipped_run[8] = {0};    /* Array to store flipped player run sprite paths */
    char *flipped_idle = NULL;     /* Flipped player idle sprite path */
    char *flipped_jump = NULL;     /* Flipped player jump sprite path */
    char *flipped_flag = NULL;     /* Flipped flag sprite path */
    char *flipped_enemy[3] = {0};  /* Array to store flipped enemy sprite paths */
    char flipped_paths[14][256] = {0}; /* Temporary storage for flipped sprite paths (8 run + idle + jump + 3 enemy + flag) */
    /* Flip player run sprites for left-facing movement */
    for (int i = 0; i < 8; i++) {
        if (!(flipped_run[i] = strdup(arcade_flip_image(run_frames[i], 0)))) goto cleanup; /* Flip each run frame and store path */
        strncpy(flipped_paths[i], flipped_run[i], 255); /* Copy flipped path to temporary storage */
    }
    /* Flip player idle sprite for left-facing movement */
    if (!(flipped_idle = strdup(arcade_flip_image(idle_sprite, 0)))) goto cleanup;
    strncpy(flipped_paths[8], flipped_idle, 255);
    /* Flip player jump sprite for left-facing movement */
    if (!(flipped_jump = strdup(arcade_flip_image(jump_sprite, 0)))) goto cleanup;
    strncpy(flipped_paths[9], flipped_jump, 255);
    /* Flip enemy sprites for left-facing movement */
    for (int i = 0; i < 3; i++) {
        if (!(flipped_enemy[i] = strdup(arcade_flip_image(enemy_frames[i], 0)))) goto cleanup;
        strncpy(flipped_paths[i + 10], flipped_enemy[i], 255);
    }
    /* Flip flag sprite (though not used in gameplay, precomputed for consistency) */
    if (!(flipped_flag = strdup(arcade_flip_image(flag_sprite, 0)))) goto cleanup;
    strncpy(flipped_paths[13], flipped_flag, 255);

    /* Initialize Sprites - Create sprite objects for rendering */
    /* Player running animations (right and left facing) */
    ArcadeAnimatedSprite run_right = arcade_create_animated_sprite(70.0f, WINDOW_HEIGHT - PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE, run_frames, 8, 4);
    ArcadeAnimatedSprite run_left = arcade_create_animated_sprite(70.0f, WINDOW_HEIGHT - PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE, (const char **)flipped_run, 8, 4);
    /* Player idle sprites (right and left facing) */
    ArcadeImageSprite idle_right = arcade_create_image_sprite(70.0f, WINDOW_HEIGHT - PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE, idle_sprite);
    ArcadeImageSprite idle_left = arcade_create_image_sprite(70.0f, WINDOW_HEIGHT - PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE, flipped_idle);
    /* Player jump sprites (right and left facing) */
    ArcadeImageSprite jump_right = arcade_create_image_sprite(70.0f, WINDOW_HEIGHT - PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE, jump_sprite);
    ArcadeImageSprite jump_left = arcade_create_image_sprite(70.0f, WINDOW_HEIGHT - PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE, flipped_jump);
    /* Background sprite covering the entire window */
    ArcadeImageSprite background = arcade_create_image_sprite(0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, "./assets/sprites/background.png");

    /* Platforms - Create 8 platforms at fixed positions for the player to navigate */
    ArcadeImageSprite platforms[8]; /* Array to store platform sprites */
    float platform_x[] = {0.0f, 300.0f, 450.0f, 200.0f, 100.0f, 350.0f, 600.0f, 700.0f}; /* X positions of platforms */
    float platform_y[] = {500.0f, 400.0f, 300.0f, 250.0f, 150.0f, 150.0f, 150.0f, 100.0f}; /* Y positions of platforms */
    float platform_w[] = {200.0f, 100.0f, 80.0f, 150.0f, 100.0f, 100.0f, 80.0f, 100.0f}; /* Widths of platforms */
    for (int i = 0; i < 8; i++) {
        platforms[i] = arcade_create_image_sprite(platform_x[i], platform_y[i], platform_w[i], 20.0f, platform_sprite); /* Create each platform sprite with specified position and size */
    }

    /* Enemies - Create 2 enemies that patrol platforms */
    ArcadeAnimatedSprite enemies_right[2], enemies_left[2]; /* Arrays for right and left-facing enemy animations */
    float enemy_x[] = {250.0f, 600.0f}; /* Initial X positions of enemies */
    float enemy_y[] = {210.0f, 110.0f}; /* Initial Y positions of enemies (aligned with platforms) */
    float enemy_vx[] = {ENEMY_SPEED, -ENEMY_SPEED}; /* Initial velocities (first enemy moves right, second moves left) */
    int enemy_facing_right[] = {1, 0}; /* Facing direction for each enemy (1 = right, 0 = left) */
    int enemy_active[] = {1, 1}; /* Active state for each enemy (1 = active, 0 = defeated) */
    for (int i = 0; i < 2; i++) {
        enemies_right[i] = arcade_create_animated_sprite(enemy_x[i], enemy_y[i], PLAYER_SIZE, PLAYER_SIZE, enemy_frames, 3, 10); /* Right-facing enemy animation */
        enemies_left[i] = arcade_create_animated_sprite(enemy_x[i], enemy_y[i], PLAYER_SIZE, PLAYER_SIZE, (const char **)flipped_enemy, 3, 10); /* Left-facing enemy animation */
    }

    /* Flag and Bullets - Create the win condition flag and bullet sprites */
    ArcadeImageSprite flag = arcade_create_image_sprite(740.0f, 40.0f, 60.0f, 70.0f, flag_sprite); /* Flag sprite at the end of the level (larger than player for visibility) */
    ArcadeImageSprite bullets[MAX_BULLETS]; /* Array to store bullet sprites */
    float bullet_vx[MAX_BULLETS] = {0.0f}; /* Array to store horizontal velocity for each bullet */
    int bullet_active[MAX_BULLETS] = {0}; /* Array to track active state for each bullet (1 = active, 0 = inactive) */
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i] = arcade_create_image_sprite(0.0f, 0.0f, BULLET_SIZE, BULLET_SIZE, bullet_sprite); /* Initialize each bullet sprite (position will be set when fired) */
    }

    /* Validate Sprites - Ensure all sprite assets loaded correctly */
    if (!run_right.frames || !run_left.frames || !idle_right.pixels || !idle_left.pixels || 
        !jump_right.pixels || !jump_left.pixels || !background.pixels || !platforms[0].pixels || 
        !enemies_right[0].frames || !flag.pixels || !bullets[0].pixels) goto cleanup; /* If any sprite fails to load, jump to cleanup to free resources and exit */

    /* Initialize Groups and Overlay - Set up rendering group and UI overlay */
    SpriteGroup group; /* Rendering group to hold all sprites to be drawn each frame */
    arcade_init_group(&group, 13 + MAX_BULLETS); /* Initialize group with capacity for background (1), platforms (8), enemies (2), flag (1), player (1), and bullets (MAX_BULLETS) */
    ArcadeSprite overlay = {0.0f, 0.0f, 0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, OVERLAY_COLOR, 1}; /* Overlay sprite for dimming the screen during Start/Won/Lost states */
    float best_time = 9999.9f; /* Variable to track the best completion time (in seconds) across all attempts in the session */

    /* Initialize the Arcade Library window */
    if (arcade_init(WINDOW_WIDTH, WINDOW_HEIGHT, "Super Jump Adventure", BG_COLOR)) goto cleanup; /* Initialize window; if it fails, jump to cleanup */

    /* Game Variables - Define variables for game state and mechanics */
    float x = 70.0f, y = WINDOW_HEIGHT - PLAYER_SIZE; /* Player position (starting at bottom-left) */
    float vx = 0.0f, vy = 0.0f; /* Player velocity (horizontal and vertical) */
    int moving = 0; /* Flag to track if the player is moving (1 = moving, 0 = idle) */
    int facing_right = 1; /* Player facing direction (1 = right, 0 = left) */
    int on_ground = 0; /* Flag to track if the player is on the ground (1 = on ground, 0 = in air) */
    int jump_count = 0; /* Number of jumps performed (resets when on ground) */
    int coyote_frames = 0; /* Frames remaining for coyote time (allows jumping shortly after leaving a platform) */
    int shot_cooldown = 0; /* Frames remaining until the player can shoot again */
    unsigned long game_frame = 0; /* Total frames since the game started */
    unsigned long start_frame = 0; /* Frame when the current game started (for timing) */
    int deaths = 0; /* Number of deaths in the current game */
    float game_time = 0.0f; /* Time elapsed in the current game (in seconds) */
    GameState state = Start; /* Current game state (Start, Playing, Won, or Lost) */
    char text_buffer[256]; /* Buffer for rendering UI text (e.g., time, deaths) */

    /* Main Game Loop - Handle input, update game state, and render each frame */
    while (arcade_running() && arcade_update()) {
        /* Calculate frame time for frame-rate-independent movement */
        float delta_time = arcade_delta_time(); /* Time since last frame in seconds */
        if (delta_time > 0.1f) delta_time = 0.1f; /* Cap delta_time to prevent large jumps on lag */
        float scale = delta_time * 60.0f; /* Scale factor to normalize movement to 60 FPS */
        if (scale > 2.0f) scale = 2.0f; /* Cap scale to prevent extreme movement on lag */
        game_frame++; /* Increment frame counter */

        /* State: Start - Display start screen and wait for player to begin */
        if (state == Start) {
            if (arcade_key_pressed_once(a_space) == 2) { /* Check for Space key press to start the game */
                state = Playing; /* Transition to Playing state */
                start_frame = game_frame; /* Record the starting frame for timing */
                /* Reset player position and state */
                x = 70.0f; y = WINDOW_HEIGHT - PLAYER_SIZE;
                vx = vy = 0.0f;
                jump_count = deaths = 0;
                coyote_frames = 0;
                shot_cooldown = 0;
                game_time = 0.0f;
                /* Reset enemies */
                for (int i = 0; i < 2; i++) {
                    enemy_active[i] = 1; /* Reactivate enemies */
                    enemies_right[i].current_frame = enemies_right[i].frame_counter = 0; /* Reset animation */
                    enemies_left[i].current_frame = enemies_left[i].frame_counter = 0;
                    /* Reset enemy positions */
                    for (int j = 0; j < 3; j++) {
                        enemies_right[i].frames[j].x = enemies_left[i].frames[j].x = enemy_x[i];
                        enemies_right[i].frames[j].y = enemies_left[i].frames[j].y = enemy_y[i];
                    }
                    enemy_vx[i] = (i == 0) ? ENEMY_SPEED : -ENEMY_SPEED; /* Reset enemy velocities */
                    enemy_facing_right[i] = (enemy_vx[i] > 0); /* Set facing direction based on velocity */
                }
            }
            if (arcade_key_pressed_once(a_esc) == 2) arcade_set_running(0); /* Exit game on ESC */
        }
        /* State: Playing - Main gameplay loop */
        else if (state == Playing) {
            /* Input Handling - Process player input for movement, jumping, and shooting */
            vx = 0.0f; moving = 0; /* Reset velocity and moving state */
            if (arcade_key_pressed(a_left) == 2) { /* Left arrow: Move left */
                vx = -PLAYER_SPEED; moving = 1; facing_right = 0;
            }
            if (arcade_key_pressed(a_right) == 2) { /* Right arrow: Move right */
                vx = PLAYER_SPEED; moving = 1; facing_right = 1;
            }
            if (arcade_key_pressed_once(a_space) == 2) { /* Space key: Shoot a bullet */
                if (shot_cooldown <= 0) { /* Check if shooting is off cooldown */
                    for (int i = 0; i < MAX_BULLETS; i++) { /* Find an inactive bullet slot */
                        if (!bullet_active[i]) {
                            /* Spawn bullet at player's center */
                            bullets[i].x = x + PLAYER_SIZE / 2 - BULLET_SIZE / 2;
                            bullets[i].y = y + PLAYER_SIZE / 2 - BULLET_SIZE / 2;
                            bullet_vx[i] = facing_right ? BULLET_SPEED : -BULLET_SPEED; /* Set bullet direction */
                            bullet_active[i] = 1; /* Activate bullet */
                            shot_cooldown = BULLET_COOLDOWN; /* Start cooldown */
                            break;
                        }
                    }
                }
            }
            if (arcade_key_pressed_once(a_up) == 2 && (on_ground || coyote_frames > 0 || jump_count < MAX_JUMPS)) { /* Up arrow: Jump */
                vy = JUMP_VELOCITY; /* Apply upward velocity */
                jump_count++; /* Increment jump count */
                on_ground = 0; /* Player is no longer on ground */
                coyote_frames = 0; /* Disable coyote time */
            }
            if (arcade_key_pressed_once(a_esc) == 2) arcade_set_running(0); /* Exit game on ESC */

            /* Decrement Cooldown - Reduce shooting cooldown each frame */
            if (shot_cooldown > 0) shot_cooldown--;

            /* Physics and Collision - Update player position and handle collisions */
            vy += GRAVITY * scale; /* Apply gravity to vertical velocity */
            float new_x = x + vx * scale; /* Calculate new X position */
            float new_y = y + vy * scale; /* Calculate new Y position */
            on_ground = 0; /* Reset on_ground flag (will be set if collision occurs) */

            /* Check collisions with platforms */
            for (int i = 0; i < 8; i++) {
                float pl = platforms[i].x; /* Platform left edge */
                float pr = pl + platform_w[i]; /* Platform right edge */
                float pt = platforms[i].y; /* Platform top edge */
                float pb = pt + 20.0f; /* Platform bottom edge */
                /* Check if player intersects with platform */
                if (new_x + PLAYER_SIZE > pl && new_x < pr && new_y + PLAYER_SIZE > pt && new_y < pb) {
                    /* Landing on platform (falling down) */
                    if (vy > 0 && y + PLAYER_SIZE <= pt + 1.0f) {
                        new_y = pt - PLAYER_SIZE; /* Snap player to platform top */
                        vy = 0.0f; /* Stop vertical movement */
                        on_ground = 1; /* Player is on ground */
                        jump_count = 0; /* Reset jump count */
                        coyote_frames = COYOTE_FRAMES; /* Enable coyote time */
                    }
                    /* Hitting platform from below (jumping up) */
                    else if (vy < 0 && y >= pb - 1.0f) {
                        new_y = pb; /* Snap player to platform bottom */
                        vy = 0.0f; /* Stop vertical movement */
                    }
                    /* Hitting platform from the left (moving right) */
                    else if (vx > 0 && x + PLAYER_SIZE <= pl + 1.0f) {
                        new_x = pl - PLAYER_SIZE; /* Snap player to platform left edge */
                        vx = 0.0f; /* Stop horizontal movement */
                    }
                    /* Hitting platform from the right (moving left) */
                    else if (vx < 0 && x >= pr - 1.0f) {
                        new_x = pr; /* Snap player to platform right edge */
                        vx = 0.0f; /* Stop horizontal movement */
                    }
                }
            }

            /* Update player position and apply screen bounds */
            x = new_x; y = new_y;
            if (x < 0) { x = 0; vx = 0.0f; } /* Prevent moving off left edge */
            if (x > WINDOW_WIDTH - PLAYER_SIZE) { x = WINDOW_WIDTH - PLAYER_SIZE; vx = 0.0f; } /* Prevent moving off right edge */
            if (y > WINDOW_HEIGHT - PLAYER_SIZE) { /* Landing on the ground */
                y = WINDOW_HEIGHT - PLAYER_SIZE;
                vy = 0.0f;
                on_ground = 1;
                jump_count = 0;
                coyote_frames = COYOTE_FRAMES;
            }
            if (y < 0) { y = 0; vy = 0.0f; } /* Prevent moving off top edge */
            if (coyote_frames > 0 && !on_ground) coyote_frames--; /* Decrement coyote time if in air */

            /* Update Bullets - Move bullets and check for collisions */
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bullet_active[i]) { /* Process only active bullets */
                    bullets[i].x += bullet_vx[i] * scale; /* Move bullet horizontally */
                    /* Deactivate bullet if it goes off-screen */
                    if (bullets[i].x < 0 || bullets[i].x > WINDOW_WIDTH) {
                        bullet_active[i] = 0;
                    }
                    /* Check for bullet-enemy collisions */
                    for (int j = 0; j < 2; j++) {
                        if (enemy_active[j] &&
                            bullets[i].x + BULLET_SIZE > enemies_right[j].frames[0].x &&
                            bullets[i].x < enemies_right[j].frames[0].x + PLAYER_SIZE &&
                            bullets[i].y + BULLET_SIZE > enemies_right[j].frames[0].y &&
                            bullets[i].y < enemies_right[j].frames[0].y + PLAYER_SIZE) {
                            enemy_active[j] = bullet_active[i] = 0; /* Deactivate both enemy and bullet on hit */
                            printf("Bullet %d hit enemy %d at x=%.1f, y=%.1f\n", i, j, bullets[i].x, bullets[i].y); /* Debug output */
                            break;
                        }
                    }
                }
            }

            /* Update Enemies - Move enemies and check for collisions with player */
            for (int i = 0; i < 2; i++) {
                if (enemy_active[i]) { /* Process only active enemies */
                    ArcadeAnimatedSprite *enemy = enemy_facing_right[i] ? &enemies_right[i] : &enemies_left[i]; /* Select sprite based on facing direction */
                    enemy->frames[enemy->current_frame].x += enemy_vx[i] * scale; /* Move enemy horizontally */
                    /* Update all frames to the same position */
                    for (int j = 0; j < 3; j++) {
                        enemies_right[i].frames[j].x = enemies_left[i].frames[j].x = enemy->frames[enemy->current_frame].x;
                        enemies_right[i].frames[j].y = enemies_left[i].frames[j].y = enemy_y[i];
                    }
                    /* Update enemy animation */
                    if (++enemy->frame_counter >= enemy->frame_interval) {
                        enemy->current_frame = (enemy->current_frame + 1) % 3; /* Cycle through 3 frames */
                        enemy->frame_counter = 0;
                    }
                    /* Patrol logic: Reverse direction if enemy reaches patrol bounds */
                    float patrol_min = (i == 0) ? platform_x[3] - 50.0f : platform_x[6] - 50.0f; /* Patrol range for enemy 1 */
                    float patrol_max = (i == 0) ? platform_x[3] + 50.0f : platform_x[6] + 50.0f; /* Patrol range for enemy 2 */
                    if (enemy->frames[0].x < patrol_min || enemy->frames[0].x > patrol_max) {
                        enemy_vx[i] = -enemy_vx[i]; /* Reverse direction */
                        enemy_facing_right[i] = !enemy_facing_right[i]; /* Update facing direction */
                    }
                    /* Check for enemy-player collision (player loses on contact) */
                    if (x + PLAYER_SIZE > enemy->frames[0].x && x < enemy->frames[0].x + PLAYER_SIZE &&
                        y + PLAYER_SIZE > enemy->frames[0].y && y < enemy->frames[0].y + PLAYER_SIZE) {
                        x = 70.0f; y = WINDOW_HEIGHT - PLAYER_SIZE; /* Reset player position */
                        vx = vy = 0.0f; /* Reset velocities */
                        jump_count = coyote_frames = 0; /* Reset jump state */
                        deaths++; /* Increment death counter */
                        game_time = (float)(game_frame - start_frame) / 60.0f; /* Update game time */
                        state = Lost; /* Transition to Lost state */
                    }
                }
            }

            /* Win Condition - Check if player reaches the flag */
            if (x + PLAYER_SIZE > flag.x && x < flag.x + PLAYER_SIZE &&
                y + PLAYER_SIZE > flag.y && y < flag.y + PLAYER_SIZE) {
                game_time = (float)(game_frame - start_frame) / 60.0f; /* Calculate final time */
                if (game_time < best_time) best_time = game_time; /* Update best time if faster */
                state = Won; /* Transition to Won state */
            }
            game_time = (float)(game_frame - start_frame) / 60.0f; /* Update current game time */
        }
        /* State: Won or Lost - Display end screen and wait for restart */
        else {
            if (arcade_key_pressed_once(a_r) == 2) { /* R key: Restart the game */
                state = Playing;
                start_frame = game_frame;
                /* Reset player state */
                x = 70.0f; y = WINDOW_HEIGHT - PLAYER_SIZE;
                vx = vy = 0.0f;
                jump_count = deaths = 0;
                coyote_frames = 0;
                shot_cooldown = 0;
                game_time = 0.0f;
                /* Reset enemies */
                for (int i = 0; i < 2; i++) {
                    enemy_active[i] = 1;
                    enemies_right[i].current_frame = enemies_right[i].frame_counter = 0;
                    enemies_left[i].current_frame = enemies_left[i].frame_counter = 0;
                    for (int j = 0; j < 3; j++) {
                        enemies_right[i].frames[j].x = enemies_left[i].frames[j].x = enemy_x[i];
                        enemies_right[i].frames[j].y = enemies_left[i].frames[j].y = enemy_y[i];
                    }
                    enemy_vx[i] = (i == 0) ? ENEMY_SPEED : -ENEMY_SPEED;
                    enemy_facing_right[i] = (enemy_vx[i] > 0);
                }
            }
            if (arcade_key_pressed_once(a_esc) == 2) arcade_set_running(0); /* Exit game on ESC */
        }

        /* Update Player Sprite Positions - Sync player sprite positions with player coordinates */
        for (int i = 0; i < 8; i++) {
            run_right.frames[i].x = run_left.frames[i].x = x;
            run_right.frames[i].y = run_left.frames[i].y = y;
        }
        idle_right.x = idle_left.x = jump_right.x = jump_left.x = x;
        idle_right.y = idle_left.y = jump_right.y = jump_left.y = y;

        /* Update Animation - Update player running animation if moving */
        ArcadeAnimatedSprite *run = facing_right ? &run_right : &run_left; /* Select running animation based on facing direction */
        if (moving) {
            if (++run->frame_counter >= run->frame_interval) { /* Update animation frame */
                run->current_frame = (run->current_frame + 1) % 8;
                run->frame_counter = 0;
            }
        } else {
            run->current_frame = run->frame_counter = 0; /* Reset animation when idle */
        }

        /* Render - Add all sprites to the rendering group and draw them */
        group.count = 0; /* Reset rendering group */
        arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.image_sprite = background}, SPRITE_IMAGE); /* Add background */
        for (int i = 0; i < 8; i++) {
            arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.image_sprite = platforms[i]}, SPRITE_IMAGE); /* Add platforms */
        }
        for (int i = 0; i < 2; i++) {
            if (enemy_active[i]) {
                arcade_add_animated_to_group(&group, enemy_facing_right[i] ? &enemies_right[i] : &enemies_left[i]); /* Add active enemies */
            }
        }
        arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.image_sprite = flag}, SPRITE_IMAGE); /* Add flag */
        /* Add player sprite based on state */
        if (state == Playing) {
            if (!on_ground) {
                arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.image_sprite = facing_right ? jump_right : jump_left}, SPRITE_IMAGE); /* Jump sprite if in air */
            } else if (moving) {
                arcade_add_animated_to_group(&group, run); /* Run animation if moving */
            } else {
                arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.image_sprite = facing_right ? idle_right : idle_left}, SPRITE_IMAGE); /* Idle sprite if stationary */
            }
        } else {
            arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.image_sprite = facing_right ? idle_right : idle_left}, SPRITE_IMAGE); /* Idle sprite in Start/Won/Lost states */
        }
        /* Add active bullets that are on-screen */
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullet_active[i] && bullets[i].x >= -BULLET_SIZE && bullets[i].x < WINDOW_WIDTH &&
                bullets[i].y >= -BULLET_SIZE && bullets[i].y < WINDOW_HEIGHT) {
                arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.image_sprite = bullets[i]}, SPRITE_IMAGE);
            }
        }

        arcade_render_group(&group); /* Render all sprites in the group */

        /* UI - Render text overlays based on game state */
        if (state == Start) {
            arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.sprite = overlay}, SPRITE_COLOR); /* Add overlay for dimming */
            arcade_render_group(&group); /* Render overlay */
            arcade_render_text("Super Jump Adventure", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50, 0xFFFFFFFF); /* Game title */
            arcade_render_text("Press SPACE to start", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2, 0xFFFFFFFF); /* Start prompt */
        } else if (state == Playing) {
            snprintf(text_buffer, sizeof(text_buffer), "Time: %.1fs Deaths: %d", game_time, deaths); /* Format time and deaths */
            arcade_render_text(text_buffer, 12, WINDOW_HEIGHT - 38, 0x000000CC); /* Shadow for readability */
            arcade_render_text(text_buffer, 10, WINDOW_HEIGHT - 40, 0xFFFFFFFF); /* Main text */
        } else if (state == Won) {
            arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.sprite = overlay}, SPRITE_COLOR); /* Add overlay */
            arcade_render_group(&group); /* Render overlay */
            snprintf(text_buffer, sizeof(text_buffer), "You Won! Time: %.1fs Best: %.1fs", game_time, best_time); /* Format win message */
            arcade_render_text(text_buffer, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50, 0xFFFFFFFF); /* Win message */
            arcade_render_text("Press R to restart", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2, 0xFFFFFFFF); /* Restart prompt */
        } else if (state == Lost) {
            arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.sprite = overlay}, SPRITE_COLOR); /* Add overlay */
            arcade_render_group(&group); /* Render overlay */
            snprintf(text_buffer, sizeof(text_buffer), "Game Over! Time: %.1fs Deaths: %d", game_time, deaths); /* Format game over message */
            arcade_render_text(text_buffer, WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50, 0xFFFFFFFF); /* Game over message */
            arcade_render_text("Press R to restart", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2, 0xFFFFFFFF); /* Restart prompt */
        }

        arcade_sleep(16); /* Sleep to target ~60 FPS (16ms per frame) */
    }

    /* Cleanup - Free all allocated resources before exiting */
cleanup:
    if (background.pixels) arcade_free_image_sprite(&background); /* Free background sprite */
    if (run_right.frames) arcade_free_animated_sprite(&run_right); /* Free right-facing run animation */
    if (run_left.frames) arcade_free_animated_sprite(&run_left); /* Free left-facing run animation */
    if (idle_right.pixels) arcade_free_image_sprite(&idle_right); /* Free right-facing idle sprite */
    if (idle_left.pixels) arcade_free_image_sprite(&idle_left); /* Free left-facing idle sprite */
    if (jump_right.pixels) arcade_free_image_sprite(&jump_right); /* Free right-facing jump sprite */
    if (jump_left.pixels) arcade_free_image_sprite(&jump_left); /* Free left-facing jump sprite */
    for (int i = 0; i < 8; i++) if (platforms[i].pixels) arcade_free_image_sprite(&platforms[i]); /* Free platform sprites */
    for (int i = 0; i < 2; i++) {
        if (enemies_right[i].frames) arcade_free_animated_sprite(&enemies_right[i]); /* Free right-facing enemy animations */
        if (enemies_left[i].frames) arcade_free_animated_sprite(&enemies_left[i]); /* Free left-facing enemy animations */
    }
    for (int i = 0; i < MAX_BULLETS; i++) if (bullets[i].pixels) arcade_free_image_sprite(&bullets[i]); /* Free bullet sprites */
    if (flag.pixels) arcade_free_image_sprite(&flag); /* Free flag sprite */
    if (group.sprites) arcade_free_group(&group); /* Free rendering group */
    free_flipped_sprites(flipped_run, flipped_idle, flipped_jump, flipped_enemy, flipped_flag, 8, 3); /* Free flipped sprite paths */
    arcade_quit(); /* Close the Arcade Library window */
    return 0; /* Exit program */
}