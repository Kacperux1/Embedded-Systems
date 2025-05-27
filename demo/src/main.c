/*****************************************************************************
  *   Prosty tlumacz alfabetu Morse'a
  *
  *   Grupa A10:
  *   Lech Czochra [251497], Jedrzej Bartoszewski [251482] , Kacper Maziarz [251586]
  *
  ******************************************************************************/


#include "mcu_regs.h"
#include "type.h"
#include "stdio.h"
#include "timer32.h"
#include "gpio.h"
#include "i2c.h"
#include "ssp.h"
#include "adc.h"
#include "joystick.h"
#include "pca9532.h"
#include "acc.h"
#include "oled.h"
#include "rgb.h"


// Okresla jakie napiecie bedzie na pinie P1.2 (PWM filter) aby skorzystac z glosnika
#define P1_2_HIGH() (LPC_GPIO1->DATA |= (1U<<2))
#define P1_2_LOW()  (LPC_GPIO1->DATA &= ~(1U<<2))
#define JOYSTICK_DOWN 0x04u
#define JOYSTICK_RIGHT 0x10u

//Odwzorowanie ciagow sygnalow na poszczegolne litery
static uint32_t morseCodeTable[] = {
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

static const char morseTranslationTable[] = "abcdefghijklmnoprstuwxyz";

/**
* @brief Generuje sygnał dźwiękowy o zadanej częstotliwosci na pinie 1_2 dla brzęczyka.
*
* @param note       Czestotliwość dźwięku w Hz .
* @param durationMs Czas trwania dźwięku w milisekundach.
*
* @return Brak (zwracany typ void).
*
* @side effects:
*         Funkcja uzywa pinu współdzielonego z diodą RGB (P1.2).
 *
*/
static void playNote(uint32_t note, uint32_t durationMs) {

    uint32_t t = 0;

    if (note > (uint32_t) 0) {

        while (t < (durationMs * 1000u)) {
            P1_2_HIGH();
            delay32Us(0u, note / 2u);

            P1_2_LOW();
            delay32Us(0u, note / 2u);

            t += note;
        }

    }
    else {
        delay32Ms(0, durationMs);
    }
}

/**
 * @brief Odtwarza dźwięk w zależności od długości przytrzymania przycisku.
 *
 * @param hold Liczba oznaczająca długość przytrzymania (liczba iteracji,
 * przez ktore trzymany jest przycisk).
 *
 *@return Brak (zwracany typ void)
 * @note Długość dźwięku to zawsze 200 ms.
 * @side effects: brak
 */

static void playNoteForHold(uint32_t hold) {
    if (hold > 3u) {
        // nuta A, 440 Hz, 200 ms trwania
        playNote(2272, 200);
    } else {
        // nuta C, 262 Hz, 200 ms trwania
        playNote(3816, 200);
    }
}

/**
* @brief Ustawia diody LED w zależności od długości przytrzymania.
*
* @param hold Liczba oznaczająca długość przytrzymania.
*             - Każde 3 jednostki aktywują kolejną diodę (maksymalnie 8).
*             - Dla hold > 2, włącza dodatkowe diody w drugiej części LED-ów (bit 8 i dalej).
*
* @return Brak (zwracany typ void)
*
* @side effects: brak
*/
static void ledLineSet(uint32_t hold){
    // 11111111 przesuniete o maks ledow - hold
    if ((hold * 3u) < 8u){
        pca9532_setLeds(255U >> (sss8U - (hold * 3U)), 0xffff);
    } else {
        pca9532_setLeds(255, 0xffff);
    }

    if (hold > 2u) {
        pca9532_setLeds(255UL << 8, 0);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main (void) {
    uint8_t joy = 0;
    unsigned int code[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int number = 0;


    int8_t xoff = 0;
    int8_t yoff = 0;
    int8_t zoff = 0;

    int8_t x = 0;
    int8_t y = 0;
    int8_t z = 0;

    uint8_t btn1 = 0;

    GPIOInit();
    // init do 3-color leda
    GPIOSetDir( PORT1, 9, 1 );
    GPIOSetDir( PORT1, 10, 1 );

    LPC_SYSCON->SYSAHBCLKCTRL |= (1UL<<10); // init timer 32 bit nr 1
    LPC_SYSCON->SYSAHBCLKCTRL |= (1UL<<9);

    I2CInit( (uint32_t)I2CMASTER, 0 );
    // init do SSP (potrzebne w trybie SPI do OLED)
    SSPInit();
    // init do ADC, potencjometr
    ADCInit( ADC_CLK );

    // init do PCA9532, polaczony z 16 ledami
    pca9532_init();
    // init do joysticka (GPIO)
    joystick_init();
    // init do akcelerometru
    acc_init();
    // init do oleda, dziala tylko po inicjacji SSP w trybie SPI
    oled_init();
    // init do leda rgb (GPIO)
    rgb_init();


    /*
     * Assume base board in zero-g position when reading first value.
     */
    acc_read(&x, &y, &z);
    xoff = -x;
    yoff = -y;
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

    pca9532_setLeds(0, 0xffff);
    oled_clearScreen(OLED_COLOR_BLACK);

    int row = 1;
    int colCode = 1;
    int colChar = 1;
    uint32_t hold = 0;
    int print = 0;

    int adc_value = (((ADCRead(0) - 38) * 10) / 49) + 50;

    while (1) {

        //odczytywanie rotacji
        acc_read(&x, &y, &z);
        x = x+xoff;
        y = y+yoff;
        z = z+zoff;

        if (((x > 20) || (x < -20)) || ((y > 20) || (y < -20)))
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


        if (btn1 == 0u) {
            hold++;
            if (hold > 3u){
                rgb_setLeds(5);
            }
            else{
                rgb_setLeds(4);
            }
            print = 1;
        }
        else {

            rgb_setLeds(7);

            if(print > 0){

                if(hold > 2u) {
                    oled_putString(colCode, row,  (const uint8_t*)"-", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                    code[number] = (code[number] * 10u) + 2u;
                }
                else{
                    oled_putString(colCode, row,  (const uint8_t*)"*", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                    code[number] = (code[number] * 10u) + 1u;
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


        if(code[number]>223u) {
            oled_putString(colCode, row,  (const uint8_t*)"/", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
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

        if ((joy & JOYSTICK_DOWN) != 0u) {
            colChar = 1;
            for(int i=0; i<=number; i++){
                for(int j=0; j<24; j++){	//24 to liczba elementów w tabelce morseCodeTable bo nie wiem czy jest tutaj fukcja length albo coś takiego
                    if(code[i]==morseCodeTable[j]){
                        oled_putChar(colChar, 40, (const uint8_t)morseTranslationTable[j], OLED_COLOR_WHITE, OLED_COLOR_BLACK);
                        break;
                    }
                }
                colChar = colChar + 7;

            }
        }

        if ((joy & JOYSTICK_RIGHT) != 0u) {
            oled_putString(colCode, row,  (const uint8_t*)"/", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            number ++;
            colCode = colCode+7;
        }


        // na podstawie potencjometru zmien predkosc delayu miedzy iteracjami
        adc_value = (((ADCRead(0) - 30) * 10) / 49) + 50;

        ledLineSet(hold);

        delay32Ms(1, adc_value);

    }
}
