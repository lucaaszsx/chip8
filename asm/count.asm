.code
    mov v0, 0
    jsr add
    jmp code

.add
    add v0, 1
    ret