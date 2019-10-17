
;========================================
; basic stuff
;========================================

.nds

; end of arm7 binary, location of our new code
arm7End equ 0x23a77e0

overlayBase equ 0x021d9360

;========================================
; defines
;========================================

getGlyphIndex equ 0x0208cb88
allocHeapMem equ 0x02008a18

fontStructCwdhOffset equ 0x284
printStructXSpacingOffset equ 0x44
globalMainFontPointer equ 0x021d5a28
globalSubFontPointer equ 0x021d5a2c
cwdhAdvanceWidthOffset equ 0x2
cacheEntryAdvanceWidthOffset equ 0x2C


;========================================
; overwrites
;========================================

.open "buildfiles/gamefiles/arm9.bin", 0x02000000

;========================================
; text speedup
;========================================

.org 0x0208b840
; decrement countdown timer by 2 instead of 1, doubling text speed
;subgt   r0, r0, #1
subgt   r0, r0, #2

;========================================
; no-longer-necessary jump table for
; individual hacks.
; originally, i was using gcc and had to
; insert the hacks into the binary
; manually, and this was used to provide
; a stable target for the injection
; points.
;========================================
  
;  .org 0x203e5a4
;  creditsXOffset:
;  b creditsXOffset_ext
;  creditsXOffset_end:
  
  .org 0x200d928
  b jumpBvmStringLookupHack
  
  .org 0x208b23c
  mov   r6, #48 ; 0x30
  
  .org 0x208b254
  bl    jumpAdvanceWidthStoreCachedHack
  
  .org 0x208b324
  bl      jumpCacheCheckWideSjisCheck
  
  .org 0x208b6c0
  b     jumpTextBoxWidthCalculationHack
  
  .org 0x208bae0
  bl    jumpNewCharacterAdvanceWidthHack
  
  .org 0x208cb94
  bl    jumpGlyphCheckWideSjisCheck
  
  .org 0x208cc28
  bl    jumpAdvanceWidthUpdateUncached
  
  .org 0x208cd04
  bl    jumpAdvanceWidthUpdateCached
  
  .org 0x2095360
  bl      jumpDialogueWidthStringWidth
  
  .org 0x209537c
  bl      jumpDialogueWidthStringWidth
  
  .org 0x2095398
  bl    jumpDialogueWidthStringWidth
  
  .org 0x209549c
  bl    jumpDialogueWidthTableCalcHack
  
  .org 0x209556c
  mov   r0, r2
  
  .org 0x209559c
  mov   r0, r2
  
  .org 0x2095608
  mov   r0, r2
  
  .org 0x2095638
  mov   r0, r2
  
  .org 0x20956c8
  mov   r0, r2
  
  .org 0x20956f8
  mov   r0, r2
  
  .org 0x20958b4
  b     jumpDialogueChoiceAdvanceHack
  
  ; even pixel rows
;  .org 0x208c64c
;  strb  r2, [r0, r1]
  ; odd pixel rows
;  .org 0x208c668
 ; strb  r2, [r0, r1]
;  nop

  .org 0x209bbcc
