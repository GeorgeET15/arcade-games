/* =========================================================================
 * Arcade Library - Header File
 * =========================================================================
 * Author: GeorgeET15
 * GitHub: https://github.com/GeorgeET15
 * Repository: https://github.com/GeorgeET15/arcade-lib
 *
 * Description:
 * A lightweight C library for creating 2D arcade-style games. It provides
 * essential functionality for retro game development, including window
 * management, color and image-based sprites, sprite animation, collision
 * detection, sound playback, text rendering, and image manipulation. The library
 * is designed to be minimal, with cross-platform support for Windows (Win32) and
 * Linux (X11), making it ideal for simple games like Flappy Bird or Pong.
 *
 * Features:
 * - Window creation and event handling.
 * - Support for color-based and image-based sprites with animation.
 * - Keyboard input processing with single-press detection.
 * - Pixel buffer rendering for sprites and text.
 * - WAV audio playback.
 * - Image flipping and rotation.
 *
 * Platforms:
 * - Windows: Uses Win32 API (GDI for rendering, winmm for audio).
 * - Linux: Uses X11 for rendering and aplay for audio.
 *
 * Dependencies:
 * Linux:
 * - libX11: For window creation and rendering.
 * - libm: For mathematical functions (used by STB libraries).
 * - STB libraries (stb_image.h, stb_image_write.h, stb_image_resize2.h): For
 *   image loading, writing, and resizing.
 * - aplay: For WAV audio playback (part of alsa-utils).
 * Windows:
 * - gdi32: For window rendering.
 * - winmm: For WAV audio playback.
 * - STB libraries: Same as Linux.
 *
 * Compilation:
 * Linux:
 *   gcc -o game game.c arcade.c -lX11 -lm
 * Windows:
 *   gcc -o game game.c arcade.c -lgdi32 -lwinmm
 *
 * Usage Example:
 *   #include "arcade.h"
 *   int main() {
 *       // Initialize window (800x600, black background)
 *       arcade_init(800, 600, "My Game", 0x000000);
 *       // Create an image sprite at (100,100) with size 50x50
 *       ArcadeImageSprite sprite = arcade_create_image_sprite(100, 100, 50, 50, "sprite.png");
 *       // Initialize sprite group
 *       SpriteGroup group;
 *       arcade_init_group(&group, 1);
 *       arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.image_sprite = sprite}, SPRITE_IMAGE);
 *       // Game loop (~60 FPS)
 *       while (arcade_running() && arcade_update()) {
 *           arcade_render_group(&group);
 *           arcade_sleep(16); // ~60 FPS
 *       }
 *       // Clean up
 *       arcade_free_image_sprite(&sprite);
 *       arcade_free_group(&group);
 *       arcade_quit();
 *       return 0;
 *   }
 *
 * Notes:
 * - Ensure sprite image files (e.g., PNG) are in the correct directory.
 * - WAV audio files must be PCM, 16-bit, mono/stereo for compatibility.
 * - Use arcade_sleep to control frame rate (e.g., 16ms for ~60 FPS).
 * - Check return values of initialization functions for error handling.
 * - Free all sprites and groups before calling arcade_quit to avoid memory leaks.
 * ========================================================================= */

#ifndef ARCADE_H
#define ARCADE_H

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

/* =========================================================================
 * Enumerations
 * ========================================================================= */

/* Sprite type identifiers for rendering.
 * Used to specify the type of sprite when adding to a SpriteGroup or rendering.
 * Values:
 * - SPRITE_COLOR (0): For ArcadeSprite (color-based, solid rectangle).
 * - SPRITE_IMAGE (1): For ArcadeImageSprite (image-based, loaded from file).
 * Example:
 *   arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.sprite = my_sprite}, SPRITE_COLOR);
 */
enum
{
    SPRITE_COLOR = 0, /* Color-based sprite (ArcadeSprite) */
    SPRITE_IMAGE = 1  /* Image-based sprite (ArcadeImageSprite) */
};

/* =========================================================================
 * Key Definitions
 * ========================================================================= */

/* Predefined key codes for input handling.
 * Used with arcade_key_pressed() and arcade_key_pressed_once() to detect key
 * states. Codes are platform-specific (X11 keycodes for Linux, virtual key codes
 * for Windows) but abstracted for portability.
 * Usage:
 *   if (arcade_key_pressed_once(a_space)) {
 *       // Handle spacebar press (e.g., jump)
 *   }
 * Notes:
 * - Key codes are defined as hexadecimal values.
 * - Use arcade_key_pressed for continuous press detection (e.g., holding a key).
 * - Use arcade_key_pressed_once for single-press events (e.g., triggering an action).
 */
#define a_a 0x0061 /* A key */
#define a_b 0x0062 /* B key */
#define a_c 0x0063 /* C key */
#define a_d 0x0064 /* D key */
#define a_e 0x0065 /* E key */
#define a_f 0x0066 /* F key */
#define a_g 0x0067 /* G key */
#define a_h 0x0068 /* H key */
#define a_i 0x0069 /* I key */
#define a_j 0x006a /* J key */
#define a_k 0x006b /* K key */
#define a_l 0x006c /* L key */
#define a_m 0x006d /* M key */
#define a_n 0x006e /* N key */
#define a_o 0x006f /* O key */
#define a_p 0x0070 /* P key */
#define a_q 0x0071 /* Q key */
#define a_r 0x0072 /* R key */
#define a_s 0x0073 /* S key */
#define a_t 0x0074 /* T key */
#define a_u 0x0075 /* U key */
#define a_v 0x0076 /* V key */
#define a_w 0x0077 /* W key */
#define a_x 0x0078 /* X key */
#define a_y 0x0079 /* Y key */
#define a_z 0x007a /* Z key */

#define a_0 0x0030 /* 0 key */
#define a_1 0x0031 /* 1 key */
#define a_2 0x0032 /* 2 key */
#define a_3 0x0033 /* 3 key */
#define a_4 0x0034 /* 4 key */
#define a_5 0x0035 /* 5 key */
#define a_6 0x0036 /* 6 key */
#define a_7 0x0037 /* 7 key */
#define a_8 0x0038 /* 8 key */
#define a_9 0x0039 /* 9 key */

#define a_space 0x0020      /* Spacebar */
#define a_excl 0x0021       /* ! */
#define a_quot 0x0022       /* " */
#define a_hash 0x0023       /* # */
#define a_dollar 0x0024     /* $ */
#define a_percent 0x0025    /* % */
#define a_amp 0x0026        /* & */
#define a_squote 0x0027     /* ' */
#define a_lparen 0x0028     /* ( */
#define a_rparen 0x0029     /* ) */
#define a_asterisk 0x002a   /* * */
#define a_plus 0x002b       /* + */
#define a_comma 0x002c      /* , */
#define a_minus 0x002d      /* - */
#define a_dot 0x002e        /* . */
#define a_slash 0x002f      /* / */
#define a_colon 0x003a      /* : */
#define a_semicolon 0x003b  /* ; */
#define a_less 0x003c       /* < */
#define a_equal 0x003d      /* = */
#define a_greater 0x003e    /* > */
#define a_question 0x003f   /* ? */
#define a_at 0x0040         /* @ */
#define a_lbracket 0x005b   /* [ */
#define a_backslash 0x005c  /* \ */
#define a_rbracket 0x005d   /* ] */
#define a_caret 0x005e      /* ^ */
#define a_underscore 0x005f /* _ */
#define a_backtick 0x0060   /* ` */
#define a_lbrace 0x007b     /* { */
#define a_pipe 0x007c       /* | */
#define a_rbrace 0x007d     /* } */
#define a_tilde 0x007e      /* ~ */

#define a_up 0xff52    /* Up arrow */
#define a_down 0xff54  /* Down arrow */
#define a_left 0xff51  /* Left arrow */
#define a_right 0xff53 /* Right arrow */

#define a_enter 0xff0d     /* Enter/Return */
#define a_esc 0xff1b       /* Escape */
#define a_shift 0xffe1     /* Shift (Left Shift) */
#define a_ctrl 0xffe3      /* Control (Left Ctrl) */
#define a_alt 0xffe9       /* Alt (Left Alt) */
#define a_tab 0xff09       /* Tab */
#define a_capslock 0xffe5  /* Caps Lock */
#define a_backspace 0xff08 /* Backspace */

/* =========================================================================
 * Data Structures
 * ========================================================================= */

/*
 * ArcadeSprite: Represents a color-based sprite (solid rectangle).
 * Used for simple shapes like platforms, walls, or placeholders.
 * Fields:
 * - x, y: Position (top-left corner) in window coordinates (pixels, float).
 * - width, height: Size of the sprite (pixels, float).
 * - vy, vx: Vertical and horizontal velocity (pixels per frame, float).
 * - color: RGB color (0xRRGGBB, 24-bit).
 * - active: State (1 = active, 0 = inactive, ignored in rendering/collisions).
 * Example:
 *   ArcadeSprite platform = {100.0f, 500.0f, 200.0f, 20.0f, 0.0f, 0.0f, 0x00FF00, 1};
 *   arcade_move_sprite(&platform, 0.0f, 600);
 */
