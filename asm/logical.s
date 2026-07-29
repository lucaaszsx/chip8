.org 0x200

start:
    ; missing yet

end:
    jmp end

; invert bits of A
;   inputs: v0 (A)
;   output: v2
lnot:
    mov v2, v0   ; copy v0 (A) to v2
    mov ve, 0xff ; mask
    xor v2, ve   ; v2 = v2 ^ ve
    rts

; do a nand operation with A and B
;   inputs: v0 (A), v1 (B)
;   output: v2
lnand:
    and v2, v1 ; apply and: A . B
    jsr lnot   ; invert bits of the "and" operation result
    rts

; do a xnor operation with A and B
;   inputs: v0 (A), v1 (B)
;   output: v2
lxnor:
    xor v2, v1 ; apply xor: A ^ B
    jsr lnot   ; invert bits of the "xor" operation result
    rts

; check if A is greater than B
;   inputs: v0 (B), v1 (A)
;   output: v2
lgt:
    mov v2, v0 ; copy v0 (B) to v2
    jsr lnot   ; invert bits of B
    and v2, v1 ; apply and: ~B (v2) . A (v1)
    rts

; check if A is greater than or equal to B
;   inputs: v0 (B), v1 (A)
;   output: v0
lgte:
    mov v2, v0 ; copy v0 (B) to v2
    jsr lnot   ; invert bits of B
    or v2, v1  ; apply or: ~B (v2) + A (v1)
    rts

; check if A is less than B
;   inputs: v0 (A), v1 (B)
;   output: v0
llt:
    jsr lnot   ; invert bits of A
    and v0, v1 ; apply and: ~A (v0) . B (v1)
    rts

; check if A is less than or equal to B
;   inputs: v0 (A), v1 (B)
;   output: v0
llte:
    jsr lnot  ; invert bits of v0 (A)
    or v0, v1 ; apply or: ~A (v0) + B (v1)
    rts

check_sprite:
    .db 0x10, 0xa0, 0x40
    .equ CHECK_H 3

x_sprite:
    .db 0xa0, 0x40, 0xa0
    .equ X_H 3
