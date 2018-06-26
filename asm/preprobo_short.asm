.equ endOfLoop, 0x0209dfe8

# at 0209dfc4
#
# r0 = getpos -- initialized to 0
# r1 = putpos -- initialized to 0 (no longer used)
# r2 = string pointer
# r3 = dstptr
.thumb
startOfLoop:
# copy next character from src to dst
ldrsb   r4, [r2, r0]
strb    r4, [r3, r0]
add     r0, #1

# done if character was terminator
cmp     r4, #0
beq     endOfLoop

# character >= 0x80?
cmp     r4, #0x7F
# no: go to next
ble     startOfLoop
# yes: copy another one
ldrsb   r4, [r2, r0]
strb    r4, [r3, r0]
add     r0, #1
# loop to start
b startOfLoop