typedef struct
{
    float x, y;          /* Position (pixels, float) */
    float width, height; /* Size (pixels, float) */
    float vy, vx;        /* Velocity (pixels per frame, float) */
    unsigned int color;  /* RGB color (0xRRGGBB) */
    int active;          /* Active state (1 = active, 0 = inactive) */
} ArcadeSprite;

/*
 * ArcadeImageSprite: Represents an image-based sprite loaded from a file.
 * Used for detailed graphics like characters, enemies, or backgrounds.
 * Fields:
 * - x, y: Position (top-left corner) in window coordinates (pixels, float).
 * - width, height: Size of the sprite (pixels, float, set from image dimensions).
 * - vy, vx: Vertical and horizontal velocity (pixels per frame, float).
 * - pixels: Pixel data (RGBA, 32-bit per pixel, dynamically allocated).
 * - image_width, image_height: Pixel dimensions of the image (int).
 * - active: State (1 = active, 0 = inactive, ignored in rendering/collisions).
 * Example:
 *   ArcadeImageSprite player = arcade_create_image_sprite(100.0f, 100.0f, 50.0f, 50.0f, "player.png");
 *   arcade_move_image_sprite(&player, 0.1f, 600);
 * Notes:
 * - Pixel data must be freed with arcade_free_image_sprite to avoid memory leaks.
 * - Supports PNG files (via STB libraries); other formats may work but are untested.
 */
typedef struct
{
    float x, y;                    /* Position (pixels, float) */
    float width, height;           /* Size (pixels, float) */
    float vy, vx;                  /* Velocity (pixels per frame, float) */
    uint32_t *pixels;              /* Pixel data (RGBA, 32-bit) */
    int image_width, image_height; /* Image dimensions (pixels, int) */
    int active;                    /* Active state (1 = active, 0 = inactive) */
} ArcadeImageSprite;

/*
 * ArcadeAnimatedSprite: Represents a sprite with multiple frames for animation.
 * Used for animated characters or objects (e.g., a flapping bird).
 * Fields:
 * - frames: Array of ArcadeImageSprite for each animation frame.
 * - frame_count: Number of frames in the animation.
 * - current_frame: Index of the current frame (0 to frame_count-1).
 * - frame_interval: Frames between animation updates (controls speed).
 * - frame_counter: Internal counter for tracking animation progress.
 * Example:
 *   const char *frames[] = {"bird1.png", "bird2.png", "bird3.png"};
 *   ArcadeAnimatedSprite bird = arcade_create_animated_sprite(100.0f, 100.0f, 50.0f, 50.0f, frames, 3, 5);
 *   arcade_move_animated_sprite(&bird, 0.1f, 600);
 * Notes:
 * - All frames must have the same dimensions.
 * - Free with arcade_free_animated_sprite to avoid memory leaks.
 */
typedef struct
{
    ArcadeImageSprite *frames; /* Array of frames */
    int frame_count;           /* Number of frames */
    int current_frame;         /* Current frame index */
    int frame_interval;        /* Frames between animation updates */
    int frame_counter;         /* Animation progress counter */
} ArcadeAnimatedSprite;

/*
 * ArcadeAnySprite: Union to handle both color and image-based sprites.
 * Allows SpriteGroup to store either ArcadeSprite or ArcadeImageSprite.
 * Fields:
 * - sprite: ArcadeSprite (color-based).
 * - image_sprite: ArcadeImageSprite (image-based).
 * Example:
 *   ArcadeAnySprite any_sprite = {.image_sprite = player};
 *   arcade_add_sprite_to_group(&group, any_sprite, SPRITE_IMAGE);
 * Notesmong:
 * - Use with SPRITE_COLOR or SPRITE_IMAGE to specify the type.
 * - Ensures type safety when rendering mixed sprite types.
 */
typedef union
{
    ArcadeSprite sprite;            /* Color-based sprite */
    ArcadeImageSprite image_sprite; /* Image-based sprite */
} ArcadeAnySprite;

/*
 * SpriteGroup: Manages a collection of sprites for batch rendering.
 * Simplifies rendering multiple sprites in a single call.
 * Fields:
 * - sprites: Array of ArcadeAnySprite (color or image-based).
 * - types: Array of sprite types (SPRITE_COLOR or SPRITE_IMAGE).
 * - count: Current number of sprites in the group.
 * - capacity: Maximum number of sprites the group can hold.
 * Example:
 *   SpriteGroup group;
 *   arcade_init_group(&group, 10);
 *   arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.image_sprite = player}, SPRITE_IMAGE);
 *   arcade_render_group(&group);
 * Notes:
 * - Initialize with arcade_init_group before use.
 * - Free with arcade_free_group to avoid memory leaks.
 */
typedef struct
{
    ArcadeAnySprite *sprites; /* Array of sprites */
    int *types;               /* Array of sprite types */
    int count;                /* Current sprite count */
    int capacity;             /* Maximum sprite count */
} SpriteGroup;

/* =========================================================================
 * Core Functions
 * ========================================================================= */

/*
 * arcade_init: Initializes the arcade environment, creating a window.
 * Sets up the rendering context, input handling, and pixel buffer.
 * Parameters:
 * - window_width: Width of the window (pixels, e.g., 800).
 * - window_height: Height of the window (pixels, e.g., 600).
 * - window_title: Title of the window (e.g., "My Game").
 * - bg_color: Background color (0xRRGGBB, e.g., 0x000000 for black).
 * Returns:
 * - 0 on success.
 * - Non-zero on failure (e.g., window creation failed).
 * Example:
 *   if (arcade_init(800, 600, "My Game", 0x000000)) {
 *       fprintf(stderr, "Initialization failed\n");
 *       return 1;
 *   }
 * Notes:
 * - Call once at program start.
 * - Check return value for error handling.
 * - Window is non-resizable and centered on the screen.
 */
int arcade_init(int window_width, int window_height, const char *window_title, uint32_t bg_color);

/*
 * arcade_quit: Cleans up the arcade environment, freeing resources.
 * Closes the window, releases fonts, and frees pixel buffers.
 * Parameters: None.
 * Returns: None.
 * Example:
 *   arcade_quit();
 * Notes:
 * - Call before program exit to ensure proper cleanup.
 * - Free all sprites and groups before calling to avoid memory leaks.
 */
void arcade_quit(void);

/*
 * arcade_update: Processes events (e.g., key presses, window close).
 * Updates input states and checks for window closure.
 * Parameters: None.
 * Returns:
 * - 1 if the game should continue running.
 * - 0 if the game should stop (e.g., window closed).
 * Example:
 *   while (arcade_running() && arcade_update()) {
 *       // Update game state
 *   }
 * Notes:
 * - Call once per frame in the game loop.
 * - Handles platform-specific events (Win32 messages, X11 events).
 */
int arcade_update(void);

/*
 * arcade_running: Checks if the game is running.
 * Reflects whether the window is open and the game loop should continue.
 * Parameters: None.
 * Returns:
 * - 1 if the game is running.
 * - 0 if the game should stop (e.g., window closed or arcade_set_running(0)).
 * Example:
 *   while (arcade_running()) {
 *       arcade_update();
 *   }
 * Notes:
 * - Use in conjunction with arcade_update for the game loop.
 * - Modified by arcade_set_running or window close events.
 */
int arcade_running(void);

/*
 * arcade_set_running: Sets the running state of the game.
 * Allows manual control over the game loop (e.g., to exit programmatically).
 * Parameters:
 * - value: New running state (1 = running, 0 = stopped).
 * Returns: None.
 * Example:
 *   if (arcade_key_pressed_once(a_esc)) {
 *       arcade_set_running(0); // Exit on ESC
 *   }
 * Notes:
 * - Typically used for user-initiated exit (e.g., pressing ESC).
 * - Does not close the window; call arcade_quit for cleanup.
 */
void arcade_set_running(int value);

/*
 * arcade_sleep: Pauses execution for a specified number of milliseconds.
 * Used to control frame rate by limiting how often the game loop runs.
 * Parameters:
 * - milliseconds: Duration to sleep (e.g., 16 for ~60 FPS).
 * Returns: None.
 * Example:
 *   while (arcade_running() && arcade_update()) {
 *       arcade_render_group(&group);
 *       arcade_sleep(16); // ~60 FPS
 *   }
 * Notes:
 * - Uses Sleep (Windows) or usleep (Linux).
 * - Approximate; actual frame rate depends on system scheduling.
 * - Common values: 16ms (~60 FPS), 33ms (~30 FPS).
 */
void arcade_sleep(unsigned int milliseconds);

/*
 * arcade_delta_time: Calculates the time elapsed since the last frame in seconds.
 * Used to scale movement and updates for frame-rate-independent gameplay.
 * Parameters: None.
 * Returns: Time difference in seconds (float) since the last call.
 * Example:
 *   while (arcade_running() && arcade_update()) {
 *       float dt = arcade_delta_time();
 *       player.x += player.vx * dt * 60.0f; // Normalize to 60 FPS
 *       arcade_render_group(&group);
 *   }
 * Notes:
 * - Uses QueryPerformanceCounter (Windows) or clock_gettime (Linux).
 * - Clamps delta time to 0.1s max to prevent large jumps during lag.
 * - First call returns 0.0f to avoid initial movement spikes.
 * - Typical values: ~0.0167s (60 FPS), ~0.0333s (30 FPS).
 */
float arcade_delta_time(void);

