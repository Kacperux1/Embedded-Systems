/*****************************************************************************
  *   This example is controlling the LEDs using the joystick
  *
  *   Copyright(C) 2009, Embedded Artists AB
  *   All rights reserved.
  *
  ******************************************************************************/


 #include "mcu_regs.h"
 #include "type.h"
 #include "uart.h"
 #include "stdio.h"
 #include "timer32.h"
 #include "gpio.h"
 #include "i2c.h"
 #include "ssp.h"
 #include "adc.h"

 #include "joystick.h"
 #include "pca9532.h"
 #include "acc.h"
 #include "rotary.h"
 #include "led7seg.h"
 #include "oled.h"
 #include "rgb.h"

 static uint8_t barPos = 2;
 static uint8_t buf[20];


 static void moveBar(uint8_t steps, uint8_t dir)
 {
     uint16_t ledOn = 0;

     if (barPos == 0)
         ledOn = (1 << 0) | (3 << 14);
     else if (barPos == 1)
         ledOn = (3 << 0) | (1 << 15);
     else
         ledOn = 0x07 << (barPos-2);

     barPos += (dir*steps);
     barPos = (barPos % 16);

     pca9532_setLeds(ledOn, 0xffff);
 }

 #define P1_2_HIGH() (LPC_GPIO1->DATA |= (0x1<<2))
 #define P1_2_LOW()  (LPC_GPIO1->DATA &= ~(0x1<<2))


 static uint32_t notes[] = {
         2272, // A - 440 Hz
         2024, // B - 494 Hz
         3816, // C - 262 Hz
         3401, // D - 294 Hz
         3030, // E - 330 Hz
         2865, // F - 349 Hz
         2551, // G - 392 Hz
         1136, // a - 880 Hz
         1012, // b - 988 Hz
         1912, // c - 523 Hz
         1703, // d - 587 Hz
         1517, // e - 659 Hz
         1432, // f - 698 Hz
         1275, // g - 784 Hz
 };

 static unsigned int morseCodeTable[] = {
 		// 1*, 2-
 		12, //*-		a
 		2111, //-***	b
 		2121, //-*-*	c
 		211, //-**		d
 		1, //*			e
 		1121, //**-*	f
 		221, //--*		g
 		1111, //****	h
 		11, //**		i
 		1222, //*---	j
 		212, //-*-		k
 		1211, //*-**	l
 		22, //--		m
 		21, //-*		n
 		222, //---		o
 		1221, //*--*	p
 		121, //*-*		r
 		111, //***		s
 		2, //-			t
 		112, //**-		u
 		122, //*--		w
 		2112, //-**-	x
 		2122, //-*--	y
 		2211 //--**		z
 };

 char morseTranslationTable[] = {
 		"abcdefghijklmnoprstuwxyz"
 };

