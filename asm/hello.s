.org 0x200

.equ FONT_HEIGHT 3

start:
    ; draw "he"
    mov v0, 0
    mov v1, 0
    mvi s_he
    draw v0, v1, FONT_HEIGHT

    ; draw "llo"
    add v0, 8
    mvi s_llo
    draw v0, v1, FONT_HEIGHT

end:
    jmp end

s_he:
    .db 0xae, 0xec, 0xae

s_llo:
    .db 0x93, 0x93, 0xdb

.end

hello, hru? this is a text after the ".end" directive and shoudn't be parsed.
here you can define comments, descriptions, etc.
