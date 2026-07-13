#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdbool.h>
#include <stdint.h>

#define KEYPAD_KEY_0 0x0
#define KEYPAD_KEY_1 0x1
#define KEYPAD_KEY_2 0x2
#define KEYPAD_KEY_3 0x3
#define KEYPAD_KEY_4 0x4
#define KEYPAD_KEY_5 0x5
#define KEYPAD_KEY_6 0x6
#define KEYPAD_KEY_7 0x7
#define KEYPAD_KEY_8 0x8
#define KEYPAD_KEY_9 0x9
#define KEYPAD_KEY_A 0xa
#define KEYPAD_KEY_B 0xb
#define KEYPAD_KEY_C 0xc
#define KEYPAD_KEY_D 0xd
#define KEYPAD_KEY_E 0xe
#define KEYPAD_KEY_F 0xf
#define KEYPAD_SIZE  16

struct Chip8Keypad {
    bool keys[KEYPAD_SIZE];
    bool key_was_down[KEYPAD_SIZE];
    bool waiting;
};

void chip8_keypad_press(struct Chip8Keypad *keypad, uint8_t key);
void chip8_keypad_release(struct Chip8Keypad *keypad, uint8_t key);
bool chip8_keypad_pressed(struct Chip8Keypad *keypad, uint8_t key);
void chip8_keypad_wait(struct Chip8Keypad *keypad);
bool chip8_keypad_poll(struct Chip8Keypad *keypad, uint8_t *key_out);

#endif