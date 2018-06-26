# Currently, this code is destined for 0x023A77E0
# (arm7.bin 0x277e0)

# Routine that returns the glyph index for a given character
.equ getGlyphIndex, 0x0208cb88
.equ allocHeapMem, 0x02008a18

.equ fontStructCwdhOffset, 0x284
.equ printStructXSpacingOffset, 0x44
.equ globalMainFontPointer, 0x021d5a28
.equ globalSubFontPointer, 0x021d5a2c
.equ cwdhAdvanceWidthOffset, 0x2
.equ cacheEntryAdvanceWidthOffset, 0x2C

# Jump table for routines directly called via modifications to the original
# code.
# Here for convenience, so I don't have to keep manually updating the
# injection points time I make a change.
hackJumpTable:
  # 0x023A77E0
  # ?
  jumpAdvanceWidthUpdateUncached:       b advanceWidthUpdateUncached
  jumpAdvanceWidthUpdateCached:         b advanceWidthUpdateCached
  # ?
  jumpAdvanceWidthStoreCachedHack:      b advanceWidthStoreCachedHack
  jumpCacheCheckWideSjisCheck:          b cacheCheckWideSjisCheck
  # 0x023A77F0
  # ?
  jumpGlyphCheckWideSjisCheck:          b glyphCheckWideSjisCheck
  jumpGetStringWidth:                   b getStringWidth
  jumpPrepRoboHackOverflow:             b prepRoboHackOverflow
  jumpTextBoxWidthCalculationHack:      b textBoxWidthCalculationHack
  # 0x023A7800
  # ?, but probably not?
  jumpBvmStringLookupHack:              b bvmStringLookupHack
  jumpNameEntryInitHack:                b nameEntryInitHack
  jumpDialogueChoiceAdvanceHack:        b dialogueChoiceAdvanceHack
  jumpDialogueWidthStringWidth:         b dialogueWidthStringWidth
  # 0x023A7810
  jumpDialogueWidthTableCalcHack:       b dialogueWidthTableCalcHack
  jumpNewCharacterAdvanceWidthHack:     b newCharacterAdvanceWidthHack


# We can't do this. The routine this was hacked into is sometimes called
# from outside the standard printing routine, meaning no print struct exists.
# Ignore.
advanceWidthUpdateUncached:
# save registers
stmfd   sp!, {lr}
# do makeup work
bl getGlyphIndex
# do update
#bl updatePrintStructAdvanceWidth
# ret
ldmfd   sp!, {pc}

# We can't do this. The routine this was hacked into is sometimes called
# from outside the standard printing routine, meaning no print struct exists.
# Ignore.
#
# r4 = cacheinfo pointer
# r6 = font pointer
# r10 = print struct pointer
advanceWidthUpdateCached:
# save registers
stmfd   sp!, {r7, lr}

# get advance width of character
#ldr     r0, [r4, #cacheEntryAdvanceWidthOffset]

# write to print struct
#strh    r0, [r10, #printStructXSpacingOffset]

