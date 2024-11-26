#include <string.h>
#include <stdint.h>

int8_t find_mapping(char const* input) {
  if (strcmp(input, "a") == 0) return 0;
  if (strcmp(input, "b") == 0) return 1;
  if (strcmp(input, "x") == 0) return 3;
  if (strcmp(input, "y") == 0) return 4;
  if (strcmp(input, "l1") == 0) return 6;
  if (strcmp(input, "r1") == 0) return 7;
  if (strcmp(input, "l2") == 0) return 8;
  if (strcmp(input, "r2") == 0) return 9;
  if (strcmp(input, "select") == 0) return 10;
  if (strcmp(input, "start") == 0) return 11;
  if (strcmp(input, "home") == 0) return 12;
  if (strcmp(input, "l3") == 0) return 13;
  if (strcmp(input, "r3") == 0) return 14;
  return -1;
};

int8_t find_dpad(char const* input) {
  if (strcmp(input, "dpad_u") == 0) return 0;
  if (strcmp(input, "dpad_r") == 0) return 1;
  if (strcmp(input, "dpad_d") == 0) return 2;
  if (strcmp(input, "dpad_l") == 0) return 3;
  return -1;
};

uint8_t find_dpad_dir(uint8_t* dpad){
  // Diagonal directions
  if (dpad[0] && dpad[1]) return 2; // Up-Right
  if (dpad[2] && dpad[1]) return 4; // Down-Right
  if (dpad[2] && dpad[3]) return 6; // Down-Left
  if (dpad[0] && dpad[3]) return 8; // Up-Left

  // Straight directions
  if (dpad[0]) return 1; // Up
  if (dpad[1]) return 3; // Right
  if (dpad[2]) return 5; // Down
  if (dpad[3]) return 7; // Left

  return 0; // No direction pressed
}
