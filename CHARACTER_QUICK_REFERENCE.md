# Character FSM - Quick Reference

## Input Controls
```
┌─────────────────────────────────────┐
│  Joystick X-axis (left/right)       │  → Move character (4 px/frame)
├─────────────────────────────────────┤
│  BTN2 (Blue button)                 │  → Jump (only on ground)
└─────────────────────────────────────┘
```

## State Transitions

```
                    ┌──────────┐
                    │  GROUND? │
                    └────┬─────┘
                         │
        ┌────────────────┼────────────────┐
        │                │                │
       NO              YES              YES
        │                │                │
        │         ┌─────────────┐         │
        │────────>│  JUMPING    │         │
        │         └─────────────┘         │
        │              │                  │
        │              │ ◄────────────────┤
        │              │ (landing)        │
        │              ▼                  │
        │    ┌──────────────────┐         │
        │    │ Check direction  │         │
        │    ├──────────────────┤         │
        │    │ X=0 ? Y dir!=0   │         │
        │    └────┬──────────┬──┘         │
        │         │YES       │NO          │
        │    ┌────▼──┐  ┌────▼─────┐     │
        │    │ IDLE  │  │  RUNNING  │    │
        │    └────┬──┘  └────┬──────┘    │
        │         │           │          │
        │         │ (X≠0)      │ (X=0)   │
        │         └─────┬──────┘         │
        │               │                │
        └───────────────┼────────────────┘
              JUMP BTN?  
```

## State Animations

### IDLE
```
   ◎◎
   ││
  ╱╲╱╲
```
Frame: 1 (static)

### RUNNING (alternating)
```
Frame 1:         Frame 2:
   ◎◎            ◎◎
   ││            ││
  ╱║ ║           ║ ╲╱
   ║ ╲           ╱ ║
```

### JUMPING
```
   ◎▲
   ││
  ╱║ ║
   ║ ║
```

## File Map

```
Unit_3_4_FSM_Game/
├── Character/
│   ├── Character.h          ← FSM definitions & prototypes
│   └── Character.c          ← Sprites, FSM logic, physics
│
├── Core/Src/
│   └── main.c               ← update_character() & render_game()
│
└── CMakeLists.txt          ← Build config
```

## Important Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| `CHAR_SPEED` | 4 | Horizontal movement speed (px/frame) |
| `CHAR_GRAVITY` | 1.1f | Gravity acceleration (px/frame²) |
| `CHAR_JUMP_VELOCITY` | -10.0f | Jump initial velocity (upward) |
| `CHAR_MAX_FALL_VELOCITY` | 15.0f | Terminal velocity |
| `CHAR_WIDTH` | 32 | Character sprite width (8px scaled 4x) |
| `CHAR_HEIGHT` | 32 | Character sprite height (8px scaled 4x) |
| `GROUND_Y` | 200 | Ground position on LCD |
| `JUMP_DEBOUNCE_DELAY` | 100ms | Jump button debounce |

## Key Functions

### Character Module Functions
- `Character_Init(char*)` - Initialize character
- `Character_Update(char*, joystick, jump_btn)` - Update FSM (call each frame)
- `Character_Draw(char*)` - Draw character on LCD
- `Character_DrawGround()` - Draw ground reference
- `Character_GetStateName(char*)` - Get state name string

### Main Loop Integration (Update/Render Pattern)
```c
// In main:
Character_t game_character;
Character_Init(&game_character);

// In main loop (30ms per frame):
Joystick_Read(&joystick_cfg, &joystick_data);

// UPDATE: Game logic only
update_character(&joystick_data);
    // Calls: Character_Update(&game_character, joy, jump_button_pressed)
    // Updates FSM, physics, animations

// RENDER: Drawing only
render_game();
    // Clears screen
    // Draws ground, character, debug info
    // Refreshes LCD
```

## Physics Simulation

### Each Frame (in Character_Update):
1. **Read input** (joystick X, jump button)
2. **Horizontal movement**: `x += direction * CHAR_SPEED` (4 px/frame)
3. **Jump initiation**: If `jump_button && on_ground`: set `velocity_y = -10.0f`, move character up 1px
4. **Vertical physics** (if in air):
   - Apply gravity: `velocity_y += CHAR_GRAVITY` (1.1 px/frame²)
   - Cap velocity: `velocity_y = min(velocity_y, CHAR_MAX_FALL_VELOCITY)` (15.0)
   - Update position: `y += velocity_y`
   - Check landing: `if (y >= GROUND_Y) { y = GROUND_Y; velocity_y = 0; }`
5. **Update FSM state** based on physics + input
6. **Update animation** frame counter

### Key Physics Logic Fix
The jump is initiated by setting upward velocity AND moving character above ground by 1px. This ensures the next frame detects the character as airborne, allowing the velocity to take effect.

## Customization Tips

### Change Jump Height
Modify `CHAR_JUMP_VELOCITY` in Character.h (more negative = higher)
```c
#define CHAR_JUMP_VELOCITY -10.0f  // Current value (medium jump)
#define CHAR_JUMP_VELOCITY -15.0f  // Example: higher jump
```

### Change Run Speed
Modify `CHAR_SPEED` in Character.h
```c
#define CHAR_SPEED 4  // Current value (medium speed)
#define CHAR_SPEED 6  // Example: faster running
```

### Adjust Gravity
Modify `CHAR_GRAVITY` in Character.h
```c
#define CHAR_GRAVITY 1.1f  // Current value (moon-like)
#define CHAR_GRAVITY 2.0f  // Example: heavier, faster falling
```

### Add New State
1. Add enum value: `CHAR_NEWSTATE`
2. Handle in FSM switch statement
3. Add sprite array for animation
4. Add case in Character_Draw()

## Common Issues

| Issue | Fix |
|-------|-----|
| Character falls off screen | Check GROUND_Y value (current: 200) |
| Jump too short | Increase `CHAR_JUMP_VELOCITY` magnitude (try -15.0f) |
| Character feels sluggish | Increase `CHAR_SPEED` (try 6-8) or decrease gravity |
| Animation choppy | Decrease ANIMATION_SPEED in Character.c (try 3) |
| Jump doesn't work | Check button debounce (100ms) or BTN2 pin config |
| Character runs off-screen | Add boundary checks in Character_Update() |

## Debugging Quick Keys

Add to Character_Update() loop:
```c
// Display velocity
printf("VY: %.1f, Y: %d\n", game_character.velocity_y, game_character.y);

// Check state transitions
if (character->state != character->prev_state) {
    printf("State: %s\n", Character_GetStateName(character));
}
```

## Next Steps

1. ✓ Understand the FSM concept
2. ✓ Run the demo
3. ✓ Modify a constant (jump height)
4. ✓ Add a new animation frame
5. ✓ Add new state (e.g., FALLING, WALL_SLIDE)
6. ✓ Add multiple characters with separate FSMs
