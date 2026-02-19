# ELEC2645 - Character FSM with Dash Ability Demo

Learn about **Finite State Machines (FSM)** through a practical game character implementation. This demo shows how FSMs are used in game development to manage character behavior and state transitions triggered by button input.

**Key Files:**
- [Character/Character.h](Character/Character.h) - FSM state definitions and constants
- [Character/Character.c](Character/Character.c) - FSM implementation
- [Core/Src/main.c](Core/Src/main.c) - Main game loop and interrupt handling
- [CHARACTER_QUICK_REFERENCE.md](CHARACTER_QUICK_REFERENCE.md) - Quick reference

---

## This Demo Project

This demo implements a **simple game character with dash ability** controlled by a Finite State Machine.

### The Character States

| State | Behavior | Sprite | Movement Speed | Transition To |
|-------|----------|--------|-----------------|---------------|
| **IDLE** | Standing still | Static sprite | 0 | WALKING (joystick move)<br>DASHING (button press) |
| **WALKING** | Moving in direction | Animated walk cycle | 2 px/frame | IDLE (joystick center)<br>DASHING (button press) |
| **DASHING** | Fast movement | Speed lines sprite | 6 px/frame | IDLE (dash ends, no input)<br>WALKING (dash ends, input held) |

### Dash System

**Button-triggered temporary speed boost:**
- Press button (BTN3) to start dashing
- Dash duration: 20 frames (~600ms at 30 FPS)
- Dash speed: 6 pixels/frame (3x normal speed)
- Can dash from IDLE or WALKING state
- Automatically returns to previous state when dash ends

### State Transitions

**Inputs determine transitions:**
- **Joystick Move** → Enter WALKING state
- **Joystick Centered** → Return to IDLE
- **Button Press** → Enter DASHING state from IDLE or WALKING
- **Dash Duration Expires** → Return to IDLE (if no input) or WALKING (if input held)

## Code Architecture

### Main Loop Pattern (Update/Render Separation)

```c
while (1) {
    // 1. Read inputs (joystick from ADC)
    Joystick_Read(&joystick_cfg, &joystick_data);
    
    // 2. Update game logic (FSM state machine)
    //    dash_button_pressed is set by interrupt handler
    update_character(&joystick_data);
        // → Character_Update(&game_character, joy, dash_button_pressed)
    
    // 3. Render everything (clear, draw, refresh)
    render_game();
        // → Draw character, debug info
    
    // 4. Frame timing
    HAL_Delay(30);  // ~33 FPS
}
```

**Why separate update and render?**
- **Clarity**: Logic and drawing are independent concerns
- **Modularity**: Can change rendering without affecting game logic
- **Testing**: Can test physics/FSM without display hardware
- **Industry standard**: All major game engines use this pattern

### Character Module Structure

```c
// Character.h - Simple sprite with state
typedef enum {
    CHAR_IDLE,        // Not moving
    CHAR_WALKING,     // Moving in direction
    CHAR_DASHING      // Fast movement (button triggered)
} CharacterState_t;

// Minimal data structure - just position and state
typedef struct {
    int16_t x, y;                   // Position
    CharacterState_t state;         // Current state
    uint8_t animation_frame;        // 0 or 1 (walk cycle)
    uint8_t frame_counter;          // Animation timing
    uint8_t dash_counter;           // Frames left in dash
} Character_t;

void Character_Init(Character_t* character);
void Character_Update(Character_t* character, Joystick_t* joy, uint8_t dash_pressed);
void Character_Draw(Character_t* character);
```

Just 5 essential fields - position, state, and animation/dash counters. Nothing more.

## How the Character FSM Works

### 1. State Storage

The character's current state is stored in a simple struct:

```c
Character_t game_character;  // Created in main, contains position and state
Character_Init(&game_character);  // Initialize at screen center
```

### 2. Interrupt-Driven Input

Button presses trigger interrupts rather than polling:

```c
// Global flag set by interrupt handler
volatile uint8_t dash_button_pressed = 0;
volatile uint32_t dash_button_last_interrupt_time = 0;
#define DEBOUNCE_DELAY 200  // milliseconds

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == BTN3_Pin) {
        uint32_t current_time = HAL_GetTick();
        // Debounce: ignore if less than 200ms since last press
        if ((current_time - dash_button_last_interrupt_time) > DEBOUNCE_DELAY) {
            dash_button_last_interrupt_time = current_time;
            dash_button_pressed = 1;  // Flag tells FSM to start dash
        }
    }
}
```

Remember to keep the ISR short and simple!

### 3. State Machine Logic (Character_Update)

Each frame, update character movement and state:

