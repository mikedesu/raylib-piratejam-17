# There Can Be... 

## README.md / Design Document

### For PirateJam 17

*Last Updated: 2025-07-28*

## Game Concept
"A shmup/infinite-runner hybrid where 'only one' rules them all"

### Core Mechanics:
- Single life-run gameplay  
- Progressive sword upgrades define playstyle
- Resource management (coins/experience) 
- Day/night cycle affecting enemy spawns

## Technical Architecture

### Core Systems:

**Entity-Component System**
- Used for all game objects (hero, enemies, items)  
- Components stored in unordered_maps keyed by entityid  
- Bitmask tracks which components each entity has  

**Scene Management**
- Finite state machine handles:
  - Company logo → Title → Gameplay → GameOver  

**Render Pipeline**
- Uses Raylib's texture rendering pipeline  
- Shaders applied post-processing for visual effects:
  - HP bars
  - Weapon durability  
  - Selection highlight  

### Key Data Structures
```cpp
// Component storage
unordered_map<entityid, string> names;  
unordered_map<entityid, entity_type> types;
unordered_map<entityid, Vector2> positions; 
// ...etc
```

## Gameplay Systems

### Controls
- **Arrow Keys**: Move hero (4-way)  
- **A Button**: Sword attack (direction-aware), confirm/select
- **Enter**: Confirm/select  

### Upgrade System
Merchant offers 3 random upgrades per visit:
- **Sword Durability** (+1 hit per upgrade)  
- **Movement Speed** (+25% speed)  
- **HP Expansion** (+1 max HP)  
- **Sword Size** (+10% hitbox)

### Enemy Types  

| Type       | HP | Movement                  | Spawn Condition  | Implemented? |
|------------|----|---------------------------|------------------|--------------|
| Orc        | 1  | Horizontal runner         | Always           | Yes          |
| Bat        | 1  | Vertical dive             | Nighttime only   | Yes          |
| Orc Boss   | 5  | Slow, heavy horizontal    | Levels 15+       | No           |

### Day/Night Cycle
- **Day**: Orcs spawn normally
- **Night**: Bats spawn, 50% faster enemy speed  
- Sky color calculated based on position of sun / moon

## Roadmap

### Planned Features

- Diagonal sword attacks  
- Screen-clearing bomb item  
- Combo counter visual  
- Magnetic item powerup
- Shield powerup  
- Persistent high scores  
- New enemies
    - Golem 
    - Zombie
    - Wisp
- Procedural music system  
- Particle effects  
- Localization support  

## Constants Reference

Key tunable values from `main.cpp`:

```cpp
// Gameplay
#define DEFAULT_ZOOM 8.0f
#define HERO_VELO_X_DEFAULT 0.25f  
#define BASE_ORC_SPEED -0.20f
#define DEFAULT_SPAWN_FREQ 300
// etc...

// Economy
#define BASE_COIN_LEVEL_UP_AMOUNT 5
#define MERCHANT_ITEM_SELECTION_MAX 3
// etc...
```

## Troubleshooting

**Known Issues:**
- Web build audio latency  
