# ELEC2645 - Character FSM Game Demo

Learn about **Finite State Machines (FSM)** through a practical game character implementation. This demo shows how FSMs are used in game development to manage character behavior, physics, and animations - a fundamental technique used in every major game engine.

**Key Files:**
- [Character/Character.h](Character/Character.h) - FSM state definitions and constants
- [Character/Character.c](Character/Character.c) - FSM implementation and physics
- [Core/Src/main.c](Core/Src/main.c) - Main game loop with update/render separation
- [CHARACTER_FSM_GUIDE.md](CHARACTER_FSM_GUIDE.md) - Comprehensive guide
- [CHARACTER_QUICK_REFERENCE.md](CHARACTER_QUICK_REFERENCE.md) - Quick reference

---

## What is a Finite State Machine?

A **Finite State Machine** is a programming pattern where:

1. **The system has a set number of "states"** (modes of operation)
2. **The system is in exactly ONE state at any time**
3. **Inputs are handled differently depending on which state the system is in**
4. **External events can cause transitions between states**

### Real-World Examples

- **Traffic Lights**: States are RED, YELLOW, GREEN - timers cause transitions
- **Mobile Phone**: States are LOCKED, HOME_SCREEN, IN_CALL, CAMERA - button presses cause transitions
- **Washing Machine**: States are IDLE, WASH, RINSE, SPIN - sensor readings cause transitions
- **Game Character**: States are IDLE, WALKING, JUMPING, ATTACKING - player input causes transitions

### The Key Insight

**The SAME input produces DIFFERENT behavior depending on the current state!**

Example: Pressing "A" on a game controller:
- In MENU state → selects menu item
- In GAMEPLAY state → character jumps
- In PAUSE state → resumes game

---

## This Demo Project

This demo implements a **Mario-like platform game character** with physics and animations controlled by a Finite State Machine.

### The Character States

| State | Behavior | Sprite | Transition To |
|-------|----------|--------|---------------|
| **IDLE** | Standing still on ground | Static sprite | RUNNING (joystick move)<br>JUMPING (button press) |
| **RUNNING** | Moving left/right | Animated run cycle | IDLE (joystick center)<br>JUMPING (button press) |
| **JUMPING** | In mid-air with physics | Jump sprite | IDLE/RUNNING (on landing) |

### Physics System

The character uses real-time physics simulation:
- **Gravity**: 1.1 pixels/frame² (constant downward acceleration)
- **Jump velocity**: -10.0 pixels/frame (initial upward velocity)
- **Run speed**: 4 pixels/frame (horizontal movement)
- **Terminal velocity**: 15.0 pixels/frame (maximum fall speed)
- **Ground collision**: Character lands at Y=200 (ground level)

### State Transitions

**Inputs determine transitions:**
- **Joystick Left/Right** → Enter RUNNING state (if on ground)
- **Joystick Centered** → Return to IDLE (if on ground)
- **BTN2 Press** → Jump to JUMPING state (if on ground)
- **Landing Detection** → Return to IDLE or RUNNING (based on joystick)

## Hardware Setup

### Input Controls

| Input | Pin | Purpose |
|-------|-----|---------|
| Joystick X-axis | ADC1_IN5 | Horizontal movement (left/right) |
| Joystick Y-axis | ADC1_IN6 | (Reserved for future use) |
| BTN2 | PC2 | Jump button (interrupt-driven) |

### Display

- **LCD**: ST7789V2 240x240 RGB565 display
- **Refresh**: ~33 FPS (30ms frame time)
- **Debug overlay**: Real-time state/position/velocity display

## Code Architecture

### Main Loop Pattern (Update/Render Separation)

```c
while (1) {
    // 1. Read inputs
    Joystick_Read(&joy_x, &joy_y);
    
    // 2. Update game logic (physics, FSM, collisions)
    update_character(&game_character, joy_x, joy_y);
    
    // 3. Render everything (clear, draw, refresh)
    render_game(&game_character);
    
    // 4. Frame timing
    HAL_Delay(30);
}
```

**Why separate update and render?**
- **Clarity**: Logic and drawing are independent concerns
- **Modularity**: Can change rendering without affecting game logic
- **Testing**: Can test physics/FSM without display hardware
- **Industry standard**: All major game engines use this pattern

### Character Module Structure

