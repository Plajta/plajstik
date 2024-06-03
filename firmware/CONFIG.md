# Plajstick config JSON
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

## Deadzone
Describes the diameter of the deadzone, this number cannot really be compared to anything so it's ideal size is really only a matter of trial and error.

## Axes
#### Currently missing support for triggers and right analog stick
List of axis names and to what ADC inputs are they bound to

Example:
```
"x": 1
```
X left analog axis is bound to ADC 1.