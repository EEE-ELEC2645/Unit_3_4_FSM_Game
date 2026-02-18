#ifndef CHARACTER_H
#define CHARACTER_H

#include <stdint.h>
#include <math.h>
#include "Joystick.h"
#include "LCD.h"

/**
 * @file Character.h
 * @brief Character/Sprite FSM module
 * 
 * This demonstrates:
 * - An INTERNAL FSM for the character (IDLE, WALKING, DASHING)
 * - State transitions based on joystick input and button presses
 * - Simple animation and movement system
 * 
 * KEY CONCEPT: The character is an "object with internal state"
 * Button presses trigger dash state, which returns to idle/walking after duration.
 */

// ===== CHARACTER STATE FSM =====

/**
 * @brief Character internal state machine
 * 
 * IDLE:    Standing still, no movement
 * WALKING: Moving around in any direction
 * DASHING: Fast movement in current direction (temporary state)
 * 
 * State transitions:
 * - IDLE -> WALKING: when joystick moves
 * - WALKING -> IDLE: when joystick returns to center
 * - IDLE/WALKING -> DASHING: when button pressed
 * - DASHING -> IDLE/WALKING: after dash duration ends
 */
typedef enum {
    CHAR_IDLE = 0,      // Standing still
    CHAR_WALKING,       // Moving around
    CHAR_DASHING       // Fast movement (temporary)
} CharacterState_t;

// Movement speed constants
#define CHAR_SPEED 2                    // Normal movement speed (pixels per frame)
#define CHAR_DASH_SPEED 6               // Dash speed (pixels per frame)
#define CHAR_DASH_DURATION 20           // Dash duration in frames

// Character dimensions
#define CHAR_WIDTH 32                   // Character sprite width (8 pixels * 4x scale)
#define CHAR_HEIGHT 32                  // Character sprite height (8 pixels * 4x scale)

// Screen boundaries
#define SCREEN_MIN_X 10
#define SCREEN_MAX_X 230
#define SCREEN_MIN_Y 10
#define SCREEN_MAX_Y 230

// ===== CHARACTER STRUCTURE =====

/**
 * @struct Character_t
 * @brief Represents a character/sprite with FSM state
 * 
 * This structure contains:
 * - Position (x, y)
 * - Internal FSM state (IDLE, WALKING, DASHING)
 * - Animation frame counter
 * - Movement direction
 * - Dash counter for duration
 */
typedef struct {
    // Position
    int16_t x;                      // X position on screen (center)
    int16_t y;                      // Y position on screen (center)
    
    // Internal FSM state
    CharacterState_t state;         // Current character state
    CharacterState_t prev_state;    // Previous state (to detect transitions)
    
    // Animation
    uint8_t animation_frame;        // Current animation frame (0-1)
    uint8_t frame_counter;          // Counter for frame timing
    
    // Movement
    int8_t move_x;                  // Current move direction X (-1, 0, or 1)
    int8_t move_y;                  // Current move direction Y (-1, 0, or 1)
    
    // Dash state
    uint8_t dash_counter;           // Frames remaining in dash state
} Character_t;

// ===== CHARACTER FUNCTION PROTOTYPES =====

/**
 * @brief Initialize character at starting position
 * @param character: Pointer to character structure
 */
void Character_Init(Character_t* character);

/**
 * @brief Update character state machine
 * @param character: Pointer to character structure
 * @param joy: Pointer to joystick state
 * @param dash_pressed: 1 if dash button was pressed, 0 otherwise
 */
void Character_Update(Character_t* character, Joystick_t* joy, uint8_t dash_pressed);

/**
 * @brief Draw character on LCD
 * @param character: Pointer to character structure
 */
void Character_Draw(Character_t* character);

/**
 * @brief Get character state name as string
 * @param character: Pointer to character structure
 * @return State name string
 */
const char* Character_GetStateName(Character_t* character);

#endif // CHARACTER_H