/* =========================================================================
 * Input Handling
 * ========================================================================= */

/*
 * arcade_key_pressed: Checks if a specific key is currently pressed.
 * Detects continuous key presses (e.g., holding a key for movement).
 * Parameters:
 * - key_val: Key code (e.g., a_space, a_w, defined above).
 * Returns:
 * - 2 if the key is pressed.
 * - 0 if the key is not pressed.
 * Example:
 *   if (arcade_key_pressed(a_right)) {
 *       player.vx = 5.0f; // Move right
 *   } else {
 *       player.vx = 0.0f;
 *   }
 * Notes:
 * - Suitable for actions requiring sustained input (e.g., movement).
 * - Uses platform-specific key mappings (Win32 virtual keys, X11 keycodes).
 */
int arcade_key_pressed(unsigned int key_val);

/*
 * arcade_key_pressed_once: Checks if a key was pressed in the current frame.
 * Detects single-press events (e.g., for triggering actions like jumping).
 * Parameters:
 * - key_val: Key code (e.g., a_space, a_p).
 * Returns:
 * - 2 if the key was pressed this frame (and not last frame).
 * - 0 otherwise.
 * Example:
 *   if (arcade_key_pressed_once(a_space)) {
 *       player.vy = -10.0f; // Jump
 *   }
 * Notes:
 * - Ideal for one-time actions (e.g., jump, pause, restart).
 * - Tracks key state changes to ensure single detection per press.
 */
int arcade_key_pressed_once(unsigned int key_val);

/*
 * arcade_clear_keys: Resets all key states to unpressed.
 * Clears the input state, useful for state transitions (e.g., pausing).
 * Parameters: None.
 * Returns: None.
 * Example:
 *   if (arcade_key_pressed_once(a_p)) {
 *       game_paused = !game_paused;
 *       arcade_clear_keys(); // Reset inputs on pause
 *   }
 * Notes:
 * - Affects all keys tracked by arcade_key_pressed and arcade_key_pressed_once.
 * - Call sparingly to avoid missing input events.
 */
void arcade_clear_keys(void);

/* =========================================================================
 * Sprite Management
 * ========================================================================= */

/*
 * arcade_move_sprite: Updates the position of a color-based sprite.
 * Applies gravity and velocity, with boundary checks to keep the sprite in the window.
 * Parameters:
 * - sprite: Pointer to ArcadeSprite to update.
 * - gravity: Gravity acceleration (pixels per frame^2, e.g., 0.1f).
 * - window_height: Height of the window (for boundary checks).
 * Returns: None.
 * Example:
 *   ArcadeSprite player = {100.0f, 100.0f, 50.0f, 50.0f, 0.0f, 0.0f, 0xFF0000, 1};
 *   arcade_move_sprite(&player, 0.1f, 600);
 * Notes:
 * - Updates sprite->x, sprite->y, sprite->vy based on sprite->vx, gravity.
 * - Clamps y-position to [0, window_height - sprite->height].
 * - Ignores inactive sprites (sprite->active == 0).
 */
void arcade_move_sprite(ArcadeSprite *sprite, float gravity, int window_height);

/*
 * arcade_move_image_sprite: Updates the position of an image-based sprite.
 * Similar to arcade_move_sprite but for ArcadeImageSprite.
 * Parameters:
 * - sprite: Pointer to ArcadeImageSprite to update.
 * - gravity: Gravity acceleration (pixels per frame^2).
 * - window_height: Height of the window.
 * Returns: None.
 * Example:
 *   ArcadeImageSprite player = arcade_create_image_sprite(100.0f, 100.0f, 50.0f, 50.0f, "player.png");
 *   arcade_move_image_sprite(&player, 0.1f, 600);
 * Notes:
 * - Same movement logic as arcade_move_sprite.
 * - Ignores inactive sprites or null pointers.
 */
void arcade_move_image_sprite(ArcadeImageSprite *sprite, float gravity, int window_height);

/*
 * arcade_check_collision: Checks for collision between two color-based sprites.
 * Uses axis-aligned bounding box (AABB) collision detection.
 * Parameters:
 * - a: Pointer to first ArcadeSprite.
 * - b: Pointer to second ArcadeSprite.
 * Returns:
 * - 1 if the sprites collide.
 * - 0 if no collision, or if either sprite is null or inactive.
 * Example:
 *   if (arcade_check_collision(&player, &platform)) {
 *       player.vy = 0.0f; // Stop falling
 *   }
 * Notes:
 * - Checks if rectangles overlap based on x, y, width, height.
 * - Only active sprites (active == 1) are considered.
 */
int arcade_check_collision(ArcadeSprite *a, ArcadeSprite *b);

/*
 * arcade_check_image_collision: Checks for collision between two image-based sprites.
 * Uses AABB collision detection.
 * Parameters:
 * - a: Pointer to first ArcadeImageSprite.
 * - b: Pointer to second ArcadeImageSprite.
 * Returns:
 * - 1 if the sprites collide.
 * - 0 if no collision, or if either sprite is null or inactive.
 * Example:
 *   ArcadeImageSprite player = arcade_create_image_sprite(100.0f, 100.0f, 50.0f, 50.0f, "player.png");
 *   ArcadeImageSprite enemy = arcade_create_image_sprite(150.0f, 100.0f, 50.0f, 50.0f, "enemy.png");
 *   if (arcade_check_image_collision(&player, &enemy)) {
 *       // Handle collision
 *   }
 * Notes:
 * - Same logic as arcade_check_collision but for image sprites.
 * - Does not check pixel-level collision (only bounding boxes).
 */
int arcade_check_image_collision(ArcadeImageSprite *a, ArcadeImageSprite *b);

/*
 * arcade_create_image_sprite: Creates an image-based sprite from a file.
 * Loads and resizes an image (e.g., PNG) to the specified dimensions.
 * Parameters:
 * - x, y: Initial position (pixels, float).
 * - w, h: Desired width and height (pixels, float).
 * - filename: Path to the image file (e.g., "sprites/player.png").
 * Returns:
 * - ArcadeImageSprite with loaded pixel data, or an empty sprite if loading fails.
 * Example:
 *   ArcadeImageSprite player = arcade_create_image_sprite(100.0f, 100.0f, 50.0f, 50.0f, "player.png");
 *   if (!player.pixels) {
 *       fprintf(stderr, "Failed to load player sprite\n");
 *   }
 * Notes:
 * - Uses STB libraries to load and resize images.
 * - Pixel data is dynamically allocated; free with arcade_free_image_sprite.
 * - Sets active = 1 on success, 0 on failure.
 */
ArcadeImageSprite arcade_create_image_sprite(float x, float y, float w, float h, const char *filename);

/*
 * arcade_free_image_sprite: Frees the pixel data of an image-based sprite.
 * Releases memory allocated for the sprite’s pixels.
 * Parameters:
 * - sprite: Pointer to ArcadeImageSprite to free.
 * Returns: None.
 * Example:
 *   ArcadeImageSprite player = arcade_create_image_sprite(100.0f, 100.0f, 50.0f, 50.0f, "player.png");
 *   arcade_free_image_sprite(&player);
 * Notes:
 * - Safe to call on null or already-freed sprites.
 * - Sets pixels = NULL, image_width = 0, image_height = 0, active = 0.
 */
void arcade_free_image_sprite(ArcadeImageSprite *sprite);

/*
 * arcade_create_animated_sprite: Creates an animated sprite with multiple frames.
 * Loads a sequence of images for animation (e.g., walking cycle).
 * Parameters:
 * - x, y: Initial position (pixels, float).
 * - w, h: Desired width and height for each frame (pixels, float).
 * - filenames: Array of image file paths (e.g., {"frame1.png", "frame2.png"}).
 * - frame_count: Number of frames.
 * - frame_interval: Frames between animation updates (e.g., 5 for 12 FPS at 60 FPS).
 * Returns:
 * - ArcadeAnimatedSprite with loaded frames, or an empty sprite if loading fails.
 * Example:
 *   const char *frames[] = {"bird1.png", "bird2.png", "bird3.png"};
 *   ArcadeAnimatedSprite bird = arcade_create_animated_sprite(100.0f, 100.0f, 50.0f, 50.0f, frames, 3, 5);
 * Notes:
 * - All frames are resized to w x h.
 * - Free with arcade_free_animated_sprite to avoid memory leaks.
 * - Sets first frame’s active = 1, others = 0.
 */
ArcadeAnimatedSprite arcade_create_animated_sprite(float x, float y, float w, float h, const char **filenames, int frame_count, int frame_interval);

/*
 * arcade_free_animated_sprite: Frees all frames of an animated sprite.
 * Releases memory for all frame pixel data.
 * Parameters:
 * - anim: Pointer to ArcadeAnimatedSprite to free.
 * Returns: None.
 * Example:
 *   ArcadeAnimatedSprite bird = arcade_create_animated_sprite(100.0f, 100.0f, 50.0f, 50.0f, frames, 3, 5);
 *   arcade_free_animated_sprite(&bird);
 * Notes:
 * - Safe to call on null or already-freed sprites.
 * - Sets frames = NULL, frame_count = 0.
 */
void arcade_free_animated_sprite(ArcadeAnimatedSprite *anim);