/*
 char morseTranslationTable[] = {
 		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'r', 's', 't', 'u', 'w', 'x', 'y', 'z'
 };*/

 static void intToString(int value, uint8_t* pBuf, uint32_t len, uint32_t base)
 {
     static const char* pAscii = "0123456789abcdefghijklmnopqrstuvwxyz";
     int pos = 0;
     int tmpValue = value;

     // the buffer must not be null and at least have a length of 2 to handle one
     // digit and null-terminator
     if (pBuf == NULL || len < 2)
     {
         return;
     }

     // a valid base cannot be less than 2 or larger than 36
     // a base value of 2 means binary representation. A value of 1 would mean only zeros
     // a base larger than 36 can only be used if a larger alphabet were used.
     if (base < 2 || base > 36)
     {
         return;
     }

     // negative value
     if (value < 0)
     {
         tmpValue = -tmpValue;
         value    = -value;
         pBuf[pos++] = '-';
     }

     // calculate the required length of the buffer
     do {
         pos++;
         tmpValue /= base;
     } while(tmpValue > 0);


     if (pos > len)
     {
         // the len parameter is invalid.
         return;
     }

     pBuf[pos] = '\0';

     do {
         pBuf[--pos] = pAscii[value % base];
         value /= base;
     } while(value > 0);

     return;

 }

 static void playNote(uint32_t note, uint32_t durationMs) {

     uint32_t t = 0;

     if (note > 0) {

         while (t < (durationMs*1000)) {
             P1_2_HIGH();
             delay32Us(0, note / 2);

             P1_2_LOW();
             delay32Us(0, note / 2);

             t += note;
         }

     }
     else {
         delay32Ms(0, durationMs);
     }
 }

 static uint32_t getNote(uint8_t ch) {
     if (ch >= 'A' && ch <= 'G')
         return notes[ch - 'A'];

     if (ch >= 'a' && ch <= 'g')
         return notes[ch - 'a' + 7];

     return 0;
 }

 static void playNoteForHold(int hold) {
 	if (hold > 3) {
 		playNote(notes[0], 200);
 	} else {
 		playNote(notes[2], 200);
 	}
 }

 static uint32_t getDuration(uint8_t ch)
 {
     if (ch < '0' || ch > '9')
         return 400;

     /* number of ms */

     return (ch - '0') * 200;
 }

 static void ledLineSet(int hold){
	 // 11111111 przesuniete o maks ledow - hold
	 if (hold * 3 < 8){
		 pca9532_setLeds(255 >> (8 - hold * 3), 0xffff);
	 } else {
		 pca9532_setLeds(255, 0xffff);
	 }

	 if (hold > 2) {
		 pca9532_setLeds(255 << 8, 0);
	 }
 }

 static void playSong(uint8_t *song) {
     uint32_t note = 0;
     uint32_t dur  = 0;
     uint32_t pause = 0;

     /*
      * A song is a collection of tones where each tone is
      * a note, duration and pause, e.g.
      *
      * "E2,F4,"
      */

     while(*song != '\0') {
         note = getNote(*song++);
         if (*song == '\0')
             break;
         dur  = getDuration(*song++);
         if (*song == '\0')
             break;
         pause = getPause(*song++);

         playNote(note, dur);
         delay32Ms(0, pause);
     }
 }

 static uint8_t * song = (uint8_t*)"C2.C2,D4,C4,F4,E8,";
         //(uint8_t*)"C2.C2,D4,C4,F4,E8,C2.C2,D4,C4,G4,F8,C2.C2,c4,A4,F4,E4,D4,A2.A2,H4,F4,G4,F8,";
         //"D4,B4,B4,A4,A4,G4,E4,D4.D2,E4,E4,A4,F4,D8.D4,d4,d4,c4,c4,B4,G4,E4.E2,F4,F4,A4,A4,G8,";

 //////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main (void) {
	 uint8_t joy = 0;
	 unsigned int code[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	 int number = 0;


     int32_t xoff = 0;
     int32_t yoff = 0;
     int32_t zoff = 0;

     int8_t x = 0;
     int8_t y = 0;
     int8_t z = 0;
     uint8_t dir = 1;

     uint8_t btn1 = 0;

     GPIOInit();
     // init do 3-color leda
     GPIOSetDir( PORT1, 9, 1 );
     GPIOSetDir( PORT1, 10, 1 );

     LPC_SYSCON->SYSAHBCLKCTRL |= (1<<10); // init timer 32 bit nr 1
     LPC_SYSCON->SYSAHBCLKCTRL |= (1<<9);

     I2CInit( (uint32_t)I2CMASTER, 0 );
     SSPInit();
     ADCInit( ADC_CLK );

     pca9532_init();
     joystick_init();
     acc_init();
     oled_init();
     rgb_init();


     /*
      * Assume base board in zero-g position when reading first value.
      */
     acc_read(&x, &y, &z);
     xoff = 0-x;
     yoff = 0-y;
     zoff = 64-z;


     /* ---- Speaker ------> */

     GPIOSetDir( PORT3, 0, 1 );
     GPIOSetDir( PORT3, 1, 1 );
     GPIOSetDir( PORT3, 2, 1 );
     GPIOSetDir( PORT1, 2, 1 );

     LPC_IOCON->JTAG_nTRST_PIO1_2 = (LPC_IOCON->JTAG_nTRST_PIO1_2 & ~0x7) | 0x01;

     GPIOSetValue( PORT3, 0, 0 );  //LM4811-clk
     GPIOSetValue( PORT3, 1, 0 );  //LM4811-up/dn
     GPIOSetValue( PORT3, 2, 0 );  //LM4811-shutdn

     /* <---- Speaker ------ */

     GPIOSetDir(PORT0, 1, 0);
     LPC_IOCON->PIO0_1 &= ~0x7;

     moveBar(1, dir);
     oled_clearScreen(OLED_COLOR_BLACK);

     int row = 1;
     int colCode = 1;
     int colChar = 1;
     int hold = 0;
     int print = 0;

//     LPC_TMR16B0->TCR = 1;
     int adc_value = (((ADCRead(0) - 38) * 10) / 49) + 50;

     while (1) {

     	//odczytywanie rotacji
     	acc_read(&x, &y, &z);
     	x = x+xoff;
     	y = y+yoff;
     	z = z+zoff;

     	if (x > 20 || x < -20 || y > 20 || y < -20)
     	{
     		oled_clearScreen(OLED_COLOR_BLACK);
     		row = 1;
     		colCode = 1;
     		colChar = 1;
     		for(int i=0; i<10; i++){
     			code[i]=0;
     		}
     		number = 0;
     	}


     	//odczytanie wartości przycisku
     	btn1 = GPIOGetValue(PORT0, 1);


         if (btn1 == 0) {
         	hold++;
         	if (hold > 3){
         		rgb_setLeds(5);
         	}
         	else{
         		rgb_setLeds(4);
         	}
             print = 1;
         }
         else {

         	rgb_setLeds(7);

         	if(print){

         		if(hold > 2) {
         		 	oled_putString(colCode, row,  (uint8_t*)"-", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
         		 	code[number] = code[number]*10+2;
         		 }
         		 else{
         		 	oled_putString(colCode, row,  (uint8_t*)"*", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
         		 	code[number] = code[number]*10+1;
         		 }

 				playNoteForHold(hold);

 				colCode = colCode + 7;
 				 if (colCode > 90){
 					 row = row + 15;
 					 colCode = 1;
 					 if(row > 50){
 						 row = 1;
 						 colCode = 1;
 						 oled_clearScreen(OLED_COLOR_BLACK);
 					 }
 				 }
 				hold = 0;
 				print = 0;
 			}
         }


  		if(code[number]>223) {
  			oled_putString(colCode, row,  (uint8_t*)"/", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
  			colCode = colCode + 7;
  			if(colCode > 90){
  				row = row+15;
  				colCode = 1;
  				if(row > 50){
  					row = 1;
  					colCode = 1;
  					oled_clearScreen(OLED_COLOR_BLACK);
  				}
  			}
  			number++;
  		}


         joy = joystick_read();

         if ((joy & JOYSTICK_DOWN) != 0) {
        	 colChar = 1;
        	 for(int i=0; i<=number; i++){
        		 for(int j=0; j<24; j++){	//24 to liczba elementów w tabelce morseCodeTable bo nie wiem czy jest tutaj fukcja length albo coś takiego
        			 if(code[i]==morseCodeTable[j]){
        				 oled_putChar(colChar, 40,  (uint8_t*)morseTranslationTable[j], OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        				 break;
        			 }
        		 }
        		 colChar = colChar + 7;

        	 }
         }

         if ((joy & JOYSTICK_RIGHT) != 0) {
        	 oled_putString(colCode, row,  (uint8_t*)"/", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        	 number ++;
        	 colCode = colCode+7;
         }


         // na podstawie potencjometru zmien predkosc delayu miedzy iteracjami
         adc_value = (((ADCRead(0) - 30) * 10) / 49) + 50;

         oled_putString(1, 50, "    ", OLED_COLOR_BLACK, OLED_COLOR_BLACK);
         intToString(adc_value, buf, 10, 10);
         oled_putString(1, 50, buf, OLED_COLOR_WHITE, OLED_COLOR_BLACK);

         ledLineSet(hold);

         delay32Ms(1, adc_value);

     }
 }