```c
// Character.h - Interface
typedef enum {
    CHAR_IDLE,
    CHAR_RUNNING,
    CHAR_JUMPING
} CharacterState_t;

typedef struct {
    int x, y;                     // Position
    float velocity_y;             // Vertical physics
    CharacterState_t state;       // Current FSM state
    int animation_frame;          // Animation counter
} Character_t;

void Character_Init(Character_t* character);
void Character_Update(Character_t* character, int joy_x, int joy_y);
void Character_Draw(Character_t* character);
```

## How the Character FSM Works

### 1. State Storage

The character's current state is stored in the `Character_t` structure:

```c
Character_t game_character = {
    .x = 120,
    .y = GROUND_Y,
    .velocity_y = 0.0f,
    .state = CHAR_IDLE,
    .animation_frame = 0
};
```

### 2. State Machine Logic (Character_Update)

Every frame, the FSM processes inputs and physics:

```c
void Character_Update(Character_t* character, int joy_x, int joy_y) {
    // 1. Handle movement input
    if (joy_x < -JOYSTICK_DEADZONE) {
        character->x -= CHAR_SPEED;  // Move left
    }
    else if (joy_x > JOYSTICK_DEADZONE) {
        character->x += CHAR_SPEED;  // Move right
    }
    
    // 2. Handle jump initiation
    if (g_jump_button_pressed && character->y >= GROUND_Y) {
        character->velocity_y = CHAR_JUMP_VELOCITY;
        character->y = GROUND_Y - 1;  // Ensure airborne detection
        g_jump_button_pressed = 0;
    }
    
    // 3. Apply physics (gravity + collision)
    if (character->y < GROUND_Y) {
        character->velocity_y += CHAR_GRAVITY;
        if (character->velocity_y > CHAR_MAX_FALL_VELOCITY) {
            character->velocity_y = CHAR_MAX_FALL_VELOCITY;
        }
        character->y += (int)character->velocity_y;
        
        if (character->y >= GROUND_Y) {
            character->y = GROUND_Y;
            character->velocity_y = 0.0f;
        }
    }
    
    // 4. Update FSM state based on physics + input
    if (character->y < GROUND_Y) {
        character->state = CHAR_JUMPING;
    }
    else if (joy_x < -JOYSTICK_DEADZONE || joy_x > JOYSTICK_DEADZONE) {
        character->state = CHAR_RUNNING;
        character->animation_frame++;
    }
    else {
        character->state = CHAR_IDLE;
        character->animation_frame = 0;
    }
}
```

### 3. State-Dependent Rendering (Character_Draw)

The sprite changes based on the current state:

```c
void Character_Draw(Character_t* character) {
    const uint8_t* sprite;
    
    switch (character->state) {
        case CHAR_IDLE:
            sprite = CharacterIDLE;
            break;
        
        case CHAR_RUNNING:
            // Animated sprite (alternates every 5 frames)
            sprite = ((character->animation_frame / 5) % 2) ? 
                     CharacterRUN1 : CharacterRUN2;
            break;
        
        case CHAR_JUMPING:
            sprite = CharacterJUMP;
            break;
    }
    
    Character_DrawScaledSprite(character->x, character->y, sprite, 4);
}
```

**This is the FSM pattern in action**: The SAME drawing function produces DIFFERENT output based on the current state!

### 4. Interrupt-Driven Jump Input

The jump button uses **hardware interrupts** for instant response:

```c
volatile uint8_t g_jump_button_pressed = 0;
volatile uint32_t g_last_jump_time = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == BTN2_Pin) {
        uint32_t current_time = HAL_GetTick();
        
        // Debouncing: Ignore presses within 100ms
        if (current_time - g_last_jump_time > 100) {
            g_jump_button_pressed = 1;  // Set flag for main loop
            g_last_jump_time = current_time;
        }
    }
}
```

**Why interrupts for buttons?**
- **Instant response**: No polling delay
- **CPU efficient**: Sleep when idle, wake on event
- **Debouncing**: Built-in timing prevents button bounce
- **Industry standard**: All game controllers use interrupt-driven input

---

## Key Programming Concepts

### 1. Finite State Machine Pattern

The FSM manages character behavior based on physics and input:
- **State storage**: enum `CharacterState_t` with 3 values
- **State logic**: Switch statement in `Character_Update()`
- **State-dependent output**: Different sprites per state in `Character_Draw()`
- **Transition rules**: Physics (grounded vs airborne) + input determines state

### 2. Physics Simulation

