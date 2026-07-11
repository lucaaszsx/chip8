#include "keypad.h"
#include <stdio.h>

void chip8_keypad_press(struct Chip8Keypad *keypad, uint8_t key) {
    if (key > 0xf) return;
    keypad->keys[key] = true;
    
    if (keypad->waiting_key == -1) keypad->waiting_key = key;
}

void chip8_keypad_release(struct Chip8Keypad *keypad, uint8_t key) {
    if (key > 0xf) return;
    keypad->keys[key] = false;
}

bool chip8_keypad_pressed(struct Chip8Keypad *keypad, uint8_t key) {
    if (key > 0xf) return false;
    return keypad->keys[key];
}

bool chip8_keypad_wait(struct Chip8Keypad *keypad, uint8_t *key) {
    if (key > 0xf) return false;
    if (keypad->waiting_key == -1) return false;

    *key = keypad->waiting_key;
    keypad->waiting_key = -1;

    return true;
}