/*
 * arcade_move_animated_sprite: Updates the position and animation of an animated sprite.
 * Applies gravity and velocity to the current frame and syncs other frames’ positions.
 * Parameters:
 * - anim: Pointer to ArcadeAnimatedSprite to update.
 * - gravity: Gravity acceleration (pixels per frame^2).
 * - window_height: Height of the window.
 * Returns: None.
 * Example:
 *   arcade_move_animated_sprite(&bird, 0.1f, 600);
 * Notes:
 * - Updates current frame’s position and velocity, then copies to other frames.
 * - Advances animation based on frame_interval.
 * - Ignores inactive or null sprites.
 */
void arcade_move_animated_sprite(ArcadeAnimatedSprite *anim, float gravity, int window_height);

/*
 * arcade_check_animated_collision: Checks for collision between an animated sprite and an image-based sprite.
 * Uses AABB collision detection on the current frame.
 * Parameters:
 * - anim: Pointer to ArcadeAnimatedSprite.
 * - other: Pointer to ArcadeImageSprite.
 * Returns:
 * - 1 if the sprites collide.
 * - 0 if no collision, or if either sprite is null or inactive.
 * Example:
 *   if (arcade_check_animated_collision(&bird, &pipe)) {
 *       // Handle collision (e.g., game over)
 *   }
 * Notes:
 * - Checks only the current frame’s bounding box.
 * - Same logic as arcade_check_image_collision.
 */
int arcade_check_animated_collision(ArcadeAnimatedSprite *anim, ArcadeImageSprite *other);

/* =========================================================================
 * Rendering
 * ========================================================================= */

/*
 * arcade_render_scene: Renders a scene with multiple sprites.
 * Draws all sprites to the pixel buffer and updates the window.
 * Parameters:
 * - sprites: Array of ArcadeAnySprite (color or image-based).
 * - count: Number of sprites to render.
 * - types: Array of sprite types (SPRITE_COLOR or SPRITE_IMAGE).
 * Returns: None.
 * Example:
 *   ArcadeAnySprite sprites[2] = {{.sprite = platform}, {.image_sprite = player}};
 *   int types[2] = {SPRITE_COLOR, SPRITE_IMAGE};
 *   arcade_render_scene(sprites, 2, types);
 * Notes:
 * - Clears the screen to the background color before rendering.
 * - Uses double buffering (Windows: GDI bitmap, Linux: XImage).
 * - Ignores inactive or null sprites.
 */
void arcade_render_scene(ArcadeAnySprite *sprites, int count, int *types);

/*
 * arcade_render_text: Renders text at a specified position.
 * Draws text using a fixed font (Courier New on Windows, 9x15 on Linux).
 * Parameters:
 * - text: Null-terminated string to render.
 * - x, y: Position of the text’s top-left corner (pixels, float).
 * - color: Text color (0xRRGGBB, e.g., 0xFFFFFF for white).
 * Returns: None.
 * Example:
 *   arcade_render_text("Score: 10", 10.0f, 10.0f, 0xFFFFFF);
 * Notes:
 * - Text is rendered with a transparent background.
 * - Skips rendering if text is null or font is unavailable.
 */
void arcade_render_text(const char *text, float x, float y, unsigned int color);

/*
 * arcade_render_text_centered: Renders text centered horizontally.
 * Calculates the x-position to center the text based on its width.
 * Parameters:
 * - text: Null-terminated string to render.
 * - y: Vertical position of the text’s top edge (pixels, float).
 * - color: Text color (0xRRGGBB).
 * Returns: None.
 * Example:
 *   arcade_render_text_centered("Game Over", 300.0f, 0xFF0000);
 * Notes:
 * - Uses the same font as arcade_render_text.
 * - Skips rendering if text is null or font is unavailable.
 */
void arcade_render_text_centered(const char *text, float y, unsigned int color);

/*
 * arcade_render_text_centered_blink: Renders centered text that blinks.
 * Toggles visibility based on a frame interval.
 * Parameters:
 * - text: Null-terminated string to render.
 * - y: Vertical position of the text’s top edge (pixels, float).
 * - color: Text color (0xRRGGBB).
 * - blink_interval: Frames for each on/off cycle (e.g., 30 for 1-second blink at 60 FPS).
 * Returns: None.
 * Example:
 *   arcade_render_text_centered_blink("Press Space", 300.0f, 0xFFFFFF, 30);
 * Notes:
 * - Blinks by rendering only when frame_counter % (2 * blink_interval) < blink_interval.
 * - Useful for start screens or alerts.
 */
void arcade_render_text_centered_blink(const char *text, float y, unsigned int color, int blink_interval);

/* =========================================================================
 * Sprite Groups
 * ========================================================================= */

/*
 * arcade_init_group: Initializes a sprite group with a specified capacity.
 * Allocates memory for sprites and their types.
 * Parameters:
 * - group: Pointer to SpriteGroup to initialize.
 * - capacity: Maximum number of sprites the group can hold.
 * Returns: None.
 * Example:
 *   SpriteGroup group;
 *   arcade_init_group(&group, 10);
 * Notes:
 * - Must be called before adding sprites.
 * - Sets count = 0, capacity = specified value.
 * - Free with arcade_free_group to avoid memory leaks.
 */
void arcade_init_group(SpriteGroup *group, int capacity);

/*
 * arcade_add_sprite_to_group: Adds a sprite to a sprite group.
 * Stores the sprite and its type for batch rendering.
 * Parameters:
 * - group: Pointer to SpriteGroup.
 * - sprite: ArcadeAnySprite to add (color or image-based).
 * - type: Sprite type (SPRITE_COLOR or SPRITE_IMAGE).
 * Returns: None.
 * Example:
 *   arcade_add_sprite_to_group(&group, (ArcadeAnySprite){.image_sprite = player}, SPRITE_IMAGE);
 * Notes:
 * - Ignores if group is full (count >= capacity).
 * - Sprite is copied, so updates to the original sprite are not reflected unless re-added.
 */
void arcade_add_sprite_to_group(SpriteGroup *group, ArcadeAnySprite sprite, int type);

/*
 * arcade_add_animated_to_group: Adds an animated sprite’s current frame to a group.
 * Adds the current frame of an ArcadeAnimatedSprite as an image-based sprite.
 * Parameters:
 * - group: Pointer to SpriteGroup.
 * - anim: Pointer to ArcadeAnimatedSprite.
 * Returns: None.
 * Example:
 *   arcade_add_animated_to_group(&group, &bird);
 * Notes:
 * - Only the current frame is added (type = SPRITE_IMAGE).
 * - Call each frame to update the animation in the group.
 * - Ignores inactive or null animated sprites.
 */
void arcade_add_animated_to_group(SpriteGroup *group, ArcadeAnimatedSprite *anim);

/*
 * arcade_render_group: Renders all sprites in a sprite group.
 * Calls arcade_render_scene with the group’s sprites and types.
 * Parameters:
 * - group: Pointer to SpriteGroup.
 * Returns: None.
 * Example:
 *   arcade_render_group(&group);
 * Notes:
 * - Clears the screen and renders all active sprites.
 * - More efficient than rendering sprites individually.
 */
void arcade_render_group(SpriteGroup *group);

/*
 * arcade_free_group: Frees the memory allocated for a sprite group.
 * Releases memory for sprites and types arrays.
 * Parameters:
 * - group: Pointer to SpriteGroup to free.
 * Returns: None.
 * Example:
 *   arcade_free_group(&group);
 * Notes:
 * - Does not free individual sprite pixel data (use arcade_free_image_sprite or arcade_free_animated_sprite).
 * - Sets count = 0, capacity = 0.
 */
void arcade_free_group(SpriteGroup *group);

/* =========================================================================
 * Audio
 * ========================================================================= */

/*
 * arcade_play_sound: Plays a WAV audio file.
 * Plays the file asynchronously (non-blocking).
 * Parameters:
 * - audio_file_path: Path to the WAV file (e.g., "audio/sfx.wav").
 * Returns:
 * - 0 on success.
 * - Non-zero on failure (e.g., file not found, invalid format).
 * Example:
 *   if (arcade_key_pressed_once(a_space)) {
 *       arcade_play_sound("audio/jump.wav");
 *   }
 * Notes:
 * - Windows: Uses PlaySound with SND_FILENAME | SND_ASYNC.
 * - Linux: Uses aplay (requires alsa-utils).
 * - WAV files must be PCM, 16-bit, mono/stereo.
 * - Frequent calls may cause delays on slow systems; consider preloading for Windows.
 */
int arcade_play_sound(const char *audio_file_path);

/*
 * arcade_stop_sound: Stops any currently playing WAV audio.
 * Immediately halts sound playback started by arcade_play_sound.
 *
 * Returns:
 * - 0 on success.
 * - Non-zero on failure (e.g., no sound playing or command fails).
 *
 * Example:
 *   if (arcade_key_pressed_once(a_s)) {
 *       arcade_stop_sound();
 *   }
 *
 * Notes:
 * - Windows: Uses PlaySound(NULL, NULL, 0) to stop current async playback.
 * - Linux: Uses pkill to terminate background `aplay` processes.
 *   This assumes that only arcade_play_sound uses `aplay -q`.
 * - This function stops all active sounds immediately. It does not pause.
 * - If multiple arcade_play_sound calls were triggered quickly, this will halt them all.
 */
