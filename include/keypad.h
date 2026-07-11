#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdbool.h>
#include <stdint.h>

struct Chip8Keypad {
    bool keys[16];
    int waiting_key;
};

void chip8_keypad_press(struct Chip8Keypad *keypad, uint8_t key);
void chip8_keypad_release(struct Chip8Keypad *keypad, uint8_t key);
bool chip8_keypad_pressed(struct Chip8Keypad *keypad, uint8_t key);
bool chip8_keypad_wait(struct Chip8Keypad *keypad, uint8_t *key);

#endif