Real-time Newtonian physics updates every frame:
```c
velocity_y += CHAR_GRAVITY;           // Acceleration (F = ma)
y += (int)velocity_y;                 // Position integration
if (y >= GROUND_Y) { velocity_y = 0; } // Collision response
```

### 3. Update/Render Separation

Industry-standard game loop pattern:
- **Update**: Pure logic (no drawing) - physics, FSM, input
- **Render**: Pure drawing (no logic) - sprites, debug overlay
- **Benefits**: Testable, modular, clear separation of concerns

### 4. Object-Oriented C

Character module demonstrates encapsulation in C:
- **Character_t struct**: Data encapsulation (position, velocity, state)
- **Character_* functions**: Method-like interface (Init, Update, Draw)
- **Information hiding**: Private sprite arrays in .c file

### 5. Interrupt Service Routines (ISRs)

Jump button uses hardware interrupt for responsiveness:
- **HAL_GPIO_EXTI_Callback**: Automatically called on button press
- **Debouncing**: 100ms timing prevents multiple triggers
- **Flag pattern**: ISR sets flag, main loop clears it (safe communication)

---

## Extending the Demo

### Easy Modifications

1. **Adjust jump height**: Change `CHAR_JUMP_VELOCITY` in Character.h
2. **Change run speed**: Modify `CHAR_SPEED` constant
3. **Tweak gravity**: Adjust `CHAR_GRAVITY` for different feel
4. **Add wall collision**: Check `character->x` bounds in Update
5. **More animation frames**: Add sprites and increase animation counter

### Intermediate Additions

1. **Add FALLING state**: Separate falling from jumping
2. **Double jump**: Track `jumps_remaining` counter
3. **Moving platforms**: Add platform array and collision detection
4. **Sprint mechanic**: Hold button for `CHAR_RUNNING_FAST` state
5. **Sound effects**: Use Buzzer module on state transitions

### Advanced Features

1. **Multiple characters**: Array of `Character_t` with AI FSMs
2. **Particle effects**: Dust clouds on landing, jump trails
3. **Camera follow**: Scroll background, keep character centered
4. **State history**: Implement coyote time (late jump forgiveness)
5. **Game state FSM**: Add MENU, PLAYING, PAUSED, GAME_OVER states

---

## Educational Value

This demo teaches fundamental game development concepts:

1. **FSM Design Pattern** - Core technique used in every game engine
2. **Physics Simulation** - Gravity, velocity, collision detection
3. **Update/Render Loop** - Industry-standard game architecture
4. **State-Dependent Behavior** - Same input produces different output
5. **Interrupt-Driven Input** - Responsive real-time control
6. **Object-Oriented C** - Encapsulation and modularity without C++
7. **Real-Time Systems** - Frame timing, fixed timestep iteration

### Connections to Professional Game Development

- **Unity/Unreal Engine**: Use same update/render separation
- **Character controllers**: Commercial games use FSM for player movement
- **State machines**: Every NPC enemy behavior is an FSM
- **Physics engines**: Box2D, PhysX use same integration techniques
- **Input systems**: All consoles use interrupt-driven controller input

---

## Extending the Demo

Try these modifications to deepen your understanding:

### Add a New State

1. Add a new state to the enum (e.g., `STATE_POLYGONS`)
2. Increment `STATE_COUNT`
3. Add a new state handler function
4. Add the new case to the switch statement
5. Add state name to the `state_names` array

### Add State-Specific LED Patterns

Make each state light the onboard LED differently:

```c
void set_led_for_state(FSM_State_t state) {
    switch(state) {
        case STATE_CIRCLES: PWM_SetDuty(&pwm_cfg, 25); break;
        case STATE_RECTANGLES: PWM_SetDuty(&pwm_cfg, 50); break;
        // etc.
    }
}
```

### Implement Conditional Transitions

Add logic so states only change under certain conditions:
## Software Debouncing

Mechanical buttons generate electrical noise when pressed. This demo uses **software debouncing** to filter spurious interrupts:

### The Problem

```
Button Press (Ideal):       Reality:
_____|‾‾‾‾‾‾‾              _|‾|_|||‾‾‾‾
                             ↑
                    Multiple triggers from bounce!
```

### The Solution (in HAL_GPIO_EXTI_Callback)

