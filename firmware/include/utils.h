#define JSON_VERSION 1

size_t strlen_s(const char *str, size_t max_len);
int16_t clamp(int16_t d, int16_t min, int16_t max);
int save_string(uint32_t target_addr, char* buf, int max_len);
int setup_from_json(char* buf);
int json_validity(char* buf);
void json_setup(char* buf, int8_t* keymap, int8_t* keymap_dpad, double* deadzone, int8_t* x_adc, int8_t* y_adc, float* x_multiplier, float* y_multiplier);
void utils_tud_cdc_write_s(char character);
