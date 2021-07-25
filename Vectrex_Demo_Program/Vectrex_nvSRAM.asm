;***************************************************************************
; Vectrex nvSRAM Demo program (by Dan Siewers)
; This program doesn't make use of User RAM.
; Instead, it uses what is normaly Vctrex ROM space for RAM.
; This program won't work with a PROM, must use some type of SRAM
; When using ith nvSRAM, variable states will be maintained between power cycles
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.
;***************************************************************************
;***************************************************************************
; DEFINE SECTION
;***************************************************************************
; load vectrex bios routine definitions
                    INCLUDE  "VECTREX.I"                  ; vectrex function includes
;***************************************************************************
; Variable / RAM SECTION
;***************************************************************************
; insert your variables (RAM usage) in the BSS section
; user RAM starts at $c880... but...
; This program doesn't use RAM outside of the Vectrex BIOS functions
; With nvSRAM, we don't need no stinkin' user RAM!
                    BSS      
                    ORG      $c880                        ; start of our ram space 
;***************************************************************************
; HEADER SECTION
;***************************************************************************
; The cartridge ROM starts at address 0
                    CODE     
                    ORG      0 
; the first few bytes are mandatory, otherwise the BIOS will not load
; the ROM file, and will start MineStorm instead
                    DB       "g GCE 2021", $80            ; 'g' is copyright sign
                    DW       music1                       ; music from the rom 
                    DB       $F8, $50, $40, -$40          ; hight, width, rel y, rel x (from 0,0) 
                    DB       "NVSRAM DEMO", $80           ; some game information, ending with $80
                    DB       0                            ; end of game header 
;***************************************************************************
; CODE SECTION
;***************************************************************************
; here the cartridge program starts off
                    jsr      Read_Btns                    ; set initial button state 
                    lda      #75                          ; top of main screen text 
                    sta      message_Y 
                    lda      #200                         ; set text move counter 
                    sta      img_persistence_counter 
                    lda      #0                           ; set zero state of button 4 
                    sta      button_4_state 
main: 
                    jsr      Wait_Recal                   ; Vectrex BIOS recalibration 
                    jsr      Intensity_5F                 ; Sets the intensity of the 
                                                          ; vector beam to $5f 
                    lda      #$F8                         ; set hight of image 
                    sta      Vec_Text_Height 
                    lda      #$50                         ; set width of image 
                    sta      Vec_Text_Width 
                    lda      message_Y                    ; get Y position of first row of text 
                    pshs     a                            ; save it 
                    ldb      #-40                         ; X position of first row of text 
                    jsr      Moveto_d                     ; move to that position 
                    ldu      #message_1                   ; display top row message 
                    jsr      Print_Str 
                    lda      #-20                         ; line feed for next line of text 
                    adda     message_Y 
                    sta      message_Y                    ; save Y position of next line of text 
                    ldb      #-40                         ; X position for next line of text 
                    jsr      Moveto_d                     ; move to that position 
                    ldu      #message_2                   ; display next line of text 
                    jsr      Print_Str 
                    lda      #-20                         ; line feed for next line of text 
                    adda     message_Y 
                    sta      message_Y                    ; save Y position of next line of text 
                    ldb      #-40                         ; X position for next line of text 
                    jsr      Moveto_d                     ; move to that position 
                    ldu      #message_3                   ; display next line of text 
                    jsr      Print_Str 
                    lda      #-20                         ; line feed for next line of text 
                    adda     message_Y 
                    sta      message_Y                    ; save Y position of next line of text 
                    ldb      #-40                         ; X position for next line of text 
                    jsr      Moveto_d                     ; move to that position 
                    ldu      #message_4                   ; display next line of text 
                    jsr      Print_Str 
                    puls     a                            ; get back original Y position of top line of text 
                    sta      message_Y 
                    inc      img_persistence_counter      ; increment the delay counter 
                    bne      check_buttons                ; counter done? 
                    lda      #250                         ; yes, reset counter 
                    sta      img_persistence_counter 
                    lda      img_direction_main           ; what's the current direction of text movement? 
                    bne      main_screen_up               ; going up 
                    dec      message_Y                    ; going down, keep going down 
                    lda      message_Y 
                    cmpa     #-11                         ; reached bottom of screen? 
                    bne      main_screen_end_loop         ; nope keep moving down 
                    inc      img_direction_main           ; yes, time to go back up 
                    inca                                  ; make sure we aren't too far down 
                    bra      main_screen_end_loop         ; go display text 

main_screen_up: 
                    inc      message_Y                    ; move text up by one line 
                    lda      message_Y 
                    cmpa     #76                          ; have we reached top of screen? 
                    bne      main_screen_end_loop         ; no, keep moving up 
                    dec      img_direction_main           ; yes, flag that we are moving down 
                    deca                                  ; make sure we aren't too far down 
main_screen_end_loop: 
                    sta      message_Y                    ; save the new text Y position 
