#ifndef CHARACTER_H
#define CHARACTER_H

#include <stdint.h>
#include <math.h>
#include "Joystick.h"
#include "LCD.h"

/**
 * @file Character.h
 * @brief Character/Sprite FSM module for interactive LCD gaming
 * 
 * This demonstrates the concept of NESTED FINITE STATE MACHINES:
 * - An INTERNAL FSM for the character (IDLE, RUNNING, JUMPING)
 * - Each state has different animation frames and physics
 * - An EXTERNAL FSM for the game states (controlled by main.c)
 * 
 * KEY CONCEPT: The character is an "object with internal state"
 * Just like the overall game has states, so can individual game objects!
 * This is a powerful pattern for game development and embedded interactive systems.
 */

// ===== CHARACTER STATE FSM =====

/**
 * @brief Character internal state machine
 * 
 * IDLE:    Standing still, no horizontal movement
 * RUNNING: Moving horizontally (left or right)
 * JUMPING: In the air, falling due to gravity
 * 
 * State transitions:
 * - IDLE -> RUNNING: when joystick X != 0
 * - RUNNING -> IDLE: when joystick X == 0
 * - ANY -> JUMPING: when jump button pressed AND character is on ground
 * - JUMPING -> IDLE/RUNNING: when character lands (y_position >= ground_y)
 */
typedef enum {
    CHAR_IDLE = 0,      // Standing still
    CHAR_RUNNING,       // Moving horizontally
    CHAR_JUMPING        // In the air
} CharacterState_t;

// Movement speed constants
#define CHAR_SPEED 4                    // Horizontal movement speed (pixels per frame)
#define CHAR_GRAVITY 1.1f               // Gravity acceleration (pixels per frame squared)
#define CHAR_JUMP_VELOCITY -10.0f       // Initial jump velocity (negative = upward)
#define CHAR_MAX_FALL_VELOCITY 15.0f    // Terminal velocity (max falling speed)

// Character dimensions
#define CHAR_WIDTH 32                   // Character sprite width (8 pixels * 4x scale)
#define CHAR_HEIGHT 32                  // Character sprite height (8 pixels * 4x scale)

// Ground position
#define GROUND_Y 200                    // Y-coordinate of the ground

// Screen boundaries
#define SCREEN_MIN_X 20
#define SCREEN_MAX_X 220

// ===== CHARACTER STRUCTURE =====

/**
 * @struct Character_t
 * @brief Represents a character/sprite with FSM state and animation
 * 
 * This structure contains:
 * - Position (x, y)
 * - Velocity (for jump physics)
 * - Internal FSM state (IDLE, RUNNING, JUMPING)
 * - Animation frame counter
 * - Direction the character is facing
 */
typedef struct {
    // Position
    int16_t x;                      // X position on screen
    int16_t y;                      // Y position on screen
    
    // Physics
    float velocity_y;               // Vertical velocity (for jumping/falling)
    
    // Internal FSM state
    CharacterState_t state;         // Current character state
    CharacterState_t prev_state;    // Previous state (to detect transitions)
    
    // Animation
    uint8_t animation_frame;        // Current animation frame (0-3)
    uint8_t frame_counter;          // Counter for frame timing
    
    // Direction
    int8_t direction;               // -1 = left, 0 = idle, 1 = right
    
} Character_t;

// ===== CHARACTER FUNCTION PROTOTYPES =====

/**
 * @brief Initialize character at starting position
 * @param character: Pointer to character structure
 */
void Character_Init(Character_t* character);

/**
 * @brief Update character state machine based on input
 * 
 * This is the main character FSM logic:
 * - Reads joystick input for movement
 * - Reads jump button input
 * - Updates internal state (IDLE -> RUNNING -> JUMPING, etc)
 * - Applies gravity during jumping
 * - Handles ground collision
 * 
 * @param character: Pointer to character structure
 * @param joy: Joystick input data
 * @param jump_button: 1 if jump button pressed, 0 otherwise
 */
void Character_Update(Character_t* character, Joystick_t* joy, uint8_t jump_button);

/**
 * @brief Draw character on LCD at current position with current animation
 * 
 * Different sprites/patterns are drawn based on character state:
 * - IDLE: Simple standing figure (no animation)
 * - RUNNING: Alternating leg animation
 * - JUMPING: Arms up, legs in running position
 * 
 * @param character: Pointer to character structure
 */
void Character_Draw(Character_t* character);

/**
 * @brief Draw the ground line and background
 * Simple visual reference for where the character can stand
 */
void Character_DrawGround(void);

/**
 * @brief Get character state as string (for debug display)
 * @param character: Pointer to character structure
 * @return String representation of current state
 */
const char* Character_GetStateName(Character_t* character);

#endif // CHARACTER_H