int arcade_stop_sound(void);

/* =========================================================================
 * Image Manipulation
 * ========================================================================= */

/*
 * arcade_flip_image: Flips an image vertically or horizontally.
 * Creates a new image file with the flipped content.
 * Parameters:
 * - input_path: Path to the input image (e.g., "sprites/player.png").
 * - flip_type: 1 for vertical flip, 0 for horizontal flip.
 * Returns:
 * - Path to the flipped image (temporary file), or NULL on failure.
 * Example:
 *   char *flipped = arcade_flip_image("player.png", 0); // Horizontal flip
 *   if (flipped) {
 *       ArcadeImageSprite sprite = arcade_create_image_sprite(100.0f, 100.0f, 50.0f, 50.0f, flipped);
 *       free(flipped);
 *   }
 * Notes:
 * - Uses STB libraries for image processing.
 * - Creates a temporary PNG file (Windows: current directory, Linux: /tmp).
 * - Caller must free the returned path.
 * - Ensure write permissions in the output directory.
 */
char *arcade_flip_image(const char *input_path, int flip_type);

/*
 * arcade_rotate_image: Rotates an image by 0, 90, 180, or 270 degrees.
 * Creates a new image file with the rotated content.
 * Parameters:
 * - input_path: Path to the input image (e.g., "sprites/player.png").
 * - degrees: Rotation angle (0, 90, 180, 270).
 * Returns:
 * - Path to the rotated image (temporary file), or NULL on failure.
 * Example:
 *   char *rotated = arcade_rotate_image("player.png", 90);
 *   if (rotated) {
 *       ArcadeImageSprite sprite = arcade_create_image_sprite(100.0f, 100.0f, 50.0f, 50.0f, rotated);
 *       free(rotated);
 *   }
 * Notes:
 * - Uses STB libraries for image processing.
 * - Creates a temporary PNG file (Windows: current directory, Linux: /tmp).
 * - Caller must free the returned path.
 * - Rotations of 90/270 swap width and height.
 */
char *arcade_rotate_image(const char *input_path, int degrees);

#endif

/* =========================================================================
 * Implementation
 * ========================================================================= */

#ifdef ARCADE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <errno.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"

/* =========================================================================
 * Internal State
 * ========================================================================= */
#ifdef _WIN32
typedef struct
{
    HWND hwnd;         /* Window handle for the game window */
    HDC hdc;           /* Device context for rendering operations */
    HBITMAP hbitmap;   /* Bitmap for double buffering to reduce flicker */
    uint32_t *pixels;  /* Pixel buffer for storing rendered frame data */
    int width, height; /* Window dimensions in pixels */
    uint32_t bg_color; /* Background color (0xRRGGBB) for clearing the screen */
    HFONT hfont;       /* Font handle for text rendering (Courier New) */
    int running;       /* Game running state (1 = running, 0 = stopped) */
} ArcadeState;
#else
typedef struct
{
    Display *display;  /* X11 display connection for communicating with the X server */
    Window window;     /* Game window identifier */
    int screen;        /* Default screen number for the display */
    uint32_t *pixels;  /* Pixel buffer for storing rendered frame data */
    int width, height; /* Window dimensions in pixels */
    Atom wm_delete;    /* Atom for handling window close events */
    XImage *image;     /* X11 image for rendering pixel data to the window */
    GC gc;             /* Graphics context for drawing operations */
    uint32_t bg_color; /* Background color (0xRRGGBB) for clearing the screen */
    XFontStruct *font; /* Font structure for text rendering (9x15 font) */
    int running;       /* Game running state (1 = running, 0 = stopped) */
} ArcadeState;
#endif

static ArcadeState state = {0};           /* Global state for the arcade environment */
static int key_states[256] = {0};         /* Current key states (0 = up, 1 = down) for input tracking */
static int last_key_states[256] = {0};    /* Previous key states for detecting single-press events */
static int global_frame_counter = 0;      /* Global frame counter for animations and blinking effects */


/* =========================================================================
 * Platform-Specific Input Handling (Windows Only)
 * ========================================================================= */
#ifdef _WIN32
static int arcade_to_vk(unsigned int arcade_key)
{
    /* Maps arcade key codes to Windows virtual key codes for input handling */
    switch (arcade_key)
    {
    // Arrow keys
    case a_up:
        return VK_UP;    /* Maps to Windows virtual key for up arrow */
    case a_down:
        return VK_DOWN;  /* Maps to Windows virtual key for down arrow */
    case a_left:
        return VK_LEFT;  /* Maps to Windows virtual key for left arrow */
    case a_right:
        return VK_RIGHT; /* Maps to Windows virtual key for right arrow */

    // Letters
    case a_a:
        return 'A';
    case a_b:
        return 'B';
    case a_c:
        return 'C';
    case a_d:
        return 'D';
    case a_e:
        return 'E';
    case a_f:
        return 'F';
    case a_g:
        return 'G';
    case a_h:
        return 'H';
    case a_i:
        return 'I';
    case a_j:
        return 'J';
    case a_k:
        return 'K';
    case a_l:
        return 'L';
    case a_m:
        return 'M';
    case a_n:
        return 'N';
    case a_o:
        return 'O';
    case a_p:
        return 'P';
    case a_q:
        return 'Q';
    case a_r:
        return 'R';
    case a_s:
        return 'S';
    case a_t:
        return 'T';
    case a_u:
        return 'U';
    case a_v:
        return 'V';
    case a_w:
        return 'W';
    case a_x:
        return 'X';
    case a_y:
        return 'Y';
    case a_z:
        return 'Z';

    // Numbers
    case a_0:
        return '0';
    case a_1:
        return '1';
    case a_2:
        return '2';
    case a_3:
        return '3';
    case a_4:
        return '4';
    case a_5:
        return '5';
    case a_6:
        return '6';
    case a_7:
        return '7';
    case a_8:
        return '8';
    case a_9:
        return '9';

    // Punctuation/special character keys (partial support; map to virtual key codes as appropriate)
    case a_space:
        return VK_SPACE;
    case a_enter:
        return VK_RETURN;
    case a_tab:
        return VK_TAB;
    case a_esc:
        return VK_ESCAPE;
    case a_backspace:
        return VK_BACK;
    case a_shift:
        return VK_SHIFT;
    case a_ctrl:
        return VK_CONTROL;
    case a_alt:
        return VK_MENU;
    case a_capslock:
        return VK_CAPITAL;

    default:
        return 0;
    }
}

#endif

/* =========================================================================
 * Platform-Specific Window Procedure (Windows Only)
 * ========================================================================= */
