.org 0x200 ; initial address of the rom

.equ FONT_HEIGHT 5

start:          ; entry point
    mov v0, 0   ; loop index counter
    mov v1, 20  ; loop max iterations
    skeq v0, v1 ; skip if loop already fisnish (v0=v1)
    add v0, 1   ; increments loop by 1

    ; draw "1" for the user (feedback)
    mov v2, 0                ; x
    mov v3, 0                ; y
    mov v4, 1                ; wanted char to be displayed
    font v4                  ; sets I to address to the sprite
    draw v2, v3, FONT_HEIGHT ; draws the character

end:
    jmp end
