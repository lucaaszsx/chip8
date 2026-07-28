.org 0x200

.equ MAX_DIGIT 0xf
.equ FONT_HEIGHT 5
.equ DIGIT_X 29
.equ DIGIT_Y 13

; entry point
start:
    ; reset v0 to be used as loop index counter
    mov v0, 0

    ; define number position on screen
    mov v1, 29 ; x
    mov v2, 13 ; y

    ; first loop call
    jsr loop

end:
    jmp end

loop:
    jsr wait

    cls

    ; draw the digit into the screen
    font v0
    draw v1, v2, FONT_HEIGHT

    ; check if the index counter was finished and return
    skne v0, MAX_DIGIT
    rts

    ; increments the index counter
    add v0, 1

    ; sets the delay timer
    mov v6, 0x3c ; 60 -> 1s
    sdelay v6

    jmp loop

; wait the delay timer (DT) reaches 0
wait:
    gdelay v6
    skeq v6, 0
    jmp wait
    rts