#ifdef _WIN32
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
    {
        int vk = (int)wParam;
        if (vk < 256)
            key_states[vk] = 1;
        break;
    }
    case WM_KEYUP:
    {
        int vk = (int)wParam;
        if (vk < 256)
            key_states[vk] = 0;
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HDC memDC = CreateCompatibleDC(hdc);
        SelectObject(memDC, state.hbitmap);
        BitBlt(hdc, 0, 0, state.width, state.height, memDC, 0, 0, SRCCOPY);
        DeleteDC(memDC);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
        state.running = 0;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
#endif

/* =========================================================================
 * Core Functions
 * ========================================================================= */

int arcade_init(int window_width, int window_height, const char *window_title, uint32_t bg_color)
{
#ifdef _WIN32
    /* Set up Windows-specific window class for the game window */
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;                    /* Assign window procedure for event handling */
    wc.hInstance = GetModuleHandle(NULL);        /* Get current application instance */
    wc.lpszClassName = "ArcadeWindow";           /* Name of the window class */
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);    /* Use standard arrow cursor */
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); /* Default window background */
    RegisterClass(&wc);

    /* Create a non-resizable window with specified dimensions */
    DWORD style = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME;
    RECT rect = {0, 0, window_width, window_height};
    AdjustWindowRect(&rect, style, FALSE);        /* Adjust window size to account for borders */
    state.hwnd = CreateWindow("ArcadeWindow", window_title, style,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              rect.right - rect.left, rect.bottom - rect.top,
                              NULL, NULL, wc.hInstance, NULL);
    if (!state.hwnd)
    {
        fprintf(stderr, "Cannot create window\n");
        return 1;                                /* Return error code on failure */
    }
    ShowWindow(state.hwnd, SW_SHOW);             /* Display the window */
    UpdateWindow(state.hwnd);                    /* Force initial window update */

    state.hdc = GetDC(state.hwnd);
    state.width = window_width;
    state.height = window_height;
    state.bg_color = bg_color;
    state.running = 1;

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = window_width;
    bmi.bmiHeader.biHeight = -window_height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    state.hbitmap = CreateDIBSection(state.hdc, &bmi, DIB_RGB_COLORS, (void **)&state.pixels, NULL, 0);
    if (!state.hbitmap || !state.pixels)
    {
        ReleaseDC(state.hwnd, state.hdc);
        DestroyWindow(state.hwnd);
        fprintf(stderr, "Cannot create bitmap\n");
        return 1;
    }

    for (int i = 0; i < window_width * window_height; i++)
    {
        state.pixels[i] = bg_color;
    }

    state.hfont = CreateFont(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
    if (!state.hfont)
    {
        DeleteObject(state.hbitmap);
        ReleaseDC(state.hwnd, state.hdc);
        DestroyWindow(state.hwnd);
        fprintf(stderr, "Cannot create font\n");
        return 1;
    }
#else
    state.display = XOpenDisplay(NULL);
    if (!state.display)
    {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    state.screen = DefaultScreen(state.display);
    state.window = XCreateSimpleWindow(state.display, RootWindow(state.display, state.screen),
                                       100, 100, window_width, window_height, 1,
                                       BlackPixel(state.display, state.screen),
                                       WhitePixel(state.display, state.screen));
    XStoreName(state.display, state.window, window_title);
    XSelectInput(state.display, state.window, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask);
    state.wm_delete = XInternAtom(state.display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(state.display, state.window, &state.wm_delete, 1);
    XMapWindow(state.display, state.window);

    state.pixels = malloc(window_width * window_height * sizeof(uint32_t));
    if (!state.pixels)
    {
        XCloseDisplay(state.display);
        fprintf(stderr, "Cannot allocate pixels\n");
        return 1;
    }
    state.width = window_width;
    state.height = window_height;
    state.bg_color = bg_color;
    state.running = 1;

    state.font = XLoadQueryFont(state.display, "9x15");
    if (!state.font)
    {
        fprintf(stderr, "Cannot load font 9x15\n");
        XCloseDisplay(state.display);
        free(state.pixels);
        return 1;
    }

    state.image = XCreateImage(state.display, DefaultVisual(state.display, state.screen),
                               DefaultDepth(state.display, state.screen), ZPixmap, 0,
                               (char *)state.pixels, window_width, window_height, 32, 0);
    if (!state.image)
    {
        free(state.pixels);
        XCloseDisplay(state.display);
        fprintf(stderr, "Cannot create XImage\n");
        return 1;
    }

    state.gc = XCreateGC(state.display, state.window, 0, NULL);
    if (!state.gc)
    {
        XDestroyImage(state.image);
        XCloseDisplay(state.display);
        fprintf(stderr, "Cannot create GC\n");
        return 1;
    }

    for (int i = 0; i < state.width * state.height; i++)
    {
        state.pixels[i] = bg_color;
    }
#endif
    return 0;
}

void arcade_quit(void)
{
#ifdef _WIN32
    if (state.hfont)
    {
        DeleteObject(state.hfont);
        state.hfont = NULL;
    }
    if (state.hbitmap)
    {
        DeleteObject(state.hbitmap);
        state.hbitmap = NULL;
        state.pixels = NULL;
    }
    if (state.hdc)
    {
        ReleaseDC(state.hwnd, state.hdc);
        state.hdc = NULL;
    }
    if (state.hwnd)
    {
        DestroyWindow(state.hwnd);
        state.hwnd = NULL;
    }
#else
    if (state.font)
    {
        XFreeFont(state.display, state.font);
        state.font = NULL;
    }
    if (state.image)
    {
        XDestroyImage(state.image);
        state.image = NULL;
        state.pixels = NULL;
    }
    if (state.gc)
    {
        XFreeGC(state.display, state.gc);
        state.gc = NULL;
    }
    if (state.display && state.window)
    {
        XDestroyWindow(state.display, state.window);
        state.window = 0;
    }
    if (state.display)
    {
        XCloseDisplay(state.display);
        state.display = NULL;
    }
#endif
}

int arcade_update(void)
{
#ifdef _WIN32
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            state.running = 0;
            return 0;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#else
    XEvent event;
    while (XPending(state.display))
    {
        XNextEvent(state.display, &event);
        if (event.type == ClientMessage && event.xclient.data.l[0] == state.wm_delete)
        {
            state.running = 0;
            return 0;
        }
        else if (event.type == KeyPress)
        {
            KeySym keysym = XLookupKeysym(&event.xkey, 0);
            key_states[keysym & 0xFF] = 1;
        }
        else if (event.type == KeyRelease)
        {
            KeySym keysym = XLookupKeysym(&event.xkey, 0);
            key_states[keysym & 0xFF] = 0;
        }
    }
#endif
    global_frame_counter++;
    return 1;
}

int arcade_running(void)
{
    return state.running;
}

void arcade_set_running(int value)
{
    state.running = value;
}

void arcade_sleep(unsigned int milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000); /* Convert ms to microseconds */
#endif
}

float arcade_delta_time(void)
{
    static double last_time = 0.0;                   /* Store time of the last frame */
    double current_time = 0.0;                       /* Current frame time */
    float delta_time;

#ifdef _WIN32
    /* Use high-resolution performance counter for precise timing on Windows */
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);        /* Get ticks per second */
    QueryPerformanceCounter(&counter);            /* Get current tick count */
    current_time = (double)counter.QuadPart / frequency.QuadPart;
#else
    struct timespec ts;
    int clock_result = -1;
#ifdef CLOCK_MONOTONIC
    /* Prefer CLOCK_MONOTONIC for consistent timing (not affected by system clock changes) */
    clock_result = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (clock_result == 0)
    {
        current_time = (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
    }
#else
#ifdef CLOCK_REALTIME
    /* Fallback to CLOCK_REALTIME (less reliable due to system clock adjustments) */
    clock_result = clock_gettime(CLOCK_REALTIME, &ts);
    if (clock_result == 0)
    {
        current_time = (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
        fprintf(stderr, "Warning: Using CLOCK_REALTIME (less reliable for timing)\n");
    }
#else
    /* Fallback to gettimeofday if no POSIX clocks are available */
    fprintf(stderr, "Warning: No POSIX clocks available, falling back to gettimeofday\n");
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == 0)
    {
        current_time = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
    }
    else
    {
        fprintf(stderr, "Error: gettimeofday failed\n");
        current_time = 0.0;
    }
#endif
#endif
    if (clock_result != 0 && current_time == 0.0)
    {
        fprintf(stderr, "Warning: clock_gettime failed, falling back to gettimeofday\n");
        struct timeval tv;
        if (gettimeofday(&tv, NULL) == 0)
        {
            current_time = (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
        }
        else
        {
            fprintf(stderr, "Error: gettimeofday failed\n");
            current_time = 0.0;
        }
    }
#endif

    /* If first call or invalid time, initialize last_time and return 0 */
    if (last_time == 0.0 || current_time == 0.0)
    {
        last_time = current_time;
        return 0.0f;
    }

    /* Calculate delta time and update last_time */
    delta_time = (float)(current_time - last_time);
    last_time = current_time;

    /* Clamp delta_time to avoid large jumps (e.g., during lag) */
    if (delta_time > 0.1f)
        delta_time = 0.1f;
    if (delta_time < 0.0f)
        delta_time = 0.0f;

    return delta_time;
}

/* =========================================================================
 * Input Handling
 * ========================================================================= */

int arcade_key_pressed(unsigned int key_val)
{
#ifdef _WIN32
    int vk = arcade_to_vk(key_val);
    return key_states[vk] ? 2 : 0;
#else
    return key_states[key_val & 0xFF] ? 2 : 0;
#endif
}

int arcade_key_pressed_once(unsigned int key_val)
{
#ifdef _WIN32
    int vk = arcade_to_vk(key_val);
    int current = key_states[vk];
    int last = last_key_states[vk];
    last_key_states[vk] = current;
    return current == 1 && last == 0 ? 2 : 0;
#else
    int key = key_val & 0xFF;
    int current = key_states[key];
    int last = last_key_states[key];
    last_key_states[key] = current;
    return current == 1 && last == 0 ? 2 : 0;
#endif
}

void arcade_clear_keys(void)
{
    memset(key_states, 0, sizeof(key_states));
    memset(last_key_states, 0, sizeof(last_key_states));
}

/* =========================================================================
 * Sprite Management
 * ========================================================================= */

static void move_sprite(ArcadeSprite *sprite, float gravity, int window_height)
{
    if (!sprite || !sprite->active)
        return;
    sprite->vy += gravity;
    sprite->y += sprite->vy;
    sprite->x += sprite->vx;
    if (sprite->y < 0.0f)
    {
        sprite->y = 0.0f;
        sprite->vy = 0.0f;
    }
    if (sprite->y > window_height - sprite->height)
    {
        sprite->y = window_height - sprite->height;
        sprite->vy = 0.0f;
    }
}

void arcade_move_sprite(ArcadeSprite *sprite, float gravity, int window_height)
{
    if (sprite)
        move_sprite(sprite, gravity, window_height);
}

void arcade_move_image_sprite(ArcadeImageSprite *sprite, float gravity, int window_height)
{
    if (!sprite || !sprite->active)
        return;
    sprite->vy += gravity;
    sprite->y += sprite->vy;
    sprite->x += sprite->vx;
    if (sprite->y < 0.0f)
    {
        sprite->y = 0.0f;
        sprite->vy = 0.0f;
    }
    if (sprite->y > window_height - sprite->height)
    {
        sprite->y = window_height - sprite->height;
        sprite->vy = 0.0f;
    }
}

static int check_collision(const ArcadeSprite *a, const ArcadeSprite *b)
{
    if (!a || !b || !a->active || !b->active)
        return 0;
    return (a->x < b->x + b->width &&
            a->x + a->width > b->x &&
            a->y < b->y + b->height &&
            a->y + a->height > b->y);
}

int arcade_check_collision(ArcadeSprite *a, ArcadeSprite *b)
{
    if (!a || !b)
        return 0;
    return check_collision(a, b);
}

int arcade_check_image_collision(ArcadeImageSprite *a, ArcadeImageSprite *b)
{
    if (!a || !b || !a->active || !b->active)
        return 0;
    return (a->x < b->x + b->width &&
            a->x + a->width > b->x &&
            a->y < b->y + b->height &&
            a->y + a->height > b->y);
}

static int load_image_sprite(ArcadeImageSprite *sprite, const char *filename, int target_width, int target_height)
{
    if (!sprite || !filename)
        return 1;
    int width, height, channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);
    if (!data)
    {
        fprintf(stderr, "Cannot load %s\n", filename);
        return 1;
    }
    unsigned char *resized_data = (unsigned char *)malloc(target_width * target_height * 4);
    if (!resized_data)
    {
        stbi_image_free(data);
        return 1;
    }
    if (stbir_resize_uint8_srgb(data, width, height, 0, resized_data, target_width, target_height, 0, 4) == 0)
    {
        fprintf(stderr, "Failed to resize %s to %dx%d\n", filename, target_width, target_height);
        stbi_image_free(data);
        free(resized_data);
        return 1;
    }
    sprite->image_width = target_width;
    sprite->image_height = target_height;
    sprite->pixels = malloc(target_width * target_height * sizeof(uint32_t));
    if (!sprite->pixels)
    {
        stbi_image_free(data);
        free(resized_data);
        return 1;
    }
    for (int y = 0; y < target_height; y++)
    {
        for (int x = 0; x < target_width; x++)
        {
            int idx = (y * target_width + x) * 4;
            sprite->pixels[y * target_width + x] =
                (resized_data[idx] << 16) | (resized_data[idx + 1] << 8) | resized_data[idx + 2] | (resized_data[idx + 3] << 24);
        }
    }
    stbi_image_free(data);
    free(resized_data);
    sprite->width = (float)target_width;
    sprite->height = (float)target_height;
    sprite->active = 1;
    return 0;
}

ArcadeImageSprite arcade_create_image_sprite(float x, float y, float w, float h, const char *filename)
{
    ArcadeImageSprite sprite = {
        .x = x, .y = y, .width = 0.0f, .height = 0.0f, .vx = 0.0f, .vy = 0.0f, .pixels = NULL, .image_width = 0, .image_height = 0, .active = 1};
    if (filename && load_image_sprite(&sprite, filename, (int)w, (int)h) != 0)
    {
        sprite.pixels = NULL;
    }
    return sprite;
}

void arcade_free_image_sprite(ArcadeImageSprite *sprite)
{
    if (sprite && sprite->pixels)
    {
        free(sprite->pixels);
        sprite->pixels = NULL;
        sprite->image_width = 0;
        sprite->image_height = 0;
        sprite->active = 0;
    }
}

ArcadeAnimatedSprite arcade_create_animated_sprite(float x, float y, float w, float h, const char **filenames, int frame_count, int frame_interval)
{
    ArcadeAnimatedSprite anim = {0};
    anim.frames = malloc(frame_count * sizeof(ArcadeImageSprite));
    anim.frame_count = frame_count;
    anim.frame_interval = frame_interval;
    for (int i = 0; i < frame_count; i++)
    {
        anim.frames[i] = arcade_create_image_sprite(x, y, w, h, filenames[i]);
        if (!anim.frames[i].pixels)
        {
            for (int j = 0; j < i; j++)
                arcade_free_image_sprite(&anim.frames[j]);
            free(anim.frames);
            return (ArcadeAnimatedSprite){0};
        }
    }
    anim.frames[0].active = 1;
    return anim;
}

void arcade_free_animated_sprite(ArcadeAnimatedSprite *anim)
{
    if (!anim || !anim->frames)
        return;
    for (int i = 0; i < anim->frame_count; i++)
        arcade_free_image_sprite(&anim->frames[i]);
    free(anim->frames);
    anim->frames = NULL;
    anim->frame_count = 0;
}

void arcade_move_animated_sprite(ArcadeAnimatedSprite *anim, float gravity, int window_height)
{
    if (!anim || !anim->frames[0].active)
        return;
    ArcadeImageSprite *current = &anim->frames[anim->current_frame];
    arcade_move_image_sprite(current, gravity, window_height);
    for (int i = 0; i < anim->frame_count; i++)
    {
        anim->frames[i].x = current->x;
        anim->frames[i].y = current->y;
        anim->frames[i].vx = current->vx;
        anim->frames[i].vy = current->vy;
    }
    if (++anim->frame_counter >= anim->frame_interval)
    {
        anim->current_frame = (anim->current_frame + 1) % anim->frame_count;
        anim->frame_counter = 0;
    }
}

int arcade_check_animated_collision(ArcadeAnimatedSprite *anim, ArcadeImageSprite *other)
{
    if (!anim || !other || !anim->frames[0].active || !other->active)
        return 0;
    return arcade_check_image_collision(&anim->frames[anim->current_frame], other);
}

/* =========================================================================
 * Rendering
 * ========================================================================= */

static void draw_sprite(ArcadeAnySprite *sprite, int type)
{
    if (!sprite)
        return;
    if (type == SPRITE_COLOR && sprite->sprite.active)
    {
        ArcadeSprite *s = &sprite->sprite;
        int x_start = (int)s->x;
        int y_start = (int)s->y;
        int x_end = x_start + (int)s->width;
        int y_end = y_start + (int)s->height;
        unsigned int color = s->color;
        /* Draw a solid rectangle for color-based sprites */
        for (int y = y_start; y < y_end && y < state.height; y++)
        {
            if (y < 0)
                continue;                        /* Skip pixels outside the top of the window */
            for (int x = x_start; x < x_end && x < state.width; x++)
            {
                if (x < 0)
                    continue;                    /* Skip pixels outside the left of the window */
                state.pixels[y * state.width + x] = color; /* Set pixel to sprite color */
            }
        }
    }
    else if (type == SPRITE_IMAGE && sprite->image_sprite.active && sprite->image_sprite.pixels)
    {
        ArcadeImageSprite *s = &sprite->image_sprite;
        int x_start = (int)s->x;
        int y_start = (int)s->y;
        int x_end = x_start + (int)s->width;
        int y_end = y_start + (int)s->height;
        int iw = s->image_width;
        int ih = s->image_height;
        /* Draw image-based sprite with alpha blending */
        for (int y = y_start, sy = 0; y < y_end && y < state.height && sy < ih; y++, sy++)
        {
            if (y < 0)
                continue;                        /* Skip pixels outside the top of the window */
            for (int x = x_start, sx = 0; x < x_end && x < state.width && sx < iw; x++, sx++)
            {
                if (x < 0)
                    continue;                    /* Skip pixels outside the left of the window */
                uint32_t pixel = s->pixels[sy * iw + sx];
                if ((pixel >> 24) > 0)           /* Only draw if pixel is not fully transparent */
                {
                    state.pixels[y * state.width + x] = pixel; /* Copy pixel to buffer */
                }
            }
        }
    }
}

void arcade_render_scene(ArcadeAnySprite *sprites, int count, int *types)
{
    for (int i = 0; i < state.width * state.height; i++)
    {
        state.pixels[i] = state.bg_color;
    }
    for (int i = 0; i < count; i++)
    {
        draw_sprite(&sprites[i], types[i]);
    }
#ifdef _WIN32
    HDC memDC = CreateCompatibleDC(state.hdc);
    SelectObject(memDC, state.hbitmap);
    BitBlt(state.hdc, 0, 0, state.width, state.height, memDC, 0, 0, SRCCOPY);
    DeleteDC(memDC);
#else
    XPutImage(state.display, state.window, state.gc, state.image, 0, 0, 0, 0, state.width, state.height);
#endif
}

void arcade_render_text(const char *text, float x, float y, unsigned int color)
{
    if (!text)
        return;
#ifdef _WIN32
    if (!state.hfont)
    {
        fprintf(stderr, "arcade_render_text: Skipping (font=%p)\n", state.hfont);
        return;
    }
    HDC memDC = CreateCompatibleDC(state.hdc);
    SelectObject(memDC, state.hbitmap);
    SelectObject(memDC, state.hfont);
    SetTextColor(memDC, color);
    SetBkMode(memDC, TRANSPARENT);
    TextOut(memDC, (int)x, (int)y, text, strlen(text));
    BitBlt(state.hdc, 0, 0, state.width, state.height, memDC, 0, 0, SRCCOPY);
    DeleteDC(memDC);
#else
    if (!state.font)
    {
        fprintf(stderr, "arcade_render_text: Skipping (font=%p)\n", state.font);
        return;
    }
    XSetForeground(state.display, state.gc, color);
    XSetFont(state.display, state.gc, state.font->fid);
    XDrawString(state.display, state.window, state.gc, (int)x, (int)y, text, strlen(text));
    XFlush(state.display);
#endif
}

void arcade_render_text_centered(const char *text, float y, unsigned int color)
{
    if (!text)
        return;
#ifdef _WIN32
    if (!state.hfont)
        return;
    SIZE size;
    HDC memDC = CreateCompatibleDC(state.hdc);
    SelectObject(memDC, state.hfont);
    GetTextExtentPoint32(memDC, text, strlen(text), &size);
    float x = (state.width - size.cx) / 2.0f;
    arcade_render_text(text, x, y, color);
    DeleteDC(memDC);
#else
    if (!state.font)
        return;
    int text_width = XTextWidth(state.font, text, strlen(text));
    float x = (state.width - text_width) / 2.0f;
    arcade_render_text(text, x, y, color);
#endif
}

void arcade_render_text_centered_blink(const char *text, float y, unsigned int color, int blink_interval)
{
    if (!text)
        return;
    if ((global_frame_counter % (2 * blink_interval)) < blink_interval)
    {
        arcade_render_text_centered(text, y, color);
    }
}

/* =========================================================================
 * Sprite Groups
 * ========================================================================= */

void arcade_init_group(SpriteGroup *group, int capacity)
{
    group->sprites = malloc(capacity * sizeof(ArcadeAnySprite));
    group->types = malloc(capacity * sizeof(int));
    group->count = 0;
    group->capacity = capacity;
}

void arcade_add_sprite_to_group(SpriteGroup *group, ArcadeAnySprite sprite, int type)
{
    if (group->count < group->capacity)
    {
        group->sprites[group->count] = sprite;
        group->types[group->count] = type;
        group->count++;
    }
}

void arcade_add_animated_to_group(SpriteGroup *group, ArcadeAnimatedSprite *anim)
{
    if (!anim || !anim->frames[0].active)
        return;
    arcade_add_sprite_to_group(group, (ArcadeAnySprite){.image_sprite = anim->frames[anim->current_frame]}, SPRITE_IMAGE);
}

void arcade_render_group(SpriteGroup *group)
{
    arcade_render_scene(group->sprites, group->count, group->types);
}

void arcade_free_group(SpriteGroup *group)
{
    free(group->sprites);
    free(group->types);
    group->count = 0;
    group->capacity = 0;
}

/* =========================================================================
 * Audio
 * ========================================================================= */

int arcade_play_sound(const char *audio_file_path)
{
#ifdef _WIN32
    /* Play WAV file asynchronously using Windows API */
    return PlaySound(audio_file_path, NULL, SND_FILENAME | SND_ASYNC) ? 0 : 1;
#else
    /* Use aplay to play WAV file in the background on Linux */
    char command[256];
    snprintf(command, sizeof(command), "aplay -q %s &", audio_file_path); /* Quiet playback in background */
    return system(command); /* Execute system command and return status */
#endif
}

int arcade_stop_sound(void)
{
#ifdef _WIN32
    /* Stop any currently playing sound using Windows API */
    return PlaySound(NULL, NULL, 0) ? 0 : 1;
#else
    /* Terminate all background aplay processes on Linux */
    return system("pkill -f 'aplay -q' > /dev/null 2>&1"); /* Kill aplay processes silently */
#endif
}

/* =========================================================================
 * Image Manipulation
 * ========================================================================= */

char *arcade_flip_image(const char *input_path, int flip_type)
{
    int width, height, channels;
    unsigned char *data = stbi_load(input_path, &width, &height, &channels, 4);
    if (!data)
    {
        fprintf(stderr, "Failed to load image %s for flipping\n", input_path);
        return NULL;
    }
    unsigned char *flipped_data = (unsigned char *)malloc(width * height * 4);
    if (!flipped_data)
    {
        stbi_image_free(data);
        fprintf(stderr, "Memory allocation failed for flipped image\n");
        return NULL;
    }
    if (flip_type == 1)
    {
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int src_idx = (height - 1 - y) * width * 4 + x * 4;
                int dst_idx = y * width * 4 + x * 4;
                for (int c = 0; c < 4; c++)
                {
                    flipped_data[dst_idx + c] = data[src_idx + c];
                }
            }
        }
    }
    else
    {
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int src_idx = y * width * 4 + (width - 1 - x) * 4;
                int dst_idx = y * width * 4 + x * 4;
                for (int c = 0; c < 4; c++)
                {
                    flipped_data[dst_idx + c] = data[src_idx + c];
                }
            }
        }
    }
