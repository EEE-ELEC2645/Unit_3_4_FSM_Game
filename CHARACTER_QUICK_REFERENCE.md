# Character FSM with Dash Ability - Quick Reference

## Input Controls
```
┌─────────────────────────────────────┐
│  Joystick (all directions)          │  → Move character (2 px/frame)
├─────────────────────────────────────┤
│  Button Press (BTN3 / PC3)          │  → Trigger dash (6 px/frame, 20 frames)
└─────────────────────────────────────┘
```

## State Transitions

```
                    ┌──────────────┐
                    │  INPUT or    │
                    │  DASH BTN?   │
                    └──────┬───────┘
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
   ┌────▼────┐      ┌─────▼─────┐      ┌────▼──────┐
   │  IDLE   │◄─────│  DASHING  │      │  WALKING  │
   └────┬────┘      └─────┬─────┘      └────┬──────┘
        │ (joystick)       │ (20 frame timer) │
        │                  │                  │
        └──────────┬───────┴──────────────────┘
                   │
            (dash or input ends)
```

## State Animations

### IDLE
```
   ◎◎
   ││
  ╱╲╱╲
```
No input

### WALKING (alternating)
```
Frame 1:         Frame 2:
   ◎◎            ◎◎
   ││            ││
  ╱║ ║           ║ ╲╱
   ║ ╲           ╱ ║
```
Moving with joystick

### DASHING (speed lines)
```
   ≫≫
  →◎◎←
   ≪≪
```
Button pressed (20 frames duration)

## Dash System

```c
typedef struct {
    int16_t x, y;                 // Position
    CharacterState_t state;       // IDLE, WALKING, or DASHING
    uint8_t animation_frame;      // Animation frame (0-1 for walking)
    int8_t move_x, move_y;        // Direction (-1, 0, or 1)
    uint8_t dash_counter;         // Frames remaining in dash (0-20)
} Character_t;
```

### Dash Timer Mechanics
```c
// When button is pressed:
if (dash_pressed && character->dash_counter == 0) {
    character->dash_counter = CHAR_DASH_DURATION;  // 20 frames
}

// Each frame:
if (character->dash_counter > 0) {
    current_speed = CHAR_DASH_SPEED;  // 6 px/frame (3x faster)
    character->dash_counter--;         // Decrement timer
}

// When timer reaches 0, FSM returns to IDLE or WALKING
```

## Button Interrupt Handling

```c
// Global flag set by interrupt
volatile uint8_t dash_button_pressed = 0;
volatile uint32_t dash_button_last_interrupt_time = 0;

// Called when BTN3 is pressed (with debouncing)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == BTN3_Pin) {
        // Ignore presses within 200ms (debounce)
        if ((HAL_GetTick() - dash_button_last_interrupt_time) > 200) {
            dash_button_last_interrupt_time = HAL_GetTick();
            dash_button_pressed = 1;
        }
    }
}
```

## File Map

```
Unit_3_4_FSM_Game/
├── Character/
│   ├── Character.h          ← FSM definitions, constants
│   └── Character.c          ← Sprites, FSM logic, movement
│
├── Core/Src/
│   ├── main.c               ← update_character(), render_game(), interrupt
│   └── gpio.c               ← GPIO/EXTI configuration
│
└── CMakeLists.txt           ← Build config
```

## Important Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| `CHAR_SPEED` | 2 | Normal movement speed (px/frame) |
| `CHAR_DASH_SPEED` | 6 | Dash movement speed (px/frame) |
| `CHAR_DASH_DURATION` | 20 | Dash duration (frames) |
| `CHAR_WIDTH` | 32 | Character sprite width (8px scaled 4x) |
| `CHAR_HEIGHT` | 32 | Character sprite height (8px scaled 4x) |
| `SCREEN_MIN_X` | 10 | Left boundary |
| `SCREEN_MAX_X` | 230 | Right boundary |
| `SCREEN_MIN_Y` | 10 | Top boundary |
| `SCREEN_MAX_Y` | 230 | Bottom boundary |
| `DEBOUNCE_DELAY` | 200 | Button debounce time (ms) |

## Key Functions

### Character Module Functions
- `Character_Init(char*)` - Initialize character at spawn point
- `Character_Update(char*, joystick, dash_pressed)` - Update FSM (call each frame)
- `Character_Draw(char*)` - Draw character sprite on LCD
- `Character_GetStateName(char*)` - Get state name string for debug

