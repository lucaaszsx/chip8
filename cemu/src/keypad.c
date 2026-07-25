#include <string.h>
#include "keypad.h"

void chip8_keypad_press(struct Chip8Keypad *keypad, uint8_t key) {
    if (key > KEYPAD_KEY_F) return;
    if (keypad->waiting) keypad->key_was_down[key] = true;
    keypad->keys[key] = true;
}

void chip8_keypad_release(struct Chip8Keypad *keypad, uint8_t key) {
    if (key > KEYPAD_KEY_F) return;
    keypad->keys[key] = false;
}

bool chip8_keypad_pressed(struct Chip8Keypad *keypad, uint8_t key) {
    if (key > KEYPAD_KEY_F) return false;
    return keypad->keys[key];
}

void chip8_keypad_wait(struct Chip8Keypad *keypad) {
    keypad->waiting = true;
    memcpy(keypad->key_was_down, keypad->keys, KEYPAD_SIZE);
}

bool chip8_keypad_poll(struct Chip8Keypad *keypad, uint8_t *key_out) {
    if (!keypad->waiting) return false;
    
    for (uint8_t key = 0; key <= KEYPAD_KEY_F; key++) {
        if (keypad->key_was_down[key] && !keypad->keys[key]) {
            *key_out = key;
            keypad->waiting = false;
            return true;
        }
    }

    return false;
}