check_buttons: 
check_button_4: 
                    jsr      get_button                   ; get status of all buttons 
                    cmpa     #4                           ; button 4 pressed? 
                    bne      check_button_1               ; nope, keep checking 
                    lda      last_state                   ; yep, get last image shown 
check_button_1: 
                    cmpa     #1                           ; button 1 pressed from last check? 
                    bne      check_button_2               ; nope, keep checking 
                    sta      last_state                   ; save this button press 
                    lda      #255                         ; reset the draw persistence counter 
                    sta      img_persistence_counter 
                    jsr      show_fury                    ; show Fury logo 
check_button_2: 
                    cmpa     #2                           ; button 2 pressed? 
                    bne      check_button_3               ; nope, go check button 3 
                    sta      last_state                   ; save this button press 
                    lda      #255                         ; reset the draw persistence counter 
                    sta      img_persistence_counter 
                    jsr      show_zanti                   ; show Zanti 
check_button_3: 
                    cmpa     #3                           ; button 3 pressed? 
                    bne      back_to_main_loop            ; nope, go back to the top of loop 
                    sta      last_state                   ; save this button press 
                    lda      #255                         ; reset the draw persistence counter 
                    sta      img_persistence_counter 
                    jsr      show_nikki                   ; show Nikki's beautiful face (we miss you girl) 
back_to_main_loop 
                    bra      main                         ; go display text 

show_fury: 
                    jsr      Wait_Recal                   ; Vectrex BIOS recalibration 
                    jsr      Intensity_5F                 ; Sets the intensity of the 
                                                          ; vector beam to $5f 
                    lda      fury_img_XY                  ; Text position relative Y 
                    ldb      fury_img_XY + 1              ; Text position relative X 
                    jsr      Moveto_d 
                    lda      #-10                         ; set hight of image 
                    sta      Vec_Text_Height 
                    lda      #$40                         ; set width of image 
                    sta      Vec_Text_Width 
                    ldu      #fury1_data 
                    jsr      draw_raster_image            ; Vectrex BIOS print routine 
                    inc      img_persistence_counter      ; increment wait timer 
                                                          ; lda img_persistence_counter ; check if timer overflow 
                    bne      show_fury_loop               ; nope, keep image in same spot 
                    lda      #255                         ; yep, reset timer 
                    sta      img_persistence_counter 
                    lda      img_direction_fury           ;check which direction to move image 
                    bne      fury_up                      ; it's up, go do that 
                    dec      fury_img_XY                  ; otherwise, it's down, move the image down 
                    lda      fury_img_XY                  ; check if we are at bottom of screen 
                    cmpa     #-40 
                    bne      show_fury_loop               ; nope, keep going down 
                    inc      img_direction_fury           ; yep, we'll be going up from now on 
                    bra      show_fury_loop 

fury_up 
                    inc      fury_img_XY                  ; move image up 
                    lda      fury_img_XY                  ; check if hit top of screen 
                    cmpa     #$55 
                    bne      show_fury_loop               ; nope, keep going up 
                    dec      img_direction_fury           ; yep, we'll be going down from now on 
show_fury_loop: 
                    jsr      get_button 
                    cmpa     #4                           ; user hit button 4? 
                    bne      show_fury                    ; no, keep looping 
                    lda      #0                           ; yes, go back to main screen 
                    rts      

show_zanti: 
                    jsr      Wait_Recal                   ; Vectrex BIOS recalibration 
                    jsr      Intensity_5F                 ; Sets the intensity of the 
                                                          ; vector beam to $5f 
                    lda      zanti_img_XY                 ; Text position relative Y 
                    ldb      zanti_img_XY + 1             ; Text position relative X 
                    jsr      Moveto_d 
                    lda      #-10                         ; set hight of image 
                    sta      Vec_Text_Height 
                    lda      #$40                         ; set width of image 
                    sta      Vec_Text_Width 
                    ldu      #sectoid_data 
                    jsr      draw_raster_image            ; Vectrex BIOS print routine 
                    inc      img_persistence_counter      ; increment wait timer 
                    bne      show_zanti_loop              ; nope, keep image in same spot 
                    lda      #255                         ; yep, reset timer 
                    sta      img_persistence_counter 
                    lda      img_direction_zanti          ;check which direction to move image 
                    bne      zanti_up                     ; it's up, go do that 
                    dec      zanti_img_XY                 ; otherwise, it's down, move the image down 
                    lda      zanti_img_XY                 ; check if we are at bottom of screen 
                    cmpa     #-45 
                    bne      show_zanti_loop              ; nope, keep going down 
                    inc      img_direction_zanti          ; yep, we'll be going up from now on 
                    bra      show_zanti_loop 

zanti_up 
                    inc      zanti_img_XY                 ; move image up 
                    lda      zanti_img_XY                 ; check if hit top of screen 
                    cmpa     #$45 
                    bne      show_zanti_loop              ; nope, keep going up 
                    dec      img_direction_zanti          ; yep, we'll be going down from now on 