### Main Loop Integration (Update/Render Pattern)
```c
// In main:
Character_t game_character;
volatile uint8_t dash_button_pressed = 0;

Character_Init(&game_character);

// In main loop (30ms per frame):
Joystick_Read(&joystick_cfg, &joystick_data);

// UPDATE: Game logic only
update_character(&joystick_data);
    // Checks if dash_button_pressed is set
    // Calls: Character_Update(&game_character, joy, dash_button_pressed)
    // Clears dash_button_pressed flag
    // Updates FSM, dash timer, animations

// RENDER: Drawing only
render_game();
    // Clears screen buffer
    // Draws character sprite
    // Displays debug info (state, position)
    // Refreshes LCD
```

## State Update Flow

### Each Frame (in Character_Update):
1. **Read joystick input** (X and Y axes)
2. **Check dash trigger** - if button pressed and not already dashing:
   - Set `dash_counter = 20` (frames)
3. **Choose movement speed**:
   - If dashing: `speed = 6` px/frame
   - Else: `speed = 2` px/frame
   - Decrement dash_counter if active
4. **Calculate new position**:
   - `new_x = x + (input_x * speed)`
   - `new_y = y + (input_y * speed)`
5. **Clamp to screen boundaries** (keep character on screen)
6. **Update position** if input detected
7. **Update FSM state**:
   - IDLE → WALKING (if input)
   - Any state → DASHING (if dash_counter > 0)
   - DASHING → IDLE/WALKING (when dash_counter reaches 0)
8. **Update animation**:
   - Advance frame counter for walk animation

## Customization Tips

### Change Movement Speed
Modify constants in Character.h:
```c
#define CHAR_SPEED 2        // Normal speed (change to 1, 3, 4, etc.)
#define CHAR_DASH_SPEED 6   // Dash speed (change to 4, 8, 10, etc.)
```

### Change Dash Duration
```c
#define CHAR_DASH_DURATION 20  // Frames (20 frames ≈ 600ms at 30 FPS)
                               // Try 10 for quick dash, 40 for long dash
```

### Change Debounce Time
```c
#define DEBOUNCE_DELAY 200  // Milliseconds (200ms is comfortable)
                            // Try 100 for faster repeat, 300 for slower
```

### Add Multiple Dash State Feedback
```c
// In render_game(), add:
if (game_character.dash_counter > 0) {
    LCD_printString("DASHING!", 100, 50, 1, 3);
}
```

### Create Dash Trail/Trail
```c
// Add to Character_t:
// uint16_t last_x, last_y;  // Previous position
// Draw line from last position to current during dash
```

## Common Issues

| Issue | Fix |
|-------|-----|
| Button press doesn't trigger dash | Check BTN3 (GPIO PC3) is configured in gpio.c |
| Dash triggers multiple times | Increase `DEBOUNCE_DELAY` (try 250 or 300) |
| Character feels too slow | Increase `CHAR_SPEED` (try 3 or 4) |
| Dash is too fast | Decrease `CHAR_DASH_SPEED` (try 4 or 5) |
| Dash lasts too long | Decrease `CHAR_DASH_DURATION` (try 10 or 15) |
| Animation looks choppy | Adjust frame counter threshold in Character_Update (try 5 or 15) |
| Character walks off screen | Check screen boundary constants (SCREEN_MIN/MAX_X/Y) |

## Debugging Quick Keys

Add to render_game() for debugging:
```c
// Display dash timer
char dash_str[32];
sprintf(dash_str, "Dash: %d/%d", 
    game_character.dash_counter, CHAR_DASH_DURATION);
LCD_printString(dash_str, 10, 220, 1, 2);

// Show button status
LCD_printString(dash_button_pressed ? "BTN ON" : "BTN OFF", 150, 220, 1, 2);
```

## Next Steps

1. **Try moving the character** - should see IDLE → WALKING transition
2. **Press the button** - should see instant transition to DASHING
3. **Watch the dash** - character moves 3x faster for exactly 20 frames
4. **Dash ends** - character returns to IDLE if not moving, WALKING if still holding joystick
5. **Experiment** - change speeds, dash duration, button responsiveness

