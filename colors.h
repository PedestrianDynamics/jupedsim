/*
 * File to define colors for highlighting the output.
 */

#ifndef COLORS_H_
#define COLORS_H_


//Text attributes
#define OFF       0     //All attributes off
#define BRIGHT      1    //Bold on
//       4    Underscore (on monochrome display adapter only)
#define BLINK       5    //Blink on
//       7    Reverse video on
//      8    Concealed on

//   Foreground colors
//       30    Black
#define RED       31
#define GREEN       32
#define YELLOW       33
#define BLUE       34
#define MAGENTA     35
//       36    Cyan
//       37    White

//    Background colors
#define BG_BLACK       40
//       41    Red
#define  BG_GREEN  42
//      43    Yellow
//     44    Blue
//#define      45
//     46    Cyan
#define BG_WHITE     47

// Special caracters
#define  HOME  printf("\033[1;1H");  // cursor up left
#define CLEAR   printf(" \033[2J"); //clear screen
#define RED_LINE  printf("%c[%d;%d;%dm\n",0x1B, BRIGHT,RED,BG_BLACK);
#define GREEN_LINE  printf("\t%c[%d;%d;%dm",0x1B, BRIGHT,GREEN,BG_BLACK);
#define BLUE_LINE  printf("\t%c[%d;%d;%dm",0x1B, BRIGHT,BLUE,BG_BLACK);
#define MAGENTA_LINE  printf("\t%c[%d;%d;%dm",0x1B, BRIGHT,MAGENTA,BG_BLACK);
#define YELLOW_LINE  printf("\t%c[%d;%d;%dm",0x1B, BRIGHT,YELLOW,BG_BLACK);

#define OFF_LINE printf("%c[%dm\n", 0x1B, OFF);



#endif
