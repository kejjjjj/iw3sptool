# iw3sptool // a multipurpose tool for call of duty 4 singleplayer

![image](https://github.com/user-attachments/assets/00f2d991-b539-46ef-89be-ecba483e3d57)

![image](https://github.com/kejjjjj/iw3sptool/assets/108032666/d9c08f5e-47fc-4a8a-90e3-eb74a546b0f0)

## Features
- collision drawing
- map exporting 
- entity drawing
- thirdperson
- player movement modification
- demo playback
- Tool assisted speedrun

## Notes
- Only for cod4 singleplayer patch 1.5

## Usage
- The .asi file goes to the "miles" folder located in the cod4 root directory
- All commands start with the following prefixes:
    - cm_ (clip map)
    - pm_ (player movement)
    - cg_thirdPerson
- cm_showCollisionFilter expects the name of a material (e.g. "clip") and using it without an argument will clear the render queue
- cm_showEntities expects the name of a classname (e.g. "trigger_multiple") and using it without an argument will clear the render queue
- you can include multiple arguments on cm_showCollisionFilter and cm_showEntities by using a space-separated list, (e.g. "clip caulk mantle")
- using the "all" argument on cm_showCollisionFilter and cm_showEntities will select *ALL* relevant items
- brushes are highlighted with green
- terrain brushmodels are highlighted with purple
- terrain is highlighted with blue
- entities and spawners are highlighted with red

![image](https://github.com/kejjjjj/iw3sptool/assets/108032666/75698826-97fb-4777-9cd7-b3da8009faf4)




# TAS Script Tutorial

TAS scripts are saved as .json files  under `cod4root\iw3sptool\tas`

### Commands
- tas_load <script_name>
- tas_start
- tas_showPrediction
- tas_strafebot

A TAS script is made up of a starting state and a list of **segments**. Each segment describes what the player does for a specific amount of time.

## 1. Basic structure

```json
{
    "startOrigin": [0, 0, 0],
    "startViewangles": [0, 0, 0],

    "segments": [
        {
            "duration": 1000,
            "forwardmove": 127
        }
    ]
}
```

Comments are supported, so you can document your scripts:

```json
{
    // Starting position
    "startOrigin": [0, 0, 0],

    "segments": [
        // Run forward for one second
        {
            "duration": 1000,
            "forwardmove": 127
        }
    ]
}
```

---

# 2. Segments

Each segment has a `duration` in milliseconds.

```json
{
    "duration": 1000,
    "forwardmove": 127
}
```

`1000` milliseconds = 1 second.

Segments run in order:

```json
"segments": [
    {
        "duration": 1000,
        "forwardmove": 127
    },
    {
        "duration": 500,
        "forwardmove": 0
    }
]
```

The player moves forward for one second, then stops for half a second.

---

# 3. Movement

Movement values range from `-127` to `127`.

### Forward/backward

```json
"forwardmove": 127
```

Move forward.

```json
"forwardmove": -127
```

Move backward.

### Left/right

```json
"rightmove": 127
```

Move right.

```json
"rightmove": -127
```

Move left.

You can combine them:

```json
{
    "duration": 1000,
    "forwardmove": 127,
    "rightmove": 127
}
```

This produces diagonal movement.

If a movement value is omitted, it defaults to zero.

---

# 4. Buttons

Buttons are specified as an array:

```json
"buttons": ["jump", "melee"]
```

Multiple buttons can be held at the same time.

Common buttons include:

| Button            | Description     |
| ----------------- | --------------- |
| `attack`          | Fire weapon     |
| `melee`           | Melee attack    |
| `use`             | Activate/use    |
| `reload`          | Reload          |
| `jump`            | Jump            |
| `crouch`          | Crouch          |
| `prone`           | Go prone        |
| `sprint`          | Sprint          |
| `ads_hold`        | Hold ADS        |
| `ads_toggle_on`   | Toggle ADS on   |
| `ads_toggle_off`  | Toggle ADS off  |
| `lean_left`       | Lean left       |
| `lean_right`      | Lean right      |
| `steady_aim`      | Steady aim      |
| `frag`            | Frag grenade    |
| `special_grenade` | Special grenade |

An empty array means no buttons are pressed:

```json
"buttons": []
```

---

# 5. View angles

Angles are controlled with the `angle` property:

```json
"angle": {
    "type": "absolute",
    "value": [pitch, yaw, roll]
}
```

The three values are:

```text
[pitch, yaw, roll]
```

## Absolute angles

`absolute` moves the view toward a specific angle:

```json
{
    "duration": 500,
    "angle": {
        "type": "absolute",
        "value": [0, 90, 0]
    }
}
```

## Delta angles

`delta` rotates relative to the current view:

```json
{
    "duration": 500,
    "angle": {
        "type": "delta",
        "value": [0, 90, 0]
    }
}
```

This turns the view 90 degrees horizontally from its current direction.

## Strafebot

The strafebot is enabled through the angle type:

```json
"angle": {
    "type": "strafebot",
}
```

For directional strafebot movement, use:

```json
{
    "duration": 2000,
    "forwardmove": 127, 
    "rightmove": 127, //strafe to the right
    "angle": {
        "type": "strafebot"
    }
}
```

For forward strafebot movement, use:

```json
{
    "duration": 2000,
    "forwardmove": 127,
    "rightmove": 0,
    "angle": {
        "type": "strafebot"
    }
}
```

---

# 6. FPS

The `fps` property controls how frequently commands are generated.

The default is `250`:

```json
{
    "duration": 1000,
    "fps": 250,
    "forwardmove": 127
}
```

You can use another rate when necessary:

```json
"fps": 125
```

---

# 7. Starting position and view

You can specify the starting position:

```json
"startOrigin": [100, 200, 50]
```

The position is:

```text
[X, Y, Z]
```

You can also specify the starting view:

```json
"startViewangles": [0, 90, 0]
```

Both properties are optional.

---

# 8. Weapons

Weapons are specified by their name:

```json
{
    "duration": 100,
    "weapon": "rpg"
}
```

You can also specify the off-hand weapon:

```json
{
    "duration": 100,
    "offhand": "beretta"
}
```

Both can be specified together:

```json
{
    "duration": 100,
    "weapon": "rpg",
    "offhand": "beretta"
}
```

If they aren't specified, the current weapon and off-hand weapon are preserved.

Use the weapon names recognized by the game.

---

# 9. Complete example

```json
{
    // Starting state
    "startOrigin": [1200, 26125, 212],
    "startViewangles": [0, 90, 0],

    "segments": [
        // Sprint forward
        {
            "duration": 1000,
            "buttons": ["sprint"],
            "forwardmove": 127
        },

        // Jump while continuing forward
        {
            "duration": 500,
            "buttons": ["jump"],
            "forwardmove": 127
        },

        // Automatic air strafing to the right
        {
            "duration": 3000,
            "forwardmove": 127,
            "rightmove": 127,
            "angle": {
                "type": "strafebot"
            }
        },

        // Turn 90 degrees
        {
            "duration": 500,
            "angle": {
                "type": "delta",
                "value": [0, 90, 0]
            }
        },

        // Switch to the RPG
        {
            "duration": 1000,
            "weapon": "rpg"
        },

        // Fire
        {
            "duration": 1000,
            "weapon": "rpg",
            "buttons": ["attack"]
        }
    ]
}
```