#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempFileName(".", "arc", 0, temp_path))
    {
        stbi_image_free(data);
        free(flipped_data);
        fprintf(stderr, "Failed to create temporary file\n");
        return NULL;
    }
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "%s.png", temp_path);
#else
    char temp_path[] = "/tmp/arcade_flip_XXXXXX";
    int fd = mkstemp(temp_path);
    if (fd == -1)
    {
        stbi_image_free(data);
        free(flipped_data);
        fprintf(stderr, "Failed to create temporary file: %s\n", strerror(errno));
        return NULL;
    }
    close(fd);
    char *full_path = malloc(strlen(temp_path) + 5);
    if (!full_path)
    {
        remove(temp_path);
        stbi_image_free(data);
        free(flipped_data);
        fprintf(stderr, "Memory allocation failed for path\n");
        return NULL;
    }
    sprintf(full_path, "%s.png", temp_path);
#endif
    if (!stbi_write_png(full_path, width, height, 4, flipped_data, width * 4))
    {
        remove(full_path);
        stbi_image_free(data);
        free(flipped_data);
#ifdef _WIN32
        fprintf(stderr, "Failed to write flipped image to %s\n", full_path);
        return NULL;
#else
        free(full_path);
        fprintf(stderr, "Failed to write flipped image to %s\n", full_path);
        return NULL;
#endif
    }
    stbi_image_free(data);
    free(flipped_data);
