#include <string.h>

int get_pin(char const* input) {
  if (!strcmp(input, "btn0")) return 0; // These will get real names, that's why i used this
  if (!strcmp(input, "btn1")) return 1;
  if (!strcmp(input, "btn2")) return 2;
  if (!strcmp(input, "btn3")) return 3;
  if (!strcmp(input, "btn4")) return 4;
  if (!strcmp(input, "btn5")) return 5;
  if (!strcmp(input, "btn6")) return 6;
  if (!strcmp(input, "btn7")) return 7;
  if (!strcmp(input, "btn8")) return 8;
  if (!strcmp(input, "btn9")) return 9;
  return -1;
};

int find_mapping(char const* input) {
  if (!strcmp(input, "a")) return 0; // WILL DO BETTER
  if (!strcmp(input, "b")) return 1;
  if (!strcmp(input, "x")) return 3;
  if (!strcmp(input, "y")) return 4;
  if (!strcmp(input, "l1")) return 6;
  if (!strcmp(input, "r1")) return 7;
  if (!strcmp(input, "l2")) return 8;
  if (!strcmp(input, "r2")) return 9;
  if (!strcmp(input, "select")) return 10;
  if (!strcmp(input, "start")) return 11;
  if (!strcmp(input, "home")) return 12;
  if (!strcmp(input, "l3")) return 13;
  if (!strcmp(input, "r3")) return 14;
  return -1;
};

int find_dpad(char const* input) {
  if (!strcmp(input, "dpad_u")) return 0;
  if (!strcmp(input, "dpad_r")) return 1;
  if (!strcmp(input, "dpad_d")) return 2;
  if (!strcmp(input, "dpad_l")) return 3;
  return -1;
};