# Currently, this code is destined for 0x021dd684

# Routine that returns the glyph index for a given character
.equ getGlyphIndex, 0x0208cb88
.equ allocHeapMem, 0x02008a18

.equ fontStructCwdhOffset, 0x284
.equ printStructXSpacingOffset, 0x44
.equ globalMainFontPointer, 0x021d5a28
.equ cwdhAdvanceWidthOffset, 0x2
.equ cacheEntryAdvanceWidthOffset, 0x2C
  

# goes at 21dd684 (overlay 9: 0x4328)
.thumb

# get row index from sp+24
ldr     r2, [sp, #24]
# multiply row index by 836 (size of rowstruct)
ldr     r1, =836
mul     r2, r1

# get column index from sp+100
ldr     r0, [sp, #100]
# multiply column index by 76 (size of colstruct)
ldr     r1, =76
mul     r1, r0
# add to row addr
add     r2, r1

# get address of robostruct from sp+12
ldr     r0, [sp, #12]
# add base offset to data
add     r0, #0xB0

# add to base position
add     r0, r2

# jump to removed crap
blx     0x023A77F8

# get font pointer
#ldr     r1, =globalMainFontPointer
#ldr     r1, [r1]
# call jumpGetStringWidth (023A77F4) in ARM mode
#blx     0x023A77F4
# move #112 (width of printing area) to r1
# would #110 be better?
#mov     r1, #112
# subtract result
#sub     r1, r0
# divide by 2
#lsr     r1, #1

# recycled code from original function

# fetch something (base x-offset?)
ldr     r2, [sp, #184]
# subtract 0x3b (center position)
sub    r1, #59
# <<= 12
lsl    r0, r1, #12
# add to something
add    r0, r2, r0
# write to something
str     r0, [sp, #184]

# jump past dead code
b       0x021dd6b4

.pool
