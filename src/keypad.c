#include "keypad.h"

void chip8_keypad_press(struct Chip8Keypad *keypad, uint8_t key) {
    if (key > 0xf) return;
    if (keypad->waiting) keypad->key_was_down[key] = true;
    keypad->keys[key] = true;
}

void chip8_keypad_release(struct Chip8Keypad *keypad, uint8_t key) {
    if (key > 0xf) return;
    keypad->keys[key] = false;
}

bool chip8_keypad_pressed(struct Chip8Keypad *keypad, uint8_t key) {
    if (key > 0xf) return false;
    return keypad->keys[key];
}

void chip8_keypad_wait(struct Chip8Keypad *keypad) {
    keypad->waiting = true;
    for (uint8_t key = 0; key <= 0xf; key++)
        keypad->key_was_down[key] = keypad->keys[key];
}

bool chip8_keypad_poll(struct Chip8Keypad *keypad, uint8_t *key_out) {
    if (!keypad->waiting) return false;
    
    for (uint8_t key = 0; key <= 0xf; key++) {
        if (keypad->key_was_down[key] && !keypad->keys[key]) {
            *key_out = key;
            keypad->waiting = false;
            return true;
        }
    }

    return false;
}