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
 * @brief WALKING animation frame 1
 * 8x8 pixel sprite
 */
const uint8_t CharacterWALK1[8][8] = {
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
 * @brief WALKING animation frame 2
 * 8x8 pixel sprite
 */
const uint8_t CharacterWALK2[8][8] = {
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
 * @brief DASHING animation - Speed lines around character
 * 8x8 pixel sprite showing dashing/moving fast
 */
const uint8_t CharacterDASHING[8][8] = {
    {255, 0, 0, 255, 255, 0, 0, 255},
    {0, 255, 255, 255, 255, 255, 255, 0},
    {0, 255, 0, 0, 0, 0, 255, 0},
    {255, 255, 0, 255, 255, 0, 255, 255},
    {255, 255, 0, 255, 255, 0, 255, 255},
    {0, 255, 0, 0, 0, 0, 255, 0},
    {0, 255, 255, 255, 255, 255, 255, 0},
    {255, 0, 0, 255, 255, 0, 0, 255}
};

// ===== CHARACTER STATE NAMES (for debug display) =====
const char* character_state_names[] = {
    "IDLE",
    "WLK",
    "DSH"
};

// ===== CHARACTER FUNCTION IMPLEMENTATIONS =====

/**
 * @brief Initialize character at starting position
 * Sets up the character structure with default values
 */
void Character_Init(Character_t* character) {
    character->x = 120;                 // Start in middle of screen
    character->y = 120;                 // Start in middle of screen
    character->state = CHAR_IDLE;       // Start in IDLE state
    character->prev_state = CHAR_IDLE;
    character->animation_frame = 0;     // Animation starts at frame 0
    character->frame_counter = 0;
    character->move_x = 0;              // Not moving initially
    character->move_y = 0;
    character->dash_counter = 0;        // Not dashing initially
}

/**
 * @brief Update character state machine
 * 
 * This implements the character's INTERNAL FSM:
 * 1. Read joystick input
 * 2. Update dash state if active
 * 3. Update position based on movement and dash state
 * 4. Update FSM state based on input
 * 5. Update animation frame
 * 
 * KEY CONCEPT: Button presses trigger dash state (temporary high-speed state)
 */
void Character_Update(Character_t* character, Joystick_t* joy, uint8_t dash_pressed) {
    
    // ------- STEP 1: Read input -------
    // Determine movement direction from joystick direction (N/S/E/W etc.)
    int8_t input_x = 0;
    int8_t input_y = 0;
    
    switch (joy->direction) {
        case CENTRE:
            input_x = 0;
            input_y = 0;
            break;
        case N:
            input_x = 0;
            input_y = -1;           // North (up)
            break;
        case NE:
            input_x = 1;
            input_y = -1;           // Northeast
            break;
        case E:
            input_x = 1;
            input_y = 0;            // East (right)
            break;
        case SE:
            input_x = 1;
            input_y = 1;            // Southeast
            break;
        case S:
            input_x = 0;
            input_y = 1;            // South (down)
            break;
        case SW:
            input_x = -1;
            input_y = 1;            // Southwest
            break;
        case W:
            input_x = -1;
            input_y = 0;            // West (left)
            break;
        case NW:
            input_x = -1;
            input_y = -1;           // Northwest
            break;
    }
    
    character->move_x = input_x;
    character->move_y = input_y;
    
    // ------- STEP 2: Handle dash trigger -------
    if (dash_pressed && character->dash_counter == 0) {
        // Dash button pressed and not already dashing - start a new dash
        character->dash_counter = CHAR_DASH_DURATION;
    }
    
    // ------- STEP 3: Determine movement speed -------
    uint8_t current_speed = CHAR_SPEED;
    if (character->dash_counter > 0) {
        current_speed = CHAR_DASH_SPEED;
        character->dash_counter--;
    }
    
    // ------- STEP 4: Calculate and update position -------
    int16_t new_x = character->x + (input_x * current_speed);
    int16_t new_y = character->y + (input_y * current_speed);
    
    // Keep on screen
    if (new_x < SCREEN_MIN_X) new_x = SCREEN_MIN_X;
    if (new_x > SCREEN_MAX_X) new_x = SCREEN_MAX_X;
    if (new_y < SCREEN_MIN_Y) new_y = SCREEN_MIN_Y;
    if (new_y > SCREEN_MAX_Y) new_y = SCREEN_MAX_Y;
    
    // Update position
    if (input_x != 0 || input_y != 0) {
        character->x = new_x;
        character->y = new_y;
    }
    
    // ------- STEP 5: Update FSM state -------
    character->prev_state = character->state;
    
    uint8_t is_moving = (input_x != 0 || input_y != 0);
    
    // State machine transitions
    switch (character->state) {
        case CHAR_IDLE:
            if (is_moving) {
                character->state = CHAR_WALKING;
            } else if (character->dash_counter > 0) {
                character->state = CHAR_DASHING;
            }
            break;
        
        case CHAR_WALKING:
            if (!is_moving && character->dash_counter == 0) {
                character->state = CHAR_IDLE;
            } else if (character->dash_counter > 0) {
                character->state = CHAR_DASHING;
            }
            break;
        
        case CHAR_DASHING:
            if (character->dash_counter == 0) {
                // Dash ended - return to previous state
                if (is_moving) {
                    character->state = CHAR_WALKING;
                } else {
                    character->state = CHAR_IDLE;
                }
            }
            break;
    }
    
    // ------- STEP 6: Update animation frame -------
    character->frame_counter++;
    
    // Every 10 frames, update animation frame for WALKING state
    if (character->frame_counter >= 10) {
        character->frame_counter = 0;
        // Cycle between 2 walking frames
        if (character->state == CHAR_WALKING) {
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
 * - WALKING: Alternating walking animation
 * - DASHING: Speed lines showing fast movement
 * 
 * The sprite is drawn with scaling for better visibility.
 */
void Character_Draw(Character_t* character) {
    
    int16_t x_pos = character->x - CHAR_WIDTH/2;
    int16_t y_pos = character->y - CHAR_HEIGHT/2;
    
    switch (character->state) {
        case CHAR_IDLE:
            LCD_Draw_Sprite_Colour_Scaled(
                x_pos, y_pos,
                8, 8,
                (uint8_t*)CharacterIDLE,
                5,                              // Orange color
                4                               // 4x scale
            );
            break;
        
        case CHAR_WALKING:
            // Alternate between 2 walking sprites
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite_Colour_Scaled(
                    x_pos, y_pos,
                    8, 8,
                    (uint8_t*)CharacterWALK1,
                    5,                          // Orange color
                    4                           // 4x scale
                );
            } else {
                LCD_Draw_Sprite_Colour_Scaled(
                    x_pos, y_pos,
                    8, 8,
                    (uint8_t*)CharacterWALK2,
                    5,                          // Orange color
                    4                           // 4x scale
                );
            }
            break;
        
        case CHAR_DASHING:
            LCD_Draw_Sprite_Colour_Scaled(
                x_pos, y_pos,
                8, 8,
                (uint8_t*)CharacterDASHING,
                5,                              // Orange color
                4                               // 4x scale
            );
            break;
    }
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