#ifdef _WIN32
    char *result = strdup(full_path);
    if (!result)
    {
        DeleteFile(temp_path);
        fprintf(stderr, "Failed to duplicate path\n");
        return NULL;
    }
    DeleteFile(temp_path);
    return result;
#else
    char *result = strdup(full_path);
    free(full_path);
    if (!result)
    {
        remove(temp_path);
        fprintf(stderr, "Failed to duplicate path\n");
        return NULL;
    }
    return result;
#endif
}

char *arcade_rotate_image(const char *input_path, int degrees)
{
    int width, height, channels;
    unsigned char *data = stbi_load(input_path, &width, &height, &channels, 4);
    if (!data)
    {
        fprintf(stderr, "Failed to load image %s for rotation\n", input_path);
        return NULL;
    }
    int new_width = (degrees == 90 || degrees == 270) ? height : width;
    int new_height = (degrees == 90 || degrees == 270) ? width : height;
    unsigned char *rotated_data = (unsigned char *)malloc(new_width * new_height * 4);
    if (!rotated_data)
    {
        stbi_image_free(data);
        fprintf(stderr, "Memory allocation failed for rotated image\n");
        return NULL;
    }
    for (int y = 0; y < new_height; y++)
    {
        for (int x = 0; x < new_width; x++)
        {
            int src_x, src_y;
            if (degrees == 90)
            {
                src_x = y;
                src_y = new_width - 1 - x;
            }
            else if (degrees == 180)
            {
                src_x = width - 1 - x;
                src_y = height - 1 - y;
            }
            else if (degrees == 270)
            {
                src_x = new_height - 1 - y;
                src_y = x;
            }
            else
            {
                src_x = x;
                src_y = y;
            }
            int src_idx = src_y * width * 4 + src_x * 4;
            int dst_idx = y * new_width * 4 + x * 4;
            for (int c = 0; c < 4; c++)
            {
                rotated_data[dst_idx + c] = data[src_idx + c];
            }
        }
    }
#ifdef _WIN32
    char temp_path[MAX_PATH];
    if (!GetTempFileName(".", "arc", 0, temp_path))
    {
        stbi_image_free(data);
        free(rotated_data);
        fprintf(stderr, "Failed to create temporary file\n");
        return NULL;
    }
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "%s.png", temp_path);
#else
    char temp_path[] = "/tmp/arcade_rotate_XXXXXX";
    int fd = mkstemp(temp_path);
    if (fd == -1)
    {
        stbi_image_free(data);
        free(rotated_data);
        fprintf(stderr, "Failed to create temporary file: %s\n", strerror(errno));
        return NULL;
    }
    close(fd);
    char *full_path = malloc(strlen(temp_path) + 5);
    if (!full_path)
    {
        remove(temp_path);
        stbi_image_free(data);
        free(rotated_data);
        fprintf(stderr, "Memory allocation failed for path\n");
        return NULL;
    }
    sprintf(full_path, "%s.png", temp_path);
#endif
    if (!stbi_write_png(full_path, new_width, new_height, 4, rotated_data, new_width * 4))
    {
        remove(full_path);
        stbi_image_free(data);
        free(rotated_data);
#ifdef _WIN32
        fprintf(stderr, "Failed to write rotated image to %s\n", full_path);
        return NULL;
#else
        free(full_path);
        fprintf(stderr, "Failed to write rotated image to %s\n", full_path);
        return NULL;
#endif
    }
    stbi_image_free(data);
    free(rotated_data);
#ifdef _WIN32
    char *result = strdup(full_path);
    if (!result)
    {
        DeleteFile(temp_path);
        fprintf(stderr, "Failed to duplicate path\n");
        return NULL;
    }
    DeleteFile(temp_path);
    return result;
#else
    char *result = strdup(full_path);
    free(full_path);
    if (!result)
    {
        remove(temp_path);
        fprintf(stderr, "Failed to duplicate path\n");
        return NULL;
    }
    return result;
#endif
}

#endif /* ARCADE_IMPLEMENTATION */