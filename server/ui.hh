#ifndef UI_HH
#define UI_HH

#include <cstdint>

namespace ui {

void init();
void close();

void beep_success();
void beep_error();

void print(const char* str);
void clear();
void set_position(uint8_t x, uint8_t y);

}

#endif //UI_HH
