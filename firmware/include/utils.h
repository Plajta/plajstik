#define JSON_VERSION 1

size_t strlen_s(const char *str, size_t max_len);
int save_string(uint32_t target_addr, char* buf, int max_len);
int setup_from_json(char* buf);
int json_validity(char* buf);
void json_setup(char* buf, uint8_t* keymap, uint8_t* keymap_dpad);
