.org 0x200 ; initial address of the rom

.equ FONT_HEIGHT 5

start:           ; entry point
    mov v0, 0    ; loop counter
    mov v1, 0xfe ; max iterations

loop:
    skne v0, v1 ; skip if v0 isn't equals to v1
    jmp end

    ; loop logic
    add v0, 1 ; increments the counter
    jmp loop

end:
    jmp end
