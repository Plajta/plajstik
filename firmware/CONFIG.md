# Plajstick config
Completely configured, completely hypothetical config JSON:
```json
{
  "version": 1,
  "buttons": {
    "select": 0,
    "start": 1,
    "home": 2,

    "a": 3,
    "b": 4,
    "x": 5,
    "y": 6,

    "r1": 7,
    "l1": 8,
    "r2": 9,
    "l2": 10,
    "r3": 11,
    "l3": 12,

    "dpad_u": 13,
    "dpad_r": 14,
    "dpad_d": 15,
    "dpad_l": 16
  },
  "deadzone": 16.0,
  "axes": {
    "x": 1,
    "y": 0
  },
  "multiplier": {
    "x": 1.0,
    "y": 1.0
  }
}
```
## Version
Describes what version of firmware this config is compatible with

Only change if you really know what you're doing

## Buttons
List of button names and to what GPIOs are they bound to

Example:
```
"l3": 3
```
Button L3 (Left stick button) is bound to GPIO 3.

All possible buttons: `a`, `b`, `x`, `y`, `l1`, `r1`, `l2`, `r2`, `l3`, `l3`, `select`, `start`, `home`, `dpad_u`, `dpad_r`, `dpad_d`, `dpad_l`

## Deadzone
Describes the diameter of the deadzone, this number cannot really be compared to anything so it's ideal size is really only a matter of trial and error.

## Axes
*Currently missing support for triggers and right analog stick*
List of axis names and to what ADC inputs are they bound to

Example:
```
"x": 1
```
X left analog axis is bound to ADC 1.

## Multiplier
Floating point numbers that multiply the ADC output so the range can be limited, extended or inverted.

Example:
```
"y": -0.32
```
Y axis is multiplied by -0.32.
