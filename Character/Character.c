#include "Character.h"

// ===== CHARACTER SPRITE/ANIMATION DATA =====

/**
 * @brief IDLE animation - Simple standing figure
 * 8x8 pixel sprite showing character standing
 */
const uint8_t CharacterIDLE[8][8] = {
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 0, 0, 255, 255, 0, 0, 255},
    {255, 0, 0, 0, 0, 0, 0, 255},
    {255, 0, 255, 255, 255, 255, 0, 255},
    {255, 0, 255, 255, 255, 255, 0, 255},
    {255, 255, 0, 255, 255, 0, 255, 255},
    {255, 255, 0, 255, 255, 0, 255, 255}
};

/**
 * @brief RUNNING animation frame 1 - Right leg back, left leg forward
 * 8x8 pixel sprite
 */
const uint8_t CharacterRUN1[8][8] = {
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 0, 0, 255, 255, 0, 0, 255},
    {255, 0, 0, 0, 0, 0, 0, 255},
    {255, 0, 255, 255, 255, 0, 0, 255},
    {255, 0, 255, 255, 0, 255, 255, 255},
    {255, 255, 0, 0, 255, 0, 255, 255},
    {255, 255, 0, 255, 255, 255, 0, 255}
};

/**
 * @brief RUNNING animation frame 2 - Left leg back, right leg forward (alternate)
 * 8x8 pixel sprite
 */
const uint8_t CharacterRUN2[8][8] = {
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 0, 0, 255, 255, 0, 0, 255},
    {255, 0, 0, 0, 0, 0, 0, 255},
    {255, 0, 0, 255, 255, 255, 255, 255},
    {255, 0, 255, 255, 0, 255, 255, 255},
    {255, 255, 0, 255, 255, 0, 0, 255},
    {255, 255, 255, 0, 255, 255, 0, 255}
};

/**
 * @brief JUMPING animation - User looking up, legs extended
 * 8x8 pixel sprite
 */
const uint8_t CharacterJUMP[8][8] = {
    {255, 255, 0, 0, 0, 0, 255, 255},
    {255, 255, 0, 255, 255, 0, 255, 255},
    {255, 0, 0, 255, 255, 0, 0, 255},
    {255, 0, 0, 0, 0, 0, 0, 255},
    {255, 0, 255, 255, 255, 255, 0, 255},
    {255, 0, 255, 255, 255, 255, 0, 255},
    {255, 255, 0, 0, 255, 0, 0, 255},
    {255, 255, 0, 0, 255, 0, 0, 255}
};

// ===== CHARACTER STATE NAMES (for debug display) =====
const char* character_state_names[] = {
    "IDLE",
    "RUNNING",
    "JUMPING"
};

// ===== CHARACTER FUNCTION IMPLEMENTATIONS =====

/**
 * @brief Initialize character at starting position
 * Sets up the character structure with default values
 */
void Character_Init(Character_t* character) {
    character->x = 120;                 // Start in middle of screen
    character->y = GROUND_Y;            // Start on ground
    character->velocity_y = 0.0f;       // No vertical velocity initially
    character->state = CHAR_IDLE;       // Start in IDLE state
    character->prev_state = CHAR_IDLE;
    character->animation_frame = 0;     // Animation starts at frame 0
    character->frame_counter = 0;
    character->direction = 0;           // Not moving horizontally initially
}

/**
 * @brief Update character state machine
 * 
 * This implements the character's INTERNAL FSM:
 * 1. Read input (joystick direction, jump button)
 * 2. Apply physics (gravity, velocity updates)
 * 3. Perform state transitions based on conditions
 * 4. Update animation frame
 * 5. Handle ground collision
 * 
 * KEY CONCEPT: This is a NESTED FSM - the character has its own state machine
 * that runs every frame, completely independent of the main game FSM.
 */