show_zanti_loop: 
                    jsr      get_button 
                    cmpa     #4                           ; user hit button 4? 
                    bne      show_zanti                   ; no, keep looping 
                    lda      #0                           ; yes, go back to main screen 
                    rts      

show_nikki: 
                    jsr      Wait_Recal                   ; Vectrex BIOS recalibration 
                    jsr      Intensity_5F                 ; Sets the intensity of the 
                                                          ; vector beam to $5f 
                    lda      nikki_img_XY                 ; Text position relative Y 
                    ldb      nikki_img_XY + 1             ; Text position relative X 
                    jsr      Moveto_d 
                    lda      #-10                         ; set hight of image 
                    sta      Vec_Text_Height 
                    lda      #$40                         ; set width of image 
                    sta      Vec_Text_Width 
                    ldu      #Niiki_face_1_data 
                    jsr      draw_raster_image            ; Vectrex BIOS print routine 
                    inc      img_persistence_counter      ; increment wait timer 
                    bne      show_nikki_loop              ; nope, keep image in same spot 
                    lda      #255                         ; yep, reset timer 
                    sta      img_persistence_counter 
                    lda      img_direction_nikki          ;check which direction to move image 
                    bne      nikki_up                     ; it's up, go do that 
                    dec      nikki_img_XY                 ; otherwise, it's down, move the image down 
                    lda      nikki_img_XY                 ; check if we are at bottom of screen 
                    cmpa     #-20 
                    bne      show_nikki_loop              ; nope, keep going down 
                    inc      img_direction_nikki          ; yes, we'll be going up from now on 
                    bra      show_nikki_loop 

nikki_up 
                    inc      nikki_img_XY                 ; move image up 
                    lda      nikki_img_XY                 ; check if hit top of screen 
                    cmpa     #$50 
                    bne      show_nikki_loop              ; nope, keep going up 
                    dec      img_direction_nikki          ; yep, we'll be going down from now on 
show_nikki_loop: 
                    jsr      get_button 
                    cmpa     #4                           ; user hit button 4? 
                    bne      show_nikki                   ; no, keep looping 
                    lda      #0                           ; yes, go back to main screen 
                    rts      

get_button: 
                    jsr      Read_Btns                    ; get button status 
                    cmpa     #$00                         ; is a button pressed? 
                    beq      get_button_end               ; no, return 
                    bita     #$01                         ; button 1 pressed? 
                    beq      get_button_2                 ; nope, check next button 
                    lda      #1 
                    bra      get_button_end 

get_button_2: 
                    bita     #$02                         ; button 2 pressed? 
                    beq      get_button_3                 ; nope, check next button 
                    lda      #2 
                    bra      get_button_end 

get_button_3: 
                    bita     #$04                         ; button 3 pressed? 
                    beq      get_button_4                 ; nope, check next button 
                    lda      #3 
                    bra      get_button_end 

get_button_4: 
                    bita     #$08                         ; button 4 pressed? 
                    beq      get_button_end               ; nope, we're done here 
                    ldb      button_4_state               ; check if button still being pressed from last screen 
                    bne      get_button_end               ; yep, we're done here 
                    lda      #4                           ; nope, it's a new button press 
                    sta      button_4_state               ; save it 
                    rts                                   ; go back to calling routine 

get_button_end: 
                    ldb      #0                           ; flag no button press 
                    stb      button_4_state 
                    rts      

; *********************************************************************
;  These variables are kept in what is normally Vectrex ROM space
;  SRAM allows for reads and writes to this space, all is RAM
;  These variables will keep their state after power has been removed
; **********************************************************************
message_Y: 
                    DB       0 
message_1: 
                    DB       "1: FURY LOGO", $80
message_2: 
                    DB       "2: ZANTI", $80
message_3: 
                    DB       "3: NIKKI", $80
message_4: 
                    DB       "4: RECALL LAST", $80
fury_img_XY: 
                    DB       $55, -$35                    ;Y,X location of Fury Logo ( 
zanti_img_XY: 
                    DB       $35, -$19                    ;Y,X location of Zanti Logo 
nikki_img_XY: 
                    DB       $50, -$20                    ;Y,X location of Nikki image 
img_persistence_counter: 
                    DB       0                            ; image movement speed (0 = slowest, 255 = fastes) 
img_direction_main: 
                    DB       0                            ; 0 = Down, 1 = Up 
img_direction_fury: 
                    DB       0                            ; 0 = Down, 1 = Up 
img_direction_zanti: 
                    DB       0                            ; 0 = Down, 1 = Up 
img_direction_nikki: 
                    DB       0                            ; 0 = Down, 1 = Up 
button_4_state: 
                    DB       0                            ; flag for state of button 1 at last check (0 = not pressed, 1 = was pressed) 
last_state: 
                    DB       0                            ; the last image shown, follows buttons pattern (1,2,3) 
;***************************************************************************
; DATA SECTION
;***************************************************************************
                    include  "rasterDraw.asm"
                    include  "fury1.asm"
                    include  "zanti.asm"
                    include  "Niiki_face_1.asm"