;  ldr   r0, [r4, #44]
  b     allocExtraTextCompositionBufferSpaceHack
  
  .org 0x208b6cc
  b     evenBoxWidthCalculationHack
  
  .org 0x208b62c
  b textBoxNewlineWidthCalculationHack
  
  .thumb
    ;=====
    ; always use the immediately following code
    ;=====
    .org 0x209dfac
    b 0x209dfbc

    ;=====
    ; at 0209dfc4
    ;
    ; r0 = getpos -- initialized to 0
    ; r1 = putpos -- initialized to 0 (no longer used)
    ; r2 = string pointer
    ; r3 = dstptr
    ;=====
    
    endOfPreproboLoop equ 0x0209dfe8
    
    .org 0x209dfc4
    @@loop:
      ; copy next character from src to dst
      ldrsb   r4, [r2, r0]
      strb    r4, [r3, r0]
      add     r0, #1

      ; done if character was terminator
      cmp     r4, #0
      beq     endOfPreproboLoop

      ; character >= 0x80?
      cmp     r4, #0x7F
      ; no: go to next
      ble     @@loop
      ; yes: copy another one
      ldrsb   r4, [r2, r0]
      strb    r4, [r3, r0]
      add     r0, #1
      ; loop to start
      b @@loop
      
      strb      r7, [r5, #3]
      add      r5, r1, #4
      add      r1, r1, #2
;      movs      r4, #49 ; 0x31
;      strb      r4, [r3, r5]
;      b       0x209dfc4
;      strb      r4, [r5, #2]
    
    .org 0x209dfe8
    mov        r0, #10
  .arm
  
  ;=====
  ; ??????
  ;=====
  .org 0x20e4a30
  .dw 0x9
  .dw 0x9
  .dw 0x9
  
.close

; pixels per text line in robot descriptions (summary, museum, etc.)
roboDescLineHeight equ 16-3

; magical value that causes screens with lowered lineheights not to
; glitch when text is switched out (e.g. museum, robodesc)
magicLineHeightGlitchFixValue equ 5

; offset of text "pages" from original x/y pos
charDescTextPageNewXShift equ -4
charDescTextPageNewYShift equ -4
roboDescTextPageNewXShift equ -4
roboDescTextPageNewYShift equ 0

.open "buildfiles/gamefiles/overlay/overlay_0004.bin", overlayBase

  .arm

  ;===============================
  ; museum lineheight
  ;===============================
  .org 0x21dc4fc
;    mov        r7, #16
  mov        r7, roboDescLineHeight
  
  ;===============================
  ; museum description base pos
  ;===============================
  .org 0x21dca6c
  ; x
  .dw 0x8a+roboDescTextPageNewXShift
  ; y
  .dw 0x0c+roboDescTextPageNewYShift
  ; endX
  .dw 0x10a+roboDescTextPageNewXShift
  ; endY
  .dw 0xcc+roboDescTextPageNewYShift

  ;===============================
  ; museum magic lineheight fix
  ;===============================
  .org 0x21dbfa0
;    mov        r0, #2
  mov        r0, magicLineHeightGlitchFixValue

.close

.open "buildfiles/gamefiles/overlay/overlay_0005.bin", overlayBase
  .thumb
  
    .org 0x21dd9e8
    nop
    
    ;===============================
    ; character album lineheight
    ;===============================
    .org 0x21db608
  ;    mov        r0, #16
    mov        r0, roboDescLineHeight
    
    ;===============================
    ; character album description base pos
    ;===============================
    .org 0x21df910
    ; x
    .dw 0x8a+charDescTextPageNewXShift
    ; y
    .dw 0x1a+charDescTextPageNewYShift
    ; endX
    .dw 0x10a+charDescTextPageNewXShift
    ; endY
    .dw 0xda+charDescTextPageNewYShift
    
    ;===============================
    ; magical fix for glitchy reloads when line height is decreased.
    ; i have absolutely no idea what this is doing or why it works...
    ; but apparently it does??
    ;===============================
    .org 0x21db374
  ;    mov        r0, #2
      mov        r0, magicLineHeightGlitchFixValue
    
  .arm
.close

.open "buildfiles/gamefiles/overlay/overlay_0008.bin", overlayBase
  .org 0x21da32c
  b     jumpNameEntryInitHack
.close

.open "buildfiles/gamefiles/overlay/overlay_0009.bin", overlayBase
  .thumb
    
    ;======================================
    ; roboselect character limits
    ;======================================
  
    ; remove 14-byte item limitation, parts screen 1
    .org 0x21da116
    nop
    
    ; remove 14-byte item limitation, parts screen 2
    .org 0x21dbf72
    nop
    nop
    nop
    
    ; remove 14-byte item limitation, robo screen
    ; TODO: does this cause problems?
    .org 0x21dfdd8
    nop
    
    ;======================================
    ; roboselect lineheights
    ;======================================
    
    ; lineheight of "puppet info" page
    .org 0x21e022a
;    mov        r0, #16
    mov        r0, roboDescLineHeight
    
    ; lineheight of "ability info" page
    .org 0x21e0294
;    mov        r0, #16
    mov        r0, roboDescLineHeight
    
;    ; lineheight of slot list header??
    .org 0x21e02fe
    mov        r0, #14
    
;    ; lineheight of slot list
    .org 0x21e034c
    mov        r0, #10
    
    ;======================================
    ; roboselect x/y
    ;======================================
    
    ; description page
    .org 0x21ea50c
    ; x
    .dw 0x8a+roboDescTextPageNewXShift
    ; y
    .dw 0x0c+roboDescTextPageNewYShift
    ; endX
    .dw 0x10a+roboDescTextPageNewXShift
    ; endY
    .dw 0xcc+roboDescTextPageNewYShift


    ; usage page
    .org 0x21ea53c
    ; x
    .dw 0x8a+roboDescTextPageNewXShift
    ; y
    .dw 0x0c+roboDescTextPageNewYShift
    ; endX
    .dw 0x10a+roboDescTextPageNewXShift
    ; endY
    .dw 0xcc+roboDescTextPageNewYShift


;    ; slots header
;    .org 0x21ea54c
;    ; x
;    .dw 0x6+roboDescTextPageNewXShift
;    ; y
;    .dw 0x3+roboDescTextPageNewYShift

;    ; slot list
;    .org 0x21ea56c
;    ; x
;    .dw 0x90+roboDescTextPageNewXShift
;    ; y
;    .dw 0x1c+roboDescTextPageNewYShift



;     .org 0x21e0208
;     ldr        r0, [sp, #36]
;;     mov        r0, 0x8a+roboDescTextPageNewXShift
;     .org 0x21e020c
;     ldr        r0, [sp, #40]
;     mov        r0, 0x0c



;    ; ROBODESC magic??
;    .org 0x21dfc66
;    mov        r0, #4+0

;    ; ROBOHELP magic??
;    .org 0x21dfccc
;    mov        r0, #4+0

;    ; ??? magic??
;    .org 0x21dfe36
;    mov        r0, #5+0
    
    ;======================================
    ; other
    ;======================================
    
    .org 0x21dd684
    ; Currently, this code is destined for 0x021dd684

    ; Routine that returns the glyph index for a given character
;    .equ getGlyphIndex, 0x0208cb88
;    .equ allocHeapMem, 0x02008a18
;
;    .equ fontStructCwdhOffset, 0x284
;    .equ printStructXSpacingOffset, 0x44
;    .equ globalMainFontPointer, 0x021d5a28
;    .equ cwdhAdvanceWidthOffset, 0x2
;    .equ cacheEntryAdvanceWidthOffset, 0x2C
    
    ; goes at 21dd684 (overlay 9: 0x4328)

    ; get row index from sp+24
    ldr     r2, [sp, #24]
    ; multiply row index by 836 (size of rowstruct)
    ldr     r1, =836
    mul     r2, r1

    ; get column index from sp+100
    ldr     r0, [sp, #100]
    ; multiply column index by 76 (size of colstruct)
    ldr     r1, =76
    mul     r1, r0
    ; add to row addr
    add     r2, r1

    ; get address of robostruct from sp+12
    ldr     r0, [sp, #12]
    ; add base offset to data
    add     r0, #0xB0

    ; add to base position
    add     r0, r2

    ; jump to removed crap
    blx     0x023A77F8

    ; get font pointer
    ;ldr     r1, =globalMainFontPointer
    ;ldr     r1, [r1]
    ; call jumpGetStringWidth (023A77F4) in ARM mode
    ;blx     0x023A77F4
    ; move #112 (width of printing area) to r1
    ; would #110 be better?
    ;mov     r1, #112
    ; subtract result
    ;sub     r1, r0
    ; divide by 2
    ;lsr     r1, #1

    ; recycled code from original function

    ; fetch something (base x-offset?)
    ldr     r2, [sp, #184]
    ; subtract 0x3b (center position)
    sub    r1, #59
    ; <<= 12
    lsl    r0, r1, #12
    ; add to something
    add    r0, r2, r0
    ; write to something
    str     r0, [sp, #184]

    ; jump past dead code
    b       0x021dd6b4

    .pool
    
    ;======================================
    ; extend amount of MissionSelect_U 457
    ; graphics that are copied into the
    ; main map menu, because by default
    ; only the portion that actually
    ; contains text is copied, even though
    ; space hase been allocated in the
    ; sheet for more space.
    ; also i spent a very long time
    ; rewriting my NSCR generator because
    ; i thought this was caused by a blank-
    ; tile optimization as opposed to
    ; using hardcoded constants for no
    ; reason. it was very upsetting.
    ;======================================
    
    .org 0x21ea83c
    
    ; galouye workshop
    .db "S_GB",0x00,0x00,0x00,0x00      ; name of jump target
    .dw 0x10    ; ?
    .dw 0x0     ; ?
    .dw 0x8+4   ; width in tiles?
    
    ; marionation gear association
    .db "500",0x00,0x00,0x00,0x00,0x00
    .dw 0x10
    .dw 0x2
    .dw 0xc
    
    ; parts shop
    .db "S_PS",0x00,0x00,0x00,0x00
    .dw 0x10
    .dw 0x4
    .dw 0xb+1
    
    ; museum
    .db "S_MU",0x00,0x00,0x00,0x00
    .dw 0x10
    .dw 0x6
    .dw 0x5+7
    
    ; wireless center
    .db "S_NC",0x00,0x00,0x00,0x00
    .dw 0x10
    .dw 0x8
    .dw 0xa+2
    
    ; back?
;    .db 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
;    .dw 0x10
;    .dw 0xc
;    .dw 0x5+7

  .arm
.close

.open "buildfiles/gamefiles/overlay/overlay_0010.bin", overlayBase

  .arm
  
  ;======================================
  ; shop robodesc lineheight
  ;======================================
  .org 0x21dd1b0
;    mov        r7, #16
    mov        r7, roboDescLineHeight
    
  ;===============================
  ; shop robodesc base pos
  ;===============================
  .org 0x21de7c8
  ; x
  .dw 0x8a+roboDescTextPageNewXShift
  ; y
  .dw 0x0c+roboDescTextPageNewYShift
  ; endX
  .dw 0x10a+roboDescTextPageNewXShift
  ; endY
  .dw 0xcc+roboDescTextPageNewYShift
  
  ;===============================
  ; shop robodesc magic lineheight fix
  ;===============================
  .org 0x21dcc70
;    mov        r0, #2
  mov        r0, magicLineHeightGlitchFixValue

.close

.open "buildfiles/gamefiles/arm7.bin", 0x02380000
  
  ;========================================
  ; NEW CODE
  ;========================================
  
  .org arm7End
  
  ;===========================================================================
  ; jump table for other routines, an inheritance from the previous
  ; iteration with GCC
  ;===========================================================================
  
  .align 4
  
  ; 0x023A77E0
  ; ?
  jumpAdvanceWidthUpdateUncached:       b advanceWidthUpdateUncached
  ; 0x023A77E4
  jumpAdvanceWidthUpdateCached:         b advanceWidthUpdateCached
  ; ?
  ; 0x023A77E8
  jumpAdvanceWidthStoreCachedHack:      b advanceWidthStoreCachedHack
  ; 0x023A77EC
  jumpCacheCheckWideSjisCheck:          b cacheCheckWideSjisCheck
  ; 0x023A77F0
  ; ?
  jumpGlyphCheckWideSjisCheck:          b glyphCheckWideSjisCheck
  ; 0x023A77F4
  jumpGetStringWidth:                   b getStringWidth
  ; 0x023A77F8
  jumpPrepRoboHackOverflow:             b prepRoboHackOverflow
  ; 0x023A77FC
  jumpTextBoxWidthCalculationHack:      b textBoxWidthCalculationHack
  ; 0x023A7800
  ; ?, but probably not?
  jumpBvmStringLookupHack:              b bvmStringLookupHack
  ; 0x023A7804
  jumpNameEntryInitHack:                b nameEntryInitHack
  ; 0x023A7808
  jumpDialogueChoiceAdvanceHack:        b dialogueChoiceAdvanceHack
  ; 0x023A780C
  jumpDialogueWidthStringWidth:         b dialogueWidthStringWidth
  ; 0x023A7810
  jumpDialogueWidthTableCalcHack:       b dialogueWidthTableCalcHack
  ; 0x023A7814
  jumpNewCharacterAdvanceWidthHack:     b newCharacterAdvanceWidthHack

  


  ; We can't do this. The routine this was hacked into is sometimes called
  ; from outside the standard printing routine, meaning no print struct exists.
  ; Ignore.
  advanceWidthUpdateUncached:
  ; save registers
  stmfd   sp!, {lr}
  ; do makeup work
  bl getGlyphIndex
  ; do update
  ;bl updatePrintStructAdvanceWidth
  ; ret
  ldmfd   sp!, {pc}

  ; We can't do this. The routine this was hacked into is sometimes called
  ; from outside the standard printing routine, meaning no print struct exists.
  ; Ignore.
  ;
  ; r4 = cacheinfo pointer
  ; r6 = font pointer
  ; r10 = print struct pointer
  advanceWidthUpdateCached:
  ; save registers
  stmfd   sp!, {r7, lr}

  ; get advance width of character
  ;ldr     r0, [r4, #cacheEntryAdvanceWidthOffset]

  ; write to print struct
  ;strh    r0, [r10, #printStructXSpacingOffset]

  ; do makeup work
  ldr     r0, [r4, #8]

  ; ret
  ldmfd   sp!, {r7, pc}
  
  ;===========================================================================
  ; Prerequisite: Change amount of memory allocated per cache struct
  ; from 2C to at least 30 (ARM9 8b23c)
  ;
  ; from 8b254
  ;
  ; r7 = pointer to target character's raw codepoint
  ;===========================================================================
  
  advanceWidthStoreCachedHack:
  stmfd   sp!, {r1-r5, lr}

  ; makeup work
  bl      allocHeapMem
  ; save allocation address -- we have to return this later
  mov     r4, r0

  ; fetch the global font pointer
  ldr     r0, =globalMainFontPointer

  ; set up parameters for getGlyphIndex
  ; r0 = fontstruct pointer
  ldr     r0, [r0]

  ; branch if global font pointer not NULL
  cmp     r0, #0
  bne     @@notDone

    ;=====
    ; The global font pointer may be NULL when this routine is called while
    ; the game boots. It's not emulated correctly in desmume, but crashes
    ; on real hardware without this check.
    ;
    ; Apparently, the game wants to cache some characters at startup, prior
    ; to the font being properly loaded(?) -- perhaps this is the default
    ; name used before a file is loaded. I don't even know what the characters
    ; in question are -- they seem to be hiragana -- but we can assume they're
    ; 14x14 monospace.
    ;=====
    
    mov     r0, #14
    str     r0, [r4,#cacheEntryAdvanceWidthOffset]
    b       @@done

  @@notDone:
  ; r5 = global font pointer
  mov     r5, r0
  ; r1 = pointer to character code
;  mov     r1, r9
  mov     r1, r7

  ; get index of glyph
  bl      getGlyphIndex

  ; look up CWDH entry for character
  ; r1 = global font pointer
  mov     r1, r5
  bl      getGlyphCwdhEntry

  ; get advance width from CWDH entry
  ldrb    r0, [r0,#cwdhAdvanceWidthOffset]
  and     r0, #0XFF

  ; write advance width to cache entry
  str     r0, [r4,#cacheEntryAdvanceWidthOffset]

  ; return allocation address
  @@done:
  mov     r0, r4
  ldmfd   sp!, {r1-r5, pc}

  .pool

  ;===========================================================================
  ; distinguish between 1-byte and 2-byte SJIS sequences when checking
  ; whether a character is cached
  ; 
  ; r2 = pointer to sjis character
  ; r3 = 0
  ; r12 = first character in sequence
  ;
  ; from 8b324
  ;===========================================================================
  
  cacheCheckWideSjisCheck:
  ; is first character <= 0x7F? (signed)
  cmp     r12, #0

  ; branch if no
  blt     @@notSjis

    ; replace first character on stack with 0,
    ; and load first character where the caller expects
    ; second to be
    str     r3, [sp, #4]
    mov     r2, r12
    b       @@done

  ; no: do makeup work
  @@notSjis:
  ldrsb r2, [r2, #1]

  ; ret
  @@done:
  bx lr

  ;===========================================================================
  ; in getGlyphIndex, distinguish 1- and 2-byte SJIS sequences
  ;
  ; r2 = first character
  ;
  ; from 8cb94
  ;===========================================================================
  
  glyphCheckWideSjisCheck:
  ; is first character <= 0x7F? (unsigned)
  cmp     r2, #0x7F

  ; branch if yes
  bhi     @@notSjis

    ; move first character to r3 (second character)
    ; and replace first character with zero
    mov     r3, r2
    mov     r2, #0
    b       @@done

  ; do makeup work
  @@notSjis:
  ldrb    r3, [r1, #1]

  ; ret
  @@done:
  bx lr


  ;##############
  ; subroutines #
  ;##############


  ;===========================================================================
  ; return address of glyph's CWDH entry
  ;
  ; r0 = target glyph
  ; r1 = font struct pointer
  ;===========================================================================
  
  getGlyphCwdhEntry:
  ; get address of CWDH for the font (adding 8 to skip header)
  ldr     r1, [r1, #fontStructCwdhOffset]
  add     r1, #8

  ; calculate offset of this glyph's info (multiply index by 3)
  mov r2, r0, lsl #1
  add r0, r2, r0
  add r0, r1

  ; ret
  bx lr

  ;===========================================================================
  ; r0 = index of target glyph
  ; r7 = font struct pointer
  ; r10 = print struct pointer
  ; 
  ; we need to use this to find the advance width for the target
  ; character
  ;===========================================================================

  updatePrintStructAdvanceWidth:

  ; get address of CWDH for the font (adding 8 to skip header)
  ldr     r1, [r7, #fontStructCwdhOffset]
  add     r1, #8

  ; calculate offset of this glyph's info (multiply index by 3)
  mov r2, r0, lsl #1
  add r2, r0
  add r1, r2

  ; get 1-byte advance width
  ldrb    r1, [r1, #cwdhAdvanceWidthOffset]
  and r1, r1, #0xFF

  ; write the advance width to the print struct
  strh r1, [r10, #printStructXSpacingOffset]

  ; ret
  bx lr

  ;===========================================================================
  ; calculate the width of a printed string in pixels
  ; !! does not handle escape sequences !!
  ; this is intended for use on e.g. robot names, which don't
  ; need them
  ; 
  ; r0 = string pointer
  ; r1 = font struct pointer
  ;===========================================================================
  
  getStringWidth:
  stmfd   sp!, {r2-r7, lr}

  ; r6 = string pointer
  mov     r6, r0
  ; r7 = font struct pointer
  mov     r7, r1

  ; r4 = total width
  mov     r4, #0

  @@loop:
    ; r5 = next byte in sequence
    ; break if terminator
    ldrb    r5, [r6]
    and     r5, #0xFF
    cmp     r5, #0
    beq     @@done

    ; get next character's glyph index
    mov     r0, r7
    mov     r1, r6
    bl      getGlyphIndex

    ; get the CWDH entry for the character
    mov     r1, r7
    bl      getGlyphCwdhEntry

    ; add the advance width to the total width
    ldrb    r0, [r0, #cwdhAdvanceWidthOffset]
    and     r0, #0xFF
    add     r4, r0
    
    ; check if this is a 1-byte or 2-byte sequence and advance stream accordingly
    cmp     r5, #0x7F
    addhi   r6, #2
    bhi     @@loopEnd
      add     r6, #1

    ; loop to start
    @@loopEnd:
    b       @@loop

  @@done:
  ; return computed width
  mov     r0, r4

  ; end of routine
  ldmfd   sp!, {r2-r7, lr}
  ; we may come here from thumb mode, so make sure we switch if necessary
  bx      lr

  ; some crap removed from prepRobo due to not fitting in-place
  prepRoboHackOverflow:
  stmfd   sp!, {lr}

  ldr     r1, =globalMainFontPointer
  ldr     r1, [r1]
  ; call jumpGetStringWidth (023A77F4) in ARM mode
  bl     jumpGetStringWidth
  ; move #118 (width of printing area) to r1
  mov     r1, #118
  ; subtract result
  sub     r1, r0
  ; divide by 2
  lsr     r1, #1

  ldmfd   sp!, {lr}
  ; we may come here from thumb mode, so make sure we switch if necessary
  bx      lr

  .pool

  ;===========================================================================
  ; from 0208b6c0
  ; r1 = source string pointer
  ; r5 = number of bytes in character sequence
  ; r8 = width up to current character
  ; r10 = position in string
  ;===========================================================================

  textBoxWidthCalculationHackRetAddr equ 0x0208b6c4
  
  textBoxWidthCalculationHack:
  
  ; the code we jump here from abuses lr, preventing this from being
  ; a function call
  ;stmfd   sp!, {r0-r7, r9-r12, lr}
  stmfd   sp!, {r0-r7, r9-r12, lr}

  ; fetch the global font pointer
  ldr     r0, =globalSubFontPointer

  ; set up parameters for getGlyphIndex
  ; r0 = fontstruct pointer
  ldr     r0, [r0]
  ; r1 = pointer to character code
  add     r1, r10

  ; get next character's glyph index
  bl      getGlyphIndex

  ; get the CWDH entry for the character
  ; r0 = target glyph
  ; r1 = font struct pointer
  ldr     r1, =globalSubFontPointer
  ldr     r1, [r1]
  bl      getGlyphCwdhEntry

  ; add the advance width to the total width
  ldrb    r0, [r0, #cwdhAdvanceWidthOffset]
  and     r0, #0xFF
  add     r8, r0

  ;ldmfd   sp!, {r0-r7, r9-r12, pc}
  ldmfd   sp!, {r0-r7, r9-r12, lr}
  b       textBoxWidthCalculationHackRetAddr

  .pool

  ;===========================================================================
  ; (oops never mind)
  ; ensure calculated text box width is always a multiple of 14
  ;
  ; r8 = calculated width
  ;
  ; from 0x208b6cc
  ;===========================================================================
  
  evenBoxWidthCalculationHack:
  
;  tst r8, #1
;  beq @@done
;    add r8, #1
;  @@done:
  
/*  push {r0-r1}
  ; crappy division by 14
  mov     r1, r8
  mov     r0, #0
  @@divLoop:
    sub     r1, #14
    cmp     r1, #0
    blt     @@divDone
      add     r0, #1
      b       @@divLoop
  @@divDone:
  ; if result was evenly divisible by 14, leave as-is
  cmp r1,-14
  beq @@done
    ; add 1
    add r0,#1
    ; crappy mult by 14
    mov r8,r0, lsl #4
    sub r8,r8,r0
    sub r8,r8,r0
  @@done:
  pop {r0-r1} */
  
;  tst r8,#0x1F
;  beq @@done
;    and r8,-0x1F
;    add r8,#32
;  @@done:
  
;  add r8,#2
  
  ; due to a quirk that can cause "spillover" around the edge of the
  ; resultant texture if certain widths are used,
  ; if the calculated width is a multiple of 32 or one or two less
  ; than that (e.g. 30-32, 62-64, 94-96...), round it up to
  ; the nearest multiple of 32 plus one
  push {r0}
    and r0, r8, 0x1F
    
    cmp r0, #0
    beq @@plus1
    
    cmp r0, #31
    beq @@plus2
    
    cmp r0, #30
    beq @@plus3
    
    b @@done
    
    @@plus3:
      add r8,1
      ; drop through
    @@plus2:
      add r8,1
      ; drop through
    @@plus1:
      add r8,1
      ; drop through
  @@done:
  pop {r0}
  
  ; make up work
  mov   r1, #0
  b 0x208b6d0
  
  .pool

  ;===========================================================================
  ; also do the above calculations after a newline in a text box string
  ;
  ; r8 = calculated width
  ; r9 = greatest width
  ;===========================================================================
  
  textBoxNewlineWidthCalculationHack:
  
  push {r0}
    and r0, r8, 0x1F
    
    cmp r0, #0
    beq @@plus1
    
    cmp r0, #31
    beq @@plus2
    
    cmp r0, #30
    beq @@plus3
    
    b @@done
    
    @@plus3:
      add r8,1
      ; drop through
    @@plus2:
      add r8,1
      ; drop through
    @@plus1:
      add r8,1
      ; drop through
  @@done:
  pop {r0}
  
  ; make up work
  cmp   r9, r8
  b 0x208b630
  
  .pool

  ;===========================================================================
  ; for some reason i have been unable to discern, the game has started
  ; sometimes overflowing the buffer it allocates to compose text for
  ; "window-type" boxes.
  ; while i haven't worked out the root cause, allocating some extra
  ; memory for the overflow to spill into keeps things working.
  ;===========================================================================
  
  allocExtraTextCompositionBufferSpaceHack:
  
  ; make up work (retrieve nominal buffer size)
  ldr   r0, [r4, #44]
  
  ; add some extra bytes to allocation size to allow for overflow
  ;
  ; how much is enough?? 64 seemed to work, then i got reports the
  ; game was crashing after seeing the "under construction" message
  ; in shops...
  ; testing showed that this would occur under these circumstances:
  ; 1. go to a mission with the objective "Beat the enemy!" and quit
  ;    (or at least, missions 100 and 101 both work for this)
  ; 2. go to a shop that's sold out, and view the "under construction"
  ;    message
  ; 3. try to exit the shop to the world map
  ;
  ; increasing the overflow allocation to at least 256 bytes fixes the
  ; problem (or at least this specific manifestation of it, under these
  ; specific circumstances).
  ; what is the root cause of this issue?? why is the overflow
  ; happening to begin with?
  ; well, let's see if we can get away with just being wasteful...
  ;
  ; i wouldn't imagine the overflow would exceed the size of
  ; a 32x16 OAM pattern (= 256 bytes), but let's not be stingy --
  ; that's what got us in trouble in the first place
  add r0,512
  
;  str   r0, [r4, #44]
  b 0x209bbd0
  
  ;===========================================================================
  ; from 0x0200d928
  ; r0 = string offset
  ; r1 = pointer to start of string block
  ; r2 = pointer to start of bvm
  ; goal: r0 = pointer to string
  ;===========================================================================

  bvmStringLookupHackRetAddr equ 0x0200d92c
  
  bvmStringLookupHack:
  stmfd   sp!, {r1-r3}

  ; multiply string number by 2
  lsl     r0, #1
  ; add to string block
  add     r2, r0, r1

  ; load low byte
  ldr     r0, [r2]
  and     r0, #0xFF

  ; load high byte
  ldr     r3, [r2, #1]
  and     r3, #0xFF

  ; combine into 16-bit offset
  lsl     r3, #8
  orr     r0, r3, r0

  ; add to string block pointer
  add     r0, r1

  ldmfd   sp!, {r1-r3}
  b       bvmStringLookupHackRetAddr

  nameEntryInitHackRetAddr equ 0x021da338

  ;===========================================================================
  ; from 0x021da32c (overlay 8 fcc)
  ; make character naming screen default to English
  ;===========================================================================
  
  nameEntryInitHack:
  ; set initial entry screen number to 2 (English)
  mov     r1, #2
  str     r1, [r0, #0x22c]
  ldr     r3, [r0, #0x22c]

  ; load expected zero value for r1 for intializing the rest of the struct
  mov     r1, #0
  ; do makeup work
  ;ldr     r2, [r11, #0x28]
  b       nameEntryInitHackRetAddr

  dialogueChoiceAdvanceHackRetAddr equ 0x020958bc
  
  ;===========================================================================
  ; from 0x020958b4
  ;
  ; r6 = x-position
  ; r7 = position in printstring
  ; r8 = pointer to start of printstring
  ; 
  ;
  ; goal: increase r7 by number of bytes in next character,
  ;       add character's advance width to r6
  ;===========================================================================

  dialogueChoiceAdvanceHack:
  stmfd   sp!, {r4-r5}

  ; get advance width of character

  ; get print struct pointer
  ldr     r0, [r4, #0xA8]

  ; get font struct pointer
  ldr     r0, [r0, #4]
  mov     r5, r0

  ; get character pointer
;  mov     r1, r8
;  add     r1, r7
  add r1, r7, r8

  ; r0 = font struct
  ; r1 = character pointer
  ; look up glyph index
  bl      getGlyphIndex

  ; get CWDH entry
  mov     r1, r5
  bl      getGlyphCwdhEntry

  ; add advance width to total
  ldrb    r0, [r0, #cwdhAdvanceWidthOffset]
  ;and     r0, #0xFF
  add     r6, r0

  ; advance byte position

  ; get first character
  ldrsb    r0, [r8, r7]

  ; 0-7f = 1-byte sequence
  ; 80-ff = 2-byte sequence
  cmp     r0, #0
  addge   r7, #0x01
  addlt   r7, #0x02

  ldmfd   sp!, {r4-r5}
  b       dialogueChoiceAdvanceHackRetAddr
  
  .pool

  ;===========================================================================
  ; r0 = string pointer
  ;
  ; goal: return width in pixels
  ;===========================================================================
  
  dialogueWidthStringWidth:
  stmfd   sp!, {lr}

  ; get subfont pointer
  ldr     r1, =globalSubFontPointer
  ldr     r1, [r1]

  ; compute width
  bl      getStringWidth

  ldmfd   sp!, {pc}
  
  .pool
  
  ;===========================================================================
  ; poor man's division by 14 (result is +1 because that's what we need)
  ; r8 = dividend
  ;===========================================================================

  dialogueWidthTableCalcHack:
  stmfd   sp!, {r0-r1, lr}
  mov     r1, r8
  mov     r0, #0
  @@loop:
    sub     r1, #14
    add     r0, #1
    cmp     r1, #0
    bgt     @@loop
  mov     r8, r0
  ldmfd   sp!, {r0-r1, pc}
  
  .pool
  
  ;===========================================================================
  ; from 0208bae0
  ;
  ; r1 = print pointer
  ; r2 = x-position
  ; r10 = print struct
  ;===========================================================================

  basicCharacterPrint equ 0x0208c040
  getCachedCharacterAddress equ 0x0208b2e0
  someDialogueCacheCheckConstant equ 0x021d5a34
  
  newCharacterAdvanceWidthHack:

  stmfd   sp!, {lr}

    ; this is a hideous, horrible hack and i hate myself for doing it, but
    
    ; print all characters 1 pixel to the right of where they should be,
    ; EXCEPT SHIFT-JIS DIGITS.
    ; offsetting by 1 works around an issue where the leftmost row of the
    ; font outline will be duplicated on certain screens (on real hardware
    ; -- not emulated correctly in desmume).
    ; not doing numbers prevents the game from crashing when it displays
    ; money totals (e.g. in shops).
    
    ; add 1 to x-position
    add     r2, #1

    ; oh hey i think we don't need this any more due to the new
    ; composition buffer overflow prevention hack.
    ; i hope.
    ; because otherwise it makes names with digits in them look
    ; really bad.
/*    stmfd   sp!, {r0}
    
      ; check if a SJIS digit (824f-8258)
    
      ; get first character
      ldrb    r0, [r1]
      
      ; branch if not 0x82
      cmp     r0, #0x82
      bne     @@skip
      
      ; get second character
      ldrb    r0, [r1, #1]
      and     r0, #0xFF
      
      ; branch if < 4f
      cmp     r0, #0x4f
      blo     @@skip
      
      ; branch if > 0x58
      cmp     r0, #0x58
      bhi     @@skip
      
        ; move to original position
        sub     r2, #1

    @@skip:
    ldmfd   sp!, {r0} */

    ; save important stuff
    stmfd   sp!, {r0-r7}
    ; save print struct due to stupidity
    stmfd   sp!, {r0-r1}
    
      ; pre-do the check to see if this character is cached
      ; get font pointer
      ldr     r0, [r10, #4]
      ; do usual steps (copied from existing code)
      ldr     r2, =someDialogueCacheCheckConstant
      mov     r6, r0
      ldr     r0, [r2]
      mov     r5, r1
      ldr     r3, [r0]
      ldr     r1, [r6, #132]
      ldr     r3, [r3, #28]
      mov     r2, r5
      bl      getCachedCharacterAddress
      
      ; nonzero result means character is cached
      movs    r4, r0
      beq     @@zero
      
        ; cached character
        ; get advance width of character
        ldr     r0, [r4, #cacheEntryAdvanceWidthOffset]
        ; write to print struct
        strh    r0, [r10, #printStructXSpacingOffset]
        ; done
        ldmfd   sp!, {r0-r1}
        b       @@done
      
      ; zero result: non-cached character, do standard lookup
      @@zero:
      ; get print struct pointer
      ldmfd   sp!, {r0-r1}
      ; get font pointer
      ldr     r7, [r10, #4]
      mov     r0, r7
      ; get glyph index
      bl getGlyphIndex
      ; do update
      bl updatePrintStructAdvanceWidth
      

    ; restore important stuff
    @@done:
    ldmfd   sp!, {r0-r7}
    
    ; make up work
    bl      basicCharacterPrint

  ldmfd   sp!, {pc}
  
  .pool
  
.close