# do makeup work
ldr     r0, [r4, #8]

# ret
ldmfd   sp!, {r7, pc}

# Prerequisite: Change amount of memory allocated per cache struct
# from 2C to at least 30 (ARM9 8b23c)
#
# from 8b254
advanceWidthStoreCachedHack:
stmfd   sp!, {r1-r5, lr}

# makeup work
bl      allocHeapMem
# save allocation address -- we have to return this later
mov     r4, r0

# fetch the global font pointer
ldr     r0, =globalMainFontPointer

# set up parameters for getGlyphIndex
# r0 = fontstruct pointer
ldr     r0, [r0]

# branch if global font pointer not NULL
cmp     r0, #0
bne     1f

# The global font pointer may be NULL when this routine is called while
# the game boots. It's not emulated correctly in desmume, but crashes
# on real hardware without this check.
#
# Apparently, the game wants to cache some characters at startup, prior
# to the font being properly loaded(?). I don't even know what the characters
# in question are -- they seem to be hiragana -- but we can assume they're
# 14x14 monospace.
mov     r0, #14
str     r0, [r4,#cacheEntryAdvanceWidthOffset]
b       0f

1:
mov     r5, r0
# r1 = pointer to character code
mov     r1, r9

# get index of glyph
bl      getGlyphIndex

# look up CWDH entry for character
mov     r1, r5
bl      getGlyphCwdhEntry

# get advance width from CWDH entry
ldrb    r0, [r0,#cwdhAdvanceWidthOffset]
and     r0, #0XFF

# write advance width to cache entry
str     r0, [r4,#cacheEntryAdvanceWidthOffset]

# return allocation address
0:
mov     r0, r4
ldmfd   sp!, {r1-r5, pc}

.pool

# distinguish between 1-byte and 2-byte SJIS sequences when checking
# whether a character is cached
# 
# r2 = pointer to sjis character
# r3 = 0
# r12 = first character in sequence
#
# from 8b324
cacheCheckWideSjisCheck:
# is first character <= 0x7F? (signed)
cmp     r12, #0

# branch if no
blt     1f

# replace first character on stack with 0,
# and load first character where the caller expects
# second to be
str     r3, [sp, #4]
mov     r2, r12
b       2f

# no: do makeup work
1:
ldrsb r2, [r2, #1]

# ret
2:
bx lr

# in getGlyphIndex, distinguish 1- and 2-byte SJIS sequences
#
# r2 = first character
#
# from 8cb94
glyphCheckWideSjisCheck:
# is first character <= 0x7F? (unsigned)
cmp     r2, #0x7F

# branch if yes
bhi     1f

# move first character to r3 (second character)
# and replace first character with zero
mov     r3, r2
mov     r2, #0
b       2f

# do makeup work
1:
ldrb    r3, [r1, #1]

# ret
2:
bx lr


###############
# subroutines #
###############


# return address of glyph's CWDH entry
#
# r0 = target glyph
# r1 = font struct pointer
getGlyphCwdhEntry:
# get address of CWDH for the font (adding 8 to skip header)
ldr     r1, [r1, #fontStructCwdhOffset]
add     r1, #8

# calculate offset of this glyph's info (multiply index by 3)
mov r2, r0, lsl #1
add r0, r2, r0
add r0, r1

# ret
bx lr




updatePrintStructAdvanceWidth:
# r0 = index of target glyph
# r7 = font struct pointer
# r10 = print struct pointer
# 
# we need to use this to find the advance width for the target
# character

# get address of CWDH for the font (adding 8 to skip header)
ldr     r1, [r7, #fontStructCwdhOffset]
add     r1, #8

# calculate offset of this glyph's info (multiply index by 3)
mov r2, r0, lsl #1
add r2, r0
add r1, r2

# get 1-byte advance width
ldrb    r1, [r1, #cwdhAdvanceWidthOffset]
and r1, r1, #0xFF

# write the advance width to the print struct
strh r1, [r10, #printStructXSpacingOffset]

# ret
bx lr


# calculate the width of a printed string in pixels
# !! does not handle escape sequences !!
# this is intended for use on e.g. robot names, which don't
# need them
# 
# r0 = string pointer
# r1 = font struct pointer
getStringWidth:
stmfd   sp!, {r2-r7, lr}

# r6 = string pointer
mov     r6, r0
# r7 = font struct pointer
mov     r7, r1

# r4 = total width
mov     r4, #0

# loop start
1:
  # r5 = next byte in sequence
  # break if terminator
  ldrb    r5, [r6]
  and     r5, #0xFF
  cmp     r5, #0
  beq     2f

  # get next character's glyph index
  mov     r0, r7
  mov     r1, r6
  bl      getGlyphIndex

  # get the CWDH entry for the character
  mov     r1, r7
  bl      getGlyphCwdhEntry

  # add the advance width to the total width
  ldrb    r0, [r0, #cwdhAdvanceWidthOffset]
  and     r0, #0xFF
  add     r4, r0
  
  # check if this is a 1-byte or 2-byte sequence and advance stream accordingly
  cmp     r5, #0x7F
  addhi   r6, #2
  bhi     3f
  add     r6, #1

  # loop to start
  3:
  b       1b

2:
# return computed width
mov     r0, r4

# end of routine
ldmfd   sp!, {r2-r7, lr}
# we may come here from thumb mode, so make sure we switch if necessary
bx      lr

# some crap removed from prepRobo due to not fitting in-place
prepRoboHackOverflow:
stmfd   sp!, {lr}

ldr     r1, =globalMainFontPointer
ldr     r1, [r1]
# call jumpGetStringWidth (023A77F4) in ARM mode
bl     jumpGetStringWidth
# move #118 (width of printing area) to r1
mov     r1, #118
# subtract result
sub     r1, r0
# divide by 2
lsr     r1, #1

ldmfd   sp!, {lr}
# we may come here from thumb mode, so make sure we switch if necessary
bx      lr

.pool

.equ textBoxWidthCalculationHackRetAddr, 0x0208b6c4

# from 0208b6c0
# r1 = source string pointer
# r5 = number of bytes in character sequence
# r8 = width up to current character
# r10 = position in string
textBoxWidthCalculationHack:
# the code we jump here from abuses lr, preventing this from being
# a function call
#stmfd   sp!, {r0-r7, r9-r12, lr}
stmfd   sp!, {r0-r7, r9-r12, lr}

# fetch the global font pointer
ldr     r0, =globalSubFontPointer

# set up parameters for getGlyphIndex
# r0 = fontstruct pointer
ldr     r0, [r0]
# r1 = pointer to character code
add     r1, r10

# get next character's glyph index
bl      getGlyphIndex

# get the CWDH entry for the character
# r0 = target glyph
# r1 = font struct pointer
ldr     r1, =globalSubFontPointer
ldr     r1, [r1]
bl      getGlyphCwdhEntry

# add the advance width to the total width
ldrb    r0, [r0, #cwdhAdvanceWidthOffset]
and     r0, #0xFF
add     r8, r0

#ldmfd   sp!, {r0-r7, r9-r12, pc}
ldmfd   sp!, {r0-r7, r9-r12, lr}
b       textBoxWidthCalculationHackRetAddr

.pool

.equ bvmStringLookupHackRetAddr, 0x0200d92c
# from 0x0200d928
# r0 = string offset
# r1 = pointer to start of string block
# r2 = pointer to start of bvm
# goal: r0 = pointer to string
bvmStringLookupHack:
stmfd   sp!, {r1-r3}

# multiply string number by 2
lsl     r0, #1
# add to string block
add     r2, r0, r1

# load low byte
ldr     r0, [r2]
and     r0, #0xFF

# load high byte
ldr     r3, [r2, #1]
and     r3, #0xFF

# combine into 16-bit offset
lsl     r3, #8
orr     r0, r3, r0

# add to string block pointer
add     r0, r1

ldmfd   sp!, {r1-r3}
b       bvmStringLookupHackRetAddr

.equ nameEntryInitHackRetAddr, 0x021da338

# from 0x021da32c (overlay 8 fcc)
# make character naming screen default to English
nameEntryInitHack:
# set initial entry screen number to 2 (English)
mov     r1, #2
str     r1, [r0, #0x22c]
ldr     r3, [r0, #0x22c]

# load expected zero value for r1 for intializing the rest of the struct
mov     r1, #0
# do makeup work
#ldr     r2, [r11, #0x28]
b       nameEntryInitHackRetAddr

.equ dialogueChoiceAdvanceHackRetAddr, 0x020958bc

dialogueChoiceAdvanceHack:
# from 0x020958b4
#
# r6 = x-position
# r7 = position in printstring
# r8 = pointer to start of printstring
# 
#
# goal: increase r7 by number of bytes in next character,
#       add character's advance width to r6
stmfd   sp!, {r4-r5}

# get advance width of character

# get print struct pointer
ldr     r0, [r4, #0xA8]

# get font struct pointer
ldr     r0, [r0, #4]
mov     r5, r0

# get character pointer
mov     r1, r8
add     r1, r7

# r0 = font struct
# r1 = character pointer
# look up glyph index
bl      getGlyphIndex

# get CWDH entry
mov     r1, r5
bl      getGlyphCwdhEntry

# add advance width to total
ldrb    r0, [r0, #cwdhAdvanceWidthOffset]
#and     r0, #0xFF
add     r6, r0

# advance byte position

# get first character
ldrsb    r0, [r8, r7]

# 0-7f = 1-byte sequence
# 80-ff = 2-byte sequence
cmp     r0, #0
addge   r7, #0x01
addlt   r7, #0x02

ldmfd   sp!, {r4-r5}
b       dialogueChoiceAdvanceHackRetAddr

# r0 = string pointer
#
# goal: return width in pixels
dialogueWidthStringWidth:
stmfd   sp!, {lr}

# get subfont pointer
ldr     r1, =globalSubFontPointer
ldr     r1, [r1]

# compute width
bl      getStringWidth

ldmfd   sp!, {pc}

dialogueWidthTableCalcHack:
# poor man's division by 14 (result is +1 because that's what we need)
# r8 = dividend
stmfd   sp!, {r0-r1, lr}
mov     r1, r8
mov     r0, #0
0:
  sub     r1, #14
  add     r0, #1
  cmp     r1, #0
  bgt     0b
mov     r8, r0
ldmfd   sp!, {r0-r1, pc}



.equ basicCharacterPrint, 0x0208c040
.equ getCachedCharacterAddress, 0x0208b2e0
.equ someDialogueCacheCheckConstant, 0x021d5a34
# from 0208bae0
#
# r1 = print pointer
# r2 = x-position
# r10 = print struct
newCharacterAdvanceWidthHack:

stmfd   sp!, {lr}

  # this is a hideous, horrible hack and i hate myself for doing it, but
  
  # print all characters 1 pixel to the right of where they should be,
  # EXCEPT SHIFT-JIS DIGITS.
  # offsetting by 1 works around an issue where the leftmost row of the
  # font outline will be duplicated on certain screens (on real hardware
  # -- not emulated correctly in desmume).
  # not doing numbers prevents the game from crashing when it displays
  # money totals (e.g. in shops).
  
  # add 1 to x-position
  add     r2, #1

  stmfd   sp!, {r0}
  
    # check if a SJIS digit (824f-8258)
  
    # get first character
    ldrb    r0, [r1]
    
    # branch if not 0x82
    cmp     r0, #0x82
    bne     2f
    
    # get second character
    ldrb    r0, [r1, #1]
    and     r0, #0xFF
    
    # branch if < 4f
    cmp     r0, #0x4f
    blo     2f
    
    # branch if > 0x58
    cmp     r0, #0x58
    bhi     2f
    
    # move to original position
    add     r2, #-1

  2:
  ldmfd   sp!, {r0}

  # save important stuff
  stmfd   sp!, {r0-r7}
  # save print struct due to stupidity
  stmfd   sp!, {r0-r1}
  
    # pre-do the check to see if this character is cached
    # get font pointer
    ldr     r0, [r10, #4]
    # do usual steps (copied from existing code)
    ldr     r2, =someDialogueCacheCheckConstant
    mov     r6, r0
    ldr     r0, [r2]
    mov     r5, r1
    ldr     r3, [r0]
    ldr     r1, [r6, #132]
    ldr     r3, [r3, #28]
    mov     r2, r5
    bl      getCachedCharacterAddress
    
    # nonzero result means character is cached
    movs    r4, r0
    beq     0f
    
    # cached character
    # get advance width of character
    ldr     r0, [r4, #cacheEntryAdvanceWidthOffset]
    # write to print struct
    strh    r0, [r10, #printStructXSpacingOffset]
    # done
    ldmfd   sp!, {r0-r1}
    b       1f
    
    # zero result: non-cached character, do standard lookup
    0:
    # get print struct pointer
    ldmfd   sp!, {r0-r1}
    # get font pointer
    ldr     r7, [r10, #4]
    mov     r0, r7
    # get glyph index
    bl getGlyphIndex
    # do update
    bl updatePrintStructAdvanceWidth
    

  # restore important stuff
  1:
  ldmfd   sp!, {r0-r7}
  
  # make up work
  bl      basicCharacterPrint

ldmfd   sp!, {pc}



