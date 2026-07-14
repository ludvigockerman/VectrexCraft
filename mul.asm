;------------------------------------------------
; mul8
;
; Input:
;   A = första unsigned char
;   B = andra unsigned char
;
; Output:
;   D = unsigned int (16 bit)
;
; Clobbers:
;   A,B,D
;------------------------------------------------

        .module mul8
        .area code

        .globl _mul8

_mul8:
        lda 3,s       ; argument 1
        ldb 5,s       ; argument 2
        mul           ; A * B -> D
        rts