```c
void Character_Update(Character_t* character, Joystick_t* joy, uint8_t dash_pressed) {
    
    // STEP 1: Get movement direction from joystick (N/S/E/W...)
    int16_t move_x = 0, move_y = 0;
    
    switch (joy->direction) {
        case N:  move_y = -1; break;
// and so on for NE, E, SE, S, SW, W, NW
    }
    
    // STEP 2: Dash button pressed?
    if (dash_pressed && character->dash_counter == 0) {
        character->dash_counter = CHAR_DASH_DURATION;
    }
    
    // STEP 3: Apply speed (normal or dash speed)
    uint8_t speed = CHAR_SPEED;
    if (character->dash_counter > 0) {
        speed = CHAR_DASH_SPEED;
        character->dash_counter--;
    }
    
    // STEP 4: Move sprite
    int16_t new_x = character->x + (move_x * speed);
    int16_t new_y = character->y + (move_y * speed);
    if (new_x >= 20 && new_x <= 220) character->x = new_x;
    if (new_y >= 20 && new_y <= 220) character->y = new_y;
    
    // STEP 5: Update state
    uint8_t is_moving = (move_x != 0 || move_y != 0);
    if (character->dash_counter > 0) {
        character->state = CHAR_DASHING;
    } else if (is_moving) {
        character->state = CHAR_WALKING;
    } else {
        character->state = CHAR_IDLE;
    }
    
    // STEP 6: Update animation
    if (character->state == CHAR_WALKING) {
        character->frame_counter++;
        if (character->frame_counter >= 10) {
            character->frame_counter = 0;
            character->animation_frame = (character->animation_frame + 1) % 2;
        }
    } else {
        character->animation_frame = 0;
        character->frame_counter = 0;
    }
}
```

**Simple and straightforward**: Just read input, handle the dash, move the sprite, update state, and animate.

### 4. State-Dependent Rendering (Character_Draw)

Draw different sprites based on current state:

```c
void Character_Draw(Character_t* character) {
    int16_t x_pos = character->x - 16;  // Center the sprite
    int16_t y_pos = character->y - 16;
    
    switch (character->state) {
        case CHAR_IDLE:
            LCD_Draw_Sprite_Colour_Scaled(x_pos, y_pos, 8, 8, 
                                         (uint8_t*)CharacterIDLE, 5, 4);
            break;
        
        case CHAR_WALKING:
            if (character->animation_frame == 0) {
                LCD_Draw_Sprite_Colour_Scaled(x_pos, y_pos, 8, 8,
                                             (uint8_t*)CharacterWALK1, 5, 4);
            } else {
                LCD_Draw_Sprite_Colour_Scaled(x_pos, y_pos, 8, 8,
                                             (uint8_t*)CharacterWALK2, 5, 4);
            }
            break;
        
        case CHAR_DASHING:
            LCD_Draw_Sprite_Colour_Scaled(x_pos, y_pos, 8, 8,
                                         (uint8_t*)CharacterDASHING, 5, 4);
            break;
    }
}
```

Each state has its own direct LCD call - clean and easy to follow.

---

## Key Programming Concepts

### 1. Finite State Machine Pattern

The FSM manages character behavior based on input and button presses:
- **State storage**: enum `CharacterState_t` with 3 values (IDLE, WALKING, DASHING)
- **State logic**: Switch statement in `Character_Update()`
- **State-dependent output**: Different sprites per state in `Character_Draw()`
- **Transition rules**: Input + button press + dash timer determines state

### 2. Interrupt-Driven Input Handling

See implementation above in "How the Character FSM Works".

**Why interrupt-driven?**
- Responsive (no polling delay)
- Low CPU overhead (only triggered on button press)
- Industry standard for game input
- Allows main loop to focus on game logic

### 3. Update/Render Separation

Industry-standard game loop pattern:
- **Update**: Pure logic (no drawing) - FSM, input processing, state transitions
- **Render**: Pure drawing (no logic) - sprites, debug overlay, screen refresh
- **Benefits**: Testable, modular, clear separation of concerns

### 4. Object-Oriented C

Character module demonstrates simple encapsulation in C:
- **Character_t struct**: Data storage (position, state, animation)
- **Character_* functions**: Interface (Init, Update, Draw)
- **Info hiding**: Sprite arrays in .c file only
- **Simple and direct**: No complex patterns, just straightforward C

---

## Extending the Demo

### Easy Modifications

1. **Adjust speeds**: Change `CHAR_SPEED` and `CHAR_DASH_SPEED` in Character.h
2. **Longer dash**: Increase `CHAR_DASH_DURATION` (in frames)
3. **Screen boundaries**: Adjust the coordinate checks (20 to 220) in `Character_Update()`
4. **Animation speed**: Change frame counter threshold (currently 10) in `Character_Update()`
5. **New sprite graphics**: Replace `CharacterIDLE`, `CharacterWALK1`, `CharacterWALK2`, `CharacterDASHING` arrays

