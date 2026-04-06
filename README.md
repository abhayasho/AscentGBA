# Ascent — Milestone 01

Student: Ashok Abhay  

## What is finished 

- State machine wsith all required states: START (splash), INSTRUCTIONS, GAME (Mode 0 tiled map), PAUSE, WIN, LOSE. You can move through every state with the buttons below.
- Mode 4 menu screens with text labels on top of colored backgrounds. Mode 4 uses BG2 enabled so the screen is not blank.
- Mode 0 gameplay: scrolling tilemap, three connected rooms in a row (east doors: room 0 → 1 → 2). Room 2 uses a darker floor tile. The grunt enemy only appears in room 0.
- Sprites: player (green square), enemy (red square), sword swing (yellow square), HP pips at the top-left.
- Combat: A sword attack with a short hitbox; enemy has HP. Colliding with the enemy damages the player (brief invincibility after a hit).
- Win / lose: WIN when the enemy’s HP reaches 0; LOSE when the player’s HP reaches 0.

## What still needs to be added 

- Real art and animation, tower progression, bosses, abilities, audio, two independent backgrounds / parallax, runtime tile and palette effects, cheat, polished HUD text.

- I dont think I will do the XP system anymore, I will just have the ability system and have one ability equiped instead of two, to keep the project a bit more manageable. 

- Dialogue, will be done last, I still need to think of it. 

## Known issues

- Graphics are intentionally placeholder squares for M01 per course guidance.
- Collisions between enemies are not done right now. if I go near the enemy I get hurt, but soon these enemies should have like a swing or move around and shoot. This was be implemented next Milestone. 

## How to build

Use the run tasks method. 

Run your build task, or from the project directory
The build produces a `.gba` file in this folder  
## How to play and see each state

### Title / splash (dark blue)

| Button | Action |
|--------|--------|
| START | Start a new game (full reset) |
| SELECT | Open Instructions |

### Instructions (green)

| Button | Action |
|--------|--------|
| START or A | Begin game |
| B | Back to splash |

### Game (dungeon)

| Button | Action |
|--------|--------|
| D-Pad | Move |
| A | Sword attack |
| START | Pause (does not reset the run) |

### Pause (gray)

| Button | Action |
|--------|--------|
| START | Resume same run |
| SELECT | Quit to splash (resets the session) |

### Win (yellow) / Lose (red)

| Button | Action |
|--------|--------|
| START or A | Return to splash |

### Rooms

- Room 0: Enemy here. Use the east door (right side, vertical gap) to enter room 1.
- Room 1: Middle room; west back to 0, east to room 2.
- Room 2: Darker floor; west returns to room 1.