void Character_Update(Character_t* character, Joystick_t* joy, uint8_t jump_button) {
    
    // ------- STEP 1: Read input -------
    // Determine horizontal movement direction from joystick
    int8_t input_direction = 0;
    if (joy->coord_mapped.x < -0.3f) {
        input_direction = -1;           // Left
    } else if (joy->coord_mapped.x > 0.3f) {
        input_direction = 1;            // Right
    }
    
    // ------- STEP 2: Horizontal movement -------
    if (input_direction != 0) {
        character->x += (input_direction * CHAR_SPEED);
        character->direction = input_direction;
    }
    
    // Keep character on screen horizontally
    if (character->x < SCREEN_MIN_X) character->x = SCREEN_MIN_X;
    if (character->x > SCREEN_MAX_X) character->x = SCREEN_MAX_X;
    
    // ------- STEP 3: Jump button handling -------
    // Check if we should jump (only when on ground and button pressed)
    // We do this BEFORE applying gravity so the jump velocity takes effect
    int8_t is_on_ground = (character->y >= GROUND_Y) ? 1 : 0;
    
    if (jump_button && is_on_ground) {
        // Start jump - set upward velocity
        character->velocity_y = CHAR_JUMP_VELOCITY;
        // Force character slightly above ground to ensure !is_on_ground next check
        character->y = GROUND_Y - 1;
    }
    
    // ------- STEP 4: Apply gravity and vertical motion -------
    // Apply gravity if we're in the air
    if (character->y < GROUND_Y) {
        // In air - apply gravity
        character->velocity_y += CHAR_GRAVITY;
        
        // Cap falling speed at terminal velocity
        if (character->velocity_y > CHAR_MAX_FALL_VELOCITY) {
            character->velocity_y = CHAR_MAX_FALL_VELOCITY;
        }
        
        // Update vertical position
        character->y += (int16_t)character->velocity_y;
        
        // Clamp to ground or above
        if (character->y > GROUND_Y) {
            character->y = GROUND_Y;
        }
    } else {
        // On ground - reset vertical velocity
        character->velocity_y = 0.0f;
        character->y = GROUND_Y;
    }
    
    // ------- STEP 5: Update internal FSM state -------
    // (Previous state stored for detecting transitions)
    character->prev_state = character->state;
    
    // Recalculate ground state for FSM transitions
    is_on_ground = (character->y >= GROUND_Y) ? 1 : 0;
    
    // State machine transitions
    switch (character->state) {
        case CHAR_IDLE:
            // IDLE -> RUNNING: when character starts moving
            if (input_direction != 0) {
                character->state = CHAR_RUNNING;
            }
            // IDLE -> JUMPING: when character leaves ground
            if (!is_on_ground) {
                character->state = CHAR_JUMPING;
            }
            break;
        
        case CHAR_RUNNING:
            // RUNNING -> IDLE: when joystick returns to center
            if (input_direction == 0 && is_on_ground) {
                character->state = CHAR_IDLE;
            }
            // RUNNING -> JUMPING: when character leaves ground
            if (!is_on_ground) {
                character->state = CHAR_JUMPING;
            }
            break;
        
        case CHAR_JUMPING:
            // JUMPING -> IDLE/RUNNING: when character lands
            if (is_on_ground) {
                // Determine if we should go to IDLE or RUNNING based on current input
                character->state = (input_direction != 0) ? CHAR_RUNNING : CHAR_IDLE;
            }
            break;
    }
    
    // ------- STEP 7: Update animation frame -------
    character->frame_counter++;
    
    // Every 8 frames, update animation frame for RUNNING state
    if (character->frame_counter >= 8) {
        character->frame_counter = 0;
        // Cycle between 2 running frames
        if (character->state == CHAR_RUNNING) {
            character->animation_frame = (character->animation_frame + 1) % 2;
        } else {
            character->animation_frame = 0;  // Reset frame for other states
        }
    }
}

/**
 * @brief Draw character on LCD at current position
 * 
 * Different sprites are drawn based on character state:
 * - IDLE: Standing still sprite
 * - RUNNING: Alternating running animation
 * - JUMPING: Jumping sprite (arms up)
 * 
 * The sprite is drawn with scaling for better visibility.
 */
void Character_Draw(Character_t* character) {
    
    // Choose which sprite to draw based on character state
    const uint8_t* sprite = (const uint8_t*)CharacterIDLE;  // Default
    
    switch (character->state) {
        case CHAR_IDLE:
            sprite = (const uint8_t*)CharacterIDLE;
            break;
        
        case CHAR_RUNNING:
            // Alternate between 2 running sprites
            sprite = (character->animation_frame == 0) 
                ? (const uint8_t*)CharacterRUN1 
                : (const uint8_t*)CharacterRUN2;
            break;
        
        case CHAR_JUMPING:
            sprite = (const uint8_t*)CharacterJUMP;
            break;
    }
    
    // Draw sprite at character position with scaling for visibility
    // Color 5 = orange (good contrast on black background)
    LCD_Draw_Sprite_Colour_Scaled(
        character->x - CHAR_WIDTH/2,
        character->y - CHAR_HEIGHT/2,
        8,                              // Sprite is 8x8
        8,
        (uint8_t*)sprite,
        5,                              // Orange color
        4                               // 4x scale (upsize to 32x32)
    );
}

/**
 * @brief Draw the ground and game environment
 * 
 * Creates a simple Mario-like scene:
 * - Horizontal line for the ground
 * - Some simple background elements
 */
void Character_DrawGround(void) {
    // Draw ground line
    LCD_Draw_Line(
        0, GROUND_Y + CHAR_HEIGHT/2,
        240, GROUND_Y + CHAR_HEIGHT/2,
        2  // Green color (from LCD color palette)
    );
    
    // Draw simple platform/ground block pattern
    // Draw a few simple rectangular blocks under the ground line for decoration
    LCD_Draw_Rect(30, GROUND_Y + CHAR_HEIGHT/2 + 5, 30, 15, 3, 1);    // Blue rectangle
    LCD_Draw_Rect(150, GROUND_Y + CHAR_HEIGHT/2 + 5, 30, 15, 3, 1);   // Blue rectangle
}

/**
 * @brief Get character state name as string
 * Useful for debug display on LCD
 */
const char* Character_GetStateName(Character_t* character) {
    if (character->state < 3) {
        return character_state_names[character->state];
    }
    return "UNKNOWN";
}