### Intermediate Additions

1. **Add multiple dash abilities**: Track dash count, allow 2 dashes before recharge
2. **Directional visual feedback**: Change sprite rotation based on movement direction
3. **Dash particle effects**: Draw visual effects during dash state
4. **Sound on dash**: Play buzzer sound when dash starts
5. **Screen-wrap effect**: Let character move off edge and reappear on opposite side

### Advanced Features

1. **Enemy AI**: Create second character with FSM that hunts the player
2. **Collectibles**: Add state for picking up items
3. **Combo system**: Reward consecutive dashes without stopping
4. **Tutorial overlay**: Display state transitions on screen
5. **Game state FSM**: Add MENU, PLAYING, PAUSED, GAME_OVER states

---

## Educational Value

This demo teaches fundamental game development concepts:

1. **FSM Design Pattern** - Core technique used in every game engine
2. **Interrupt-Driven Input** - Responsive player control architecture
3. **Update/Render Loop** - Industry-standard game architecture
4. **State-Dependent Behavior** - Same input produces different output based on state
5. **Time-Based State Transitions** - DASHING state automatically expires
6. **Object-Oriented C** - Encapsulation and modularity without C++
7. **Real-Time Systems** - Frame timing, fixed timestep iteration

### Connections to Professional Game Development

- **Unity/Unreal Engine**: Use same update/render separation
- **Character controllers**: Commercial games use FSM for player movement
- **State machines**: Every NPC enemy behavior is an FSM
- **Input systems**: All modern games use interrupt-driven controllers
- **Ability systems**: Dash, jump, dodge are time-limited states

---

## Running the Code

### Build and Flash

1. Open this folder in VS Code with STM32 extension
2. Select **Debug** configuration in `.vscode/tasks.json`
3. Click **Build** (or press Ctrl+Shift+B)
4. Connect Nucleo-L476RG board via USB
5. Flash and run (press F5 for debugging)

### Using the Demo

1. **Observe idle state** - Character stands still
2. **Move joystick in cardinal directions (N/S/E/W)** - Character enters WALKING state with animation
3. **Press button (BTN3)** - Character enters DASHING state (fast movement, speed sprite)
4. **Watch state transitions** - Character automatically returns to previous state when dash ends or input stops

---

## Code Structure

### File Organization

```
Character/
  ├── Character.h         - FSM states, constants, function prototypes
  └── Character.c         - FSM implementation, sprites, physics
  
Core/Src/main.c          - Main game loop (update/render separation)

Joystick/                - Analog input driver
ST7789V2_Driver_STM32L4/ - LCD display driver
```

### Key Functions

| Function | Location | Purpose |
|----------|----------|---------|
| `Character_Init()` | Character.c | Initialize character struct |
| `Character_Update()` | Character.c | FSM logic (movement, state transitions, animation) |
| `Character_Draw()` | Character.c | Render sprite based on state |
| `update_character()` | main.c | Call Character_Update + clear button flag |
| `render_game()` | main.c | Clear screen, draw all, show debug info |
| `HAL_GPIO_EXTI_Callback()` | main.c | Button interrupt handler (set dash flag) |

---

## Learning Outcomes

After studying this demo, you should understand:

1. ✅ **FSM for game characters** - Industry-standard technique for managing behavior
2. ✅ **Update/render separation** - Clean architecture pattern used in all game engines
3. ✅ **State-dependent behavior** - Same function produces different output based on state
4. ✅ **Interrupt-driven input** - Hardware interrupts for responsive controls with debouncing
5. ✅ **Button debouncing** - Software filtering of mechanical button noise
6. ✅ **Time-based state transitions** - Dash state automatically expires after duration
7. ✅ **Object-oriented C** - Encapsulation using structs and function naming conventions
8. ✅ **Animation techniques** - Frame counters and sprite switching
9. ✅ **Fixed timestep game loop** - 30ms frames for consistent behavior

### Applicable to Professional Development

These concepts transfer directly to:
- **Unity C#**: GameObject Update() and Render() separation, Animator state machines
- **Unreal C++**: Character movement state machines, ability systems
- **Mobile games**: Touch input handling, sprite rendering, state-based gameplay
- **Console games**: Controller interrupt handling, dash/ability mechanics
- **Web games**: requestAnimationFrame() game loops, canvas rendering

---

## License

This project is part of the ELEC2645 Embedded Systems module at the University of Leeds.
````