```c
volatile uint32_t g_last_jump_time = 0;
#define DEBOUNCE_MS 100

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == BTN2_Pin) {
        uint32_t current_time = HAL_GetTick();
        
        // Ignore presses within 100ms of previous press
        if (current_time - g_last_jump_time > DEBOUNCE_MS) {
            g_jump_button_pressed = 1;
            g_last_jump_time = current_time;
        }
    }
}
```

The 100ms timing window filters out mechanical bounce while still feeling responsive to the player.

---

## Running the Code

### Build and Flash

1. Open this folder in VS Code with STM32 extension
2. Select **Debug** configuration in `.vscode/tasks.json`
3. Click **Build** (or press Ctrl+Shift+B)
4. Connect Nucleo-L476RG board via USB
5. Flash and run (press F5 for debugging)

### Using the Demo

1. **Observe idle state** - Character stands still on ground
2. **Move joystick left/right** - Character enters RUNNING state with animation
3. **Press BTN2** - Character jumps (JUMPING state with physics)
4. **Watch physics** - Gravity pulls character down, lands on ground
5. **Read debug overlay** - LCD shows current state, Y position, velocity

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
| `Character_Update()` | Character.c | FSM logic + physics (33 times/sec) |
| `Character_Draw()` | Character.c | Render sprite based on state |
| `Character_DrawGround()` | Character.c | Draw platform/ground line |
| `update_character()` | main.c | Call Character_Update (game logic) |
| `render_game()` | main.c | Clear screen, draw all, show debug info |
| `HAL_GPIO_EXTI_Callback()` | main.c | Jump button interrupt handler |

---

## Learning Outcomes

After studying this demo, you should understand:

1. ✅ **FSM for game characters** - Industry-standard technique for managing behavior
2. ✅ **Physics simulation** - Gravity, velocity, collision detection in real-time
3. ✅ **Update/render separation** - Clean architecture pattern used in all game engines
4. ✅ **State-dependent behavior** - Same function produces different output based on state
5. ✅ **Interrupt-driven input** - Hardware interrupts for responsive controls
6. ✅ **Object-oriented C** - Encapsulation using structs and function naming conventions
7. ✅ **Frame timing** - Fixed timestep game loop (30ms = ~33 FPS)
8. ✅ **Animation techniques** - Frame counters and sprite switching
9. ✅ **Debouncing** - Software filtering of mechanical button noise

### Applicable to Professional Development

These concepts transfer directly to:
- **Unity C#**: GameObject Update() and Render() separation
- **Unreal C++**: Character classes with state machines
- **Mobile games**: Touch input handling, sprite rendering
- **Web games**: requestAnimationFrame() game loops
- **Console games**: Controller interrupt handling

---

## Common FSM Applications

FSMs are fundamental to embedded systems beyond games:

### 1. Robot Control
```c
enum RobotState { SEARCHING, APPROACHING, GRIPPING, CARRYING, ERROR };
// Sensors and inputs trigger state transitions
```

### 2. Communication Protocols
```c
enum UARTState { IDLE, RECEIVING_HEADER, RECEIVING_DATA, PROCESSING };
// Byte received triggers transition based on protocol state
```

### 3. User Interfaces
```c
enum MenuState { MAIN_MENU, SETTINGS, CONFIRM_DIALOG, ABOUT };
// Button presses navigate menu hierarchy
```

### 4. Safety Systems
```c
enum SafetyState { NORMAL, WARNING, ALARM, EMERGENCY_STOP };
// Sensor thresholds trigger escalating safety responses
```

---

## Further Exploration

### Documentation
- [CHARACTER_FSM_GUIDE.md](CHARACTER_FSM_GUIDE.md) - Comprehensive tutorial
- [CHARACTER_QUICK_REFERENCE.md](CHARACTER_QUICK_REFERENCE.md) - Quick lookup

### Theory
- **State Transition Diagrams** - Visual representation of FSM behavior
- **Mealy vs Moore Machines** - Output timing in FSM theory
- **Hierarchical FSMs** - Complex nested state machines
- **Behavior trees** - Alternative to FSMs for AI (used in AAA games)

### Advanced Topics
- **Animation state machines** - Professional game engines (Unreal AnimGraph, Unity Animator)
- **Physics engines** - Box2D, Bullet Physics integration with FSMs
- **AI behavior** - Steering behaviors, pathfinding integrated with FSMs
- **Networked games** - Client prediction, server reconciliation with FSMs

---

## License

This project is part of the ELEC2645 Embedded Systems module at the University of Leeds.
````