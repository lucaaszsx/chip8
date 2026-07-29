.org 0x200

start:
    ; missing yet

end:
    jmp end

lg_not:
    mov ve, 0xff ; mask
    xor v0, v1   ; v0 = v0 ^ v1 -> e.g.: 0b00001010 (0xa) ^ 0b11111111 (0xff) = 0b11110101
    rts

lg_nand:
    and v0, v1
    jsr lg_not
    rts

lg_gt:

lg_gte:

lg_lt:

lg_lte:

check_sprite:
    .db 0x10, 0xa0, 0x40
    .equ CHECK_H 3

x_sprite:
    .db 0xa0, 0x40, 0xa0
    .equ X_H 3
