/*
 * Copyright (C) 2000-2004 Damien Douxchamps  <ddouxchamps@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __ICONS_H

#ifdef HAVE_GDK_PIXBUF

static char * coriander_icon_xpm[] = {
"48 48 97 2",
"  	c None",
". 	c #FF3333",
"+ 	c #47FF47",
"@ 	c #FF5151",
"# 	c #FF0000",
"$ 	c #FFE974",
"% 	c #FFE351",
"& 	c #FFDD26",
"* 	c #FFE248",
"= 	c #FFE76B",
"- 	c #00FF00",
"; 	c #FF1414",
"> 	c #FFDE2E",
", 	c #FFE662",
"' 	c #3DFF3D",
") 	c #0AFF0A",
"! 	c #FF0A0A",
"~ 	c #FF6E2C",
"{ 	c #ADEB33",
"] 	c #FF6110",
"^ 	c #A3E918",
"/ 	c #7AFF7A",
"( 	c #FF0801",
"_ 	c #FFB91F",
": 	c #FFDF37",
"< 	c #FFEA7C",
"[ 	c #FFE140",
"} 	c #F4DE24",
"| 	c #1EFA04",
"1 	c #FF4F0D",
"2 	c #FFE55A",
"3 	c #8EEB15",
"4 	c #14FF14",
"5 	c #28FF28",
"6 	c #FF4747",
"7 	c #1EFF1E",
"8 	c #FF3D3D",
"9 	c #70FF70",
"0 	c #FF7F2F",
"a 	c #0AFD01",
"b 	c #70F010",
"c 	c #FF2828",
"d 	c #FFED8E",
"e 	c #5BFF5B",
"f 	c #5BF20D",
"g 	c #D6E21F",
"h 	c #FFCB22",
"i 	c #FF7B15",
"j 	c #FFD424",
"k 	c #3DF609",
"l 	c #FF5B5B",
"m 	c #FF1A04",
"n 	c #FFA71C",
"o 	c #E0E121",
"p 	c #33F807",
"q 	c #FFEB85",
"r 	c #EACB22",
"s 	c #473D0A",
"t 	c #332C07",
"u 	c #3D3509",
"v 	c #7A6A12",
"w 	c #D6B91F",
"x 	c #A38D18",
"y 	c #0A0801",
"z 	c #000000",
"A 	c #847213",
"B 	c #C1E51C",
"C 	c #AD9619",
"D 	c #141103",
"E 	c #C1A71C",
"F 	c #66580F",
"G 	c #E0C221",
"H 	c #51460C",
"I 	c #F4D424",
"J 	c #F4DD6B",
"K 	c #EAD262",
"L 	c #1E1AE4",
"M 	c #332CD3",
"N 	c #2823DC",
"O 	c #0000FF",
"P 	c #5151FF",
"Q 	c #3333FF",
"R 	c #0A0AFF",
"S 	c #6666FF",
"T 	c #AD9B8E",
"U 	c #7A6EB0",
"V 	c #6658A8",
"W 	c #1411ED",
"X 	c #E0C240",
"Y 	c #CCB051",
"Z 	c #99847C",
"` 	c #7A6A96",
" .	c #473DC2",
"..	c #2826ED",
"+.	c #5B5BFF",
"@.	c #4747FF",
"#.	c #1414FF",
"                                                                                                ",
"                                                                                                ",
"                                                                                                ",
"                        .                                             +                         ",
"                    @ # #             $ % & & & & & & * =             - -                       ",
"                  ; # # # ;     = > & & & & & & & & & & & & & ,     ' - - )                     ",
"                ! # # # # # ~ & & & & & & & & & & & & & & & & & & { - - - - -                   ",
"              # # # # # # # # ] & & & & & & & & & & & & & & & & ^ - - - - - - - - /             ",
"            ! # # # # # # # # ( _ & & : =             < [ & & } | - - - - - - - - -             ",
"          ; # # # # # # # # # # 1 2                         $ 3 - - - - - - - - - - -           ",
"        @ # # # # # # # # # # # #                             4 - - - - - - - - - - - 5         ",
"        # # # # # # # # # # # # # @                         / - - - - - - - - - - - - - /       ",
"      6 # # # # # # # # # # # # # #                         ) - - - - - - - - - - - - - 7       ",
"          ; # # # # # # # # # # # # 8                     9 - - - - - - - - - - - - ) /         ",
"            0 # # # # # # # # # # # #                     ) - - - - - - - - - - a b             ",
"            & ] ( # # # # # # # # # # c     : & > % d   e - - - - - - - - - - f g &             ",
"          = & & h 1 # # # # # # # # ( i j & & & & & & & ^ | - - - - - - - k g & & & [           ",
"          > & & & 2   l # # # # # m n & & & & & & & & & & o p - - - - - ' q & & & & &           ",
"          & & & &         8 # # ( n & & & r s t u v w & & & o | - - 5       & & & & & d         ",
"        $ & & & :             c i & & & x y z z z z z A & & & B 4           2 & & & & *         ",
"        % & & & =               j & & x z z z t y z z z v & & &               & & & : 2         ",
"        & & & &                 & & r y z y & & & C D z z E & & =                               ",
"        & & & &               : & & s z z & & & & & & & & & & & &                               ",
"        & & & &               & & & t z t & & & & & & & & & & & &                               ",
"        & & & &               > & & u z y & & & & & & & & & & & &                               ",
"        & & & &               % & & v z z C & & & & & & & & & & &                               ",
"        & & & &               d & & w z z D & & & w F F F G & & ,                               ",
"        * & & & <               & & & A z z t F u y z z H & & &               & & : , q         ",
"        = & & & [               $ & & & v z z z z z z H I & & *             , & & & & [         ",
"          & & & &                 [ & & & E D z y s C & & & >               > & & & & <         ",
"          & & & & $                 [ & & & & & & & & & & >                 & & & & &           ",
"          , & & & &                   2 & & & & & & & & *                 : & & & & :           ",
"            & & & & ,                     J & & & & K                     & & & & & q           ",
"            * & & & &                       L M N O P                     % & & & >             ",
"              & & & & &                   Q O O O O R                       d & & d             ",
"              & & & & & >                 O O O O O O                           =               ",
"              [ & & & & & >               O O O O O O S                                         ",
"                  & & & & & & & > q     R O O O O O O O       : & %                             ",
"                    & & & & & & & & & T O O O O O O O O U > & & & & d                           ",
"                      : & & & & & & & V O O O O O O O O M & & & & & &                           ",
"                        , & & & & & & W O O O O O O O O O X & & & & & ,                         ",
"                            : & & & Y O O O O O O O O O O Z & & & > d                           ",
"                                [ & ` O O O O O O O O O O  .& : q                               ",
"                                    ..O O O O O O O O O O O                                     ",
"                                    O O O O O O O O O O O O                                     ",
"                                    O O O O O O O O O O O O +.                                  ",
"                                  @.O O O O O O O O O O O O #.                                  ",
"                                      S Q R O O O O O Q +.                                      "};

#endif

#endif
