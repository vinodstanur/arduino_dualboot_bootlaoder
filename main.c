/*
    Author: Vinod S
    http://blog.vinu.co.in
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
	
*/


#include <avr/io.h>
#define F_CPU 16000000
#include <util/delay.h>
#include "uart.h"
#include "i2c.h"
#include "boot.h"
#include <inttypes.h>
#include <avr/pgmspace.h>
#include "stk500.h"
#include "pin_defs.h"

/////////////////////// definitions /////////////////////////////////////////////////////
#define FUNC_READ 1
#define FUNC_WRITE 1
#define ER 0b10100001
#define EW 0b10100000
#define OPTIBOOT_MAJVER 6
#define OPTIBOOT_MINVER 2
#define LED_START_FLASHES 0
#define GETLENGTH(len) (void) getch() /* skip high byte */; len = getch()
#define save_vect_num (SPM_RDY_vect_num)
#define appstart_vec (save_vect_num*2)
#define WATCHDOG_1S     (_BV(WDP2) | _BV(WDP1) | _BV(WDE))
#define WATCHDOG_OFF    (0)
#define WATCHDOG_16MS   (_BV(WDE))

////////////////////// global variables ////////////////////////////////////////////////
volatile uint16_t dd = 0;
typedef uint8_t pagelen_t;
uint8_t  buff[200];

///////////////////// function prototypes //////////////////////////////////////////////
static inline void writebuffer(int8_t memtype, uint8_t *mybuff, uint16_t address, pagelen_t len);
static inline void read_mem(uint8_t memtype, uint16_t address, pagelen_t length);
void watchdogReset();
void verifySpace();
void watchdogConfig(uint8_t x);
void getNch(uint8_t count);

//////////////////// JUMP to main application function ////////////////////////////////
void app_start(void)
{

				watchdogConfig(WATCHDOG_OFF);
				__asm__ __volatile__ (
					"clr r30\n"
					"clr r31\n"
					"ijmp"
					);
}



///////////////// main function ////////////////////////////////////////////////////////
int main()
{

	uint8_t ch;
	uint16_t address = 0;

	pagelen_t length;

	ch = MCUSR;
	MCUSR = 0;
	
	watchdogConfig(WATCHDOG_OFF);

	if (ch & (_BV(WDRF) | _BV(BORF) | _BV(PORF)))
					app_start();
	
 


 PORTB &= ~(1<<PB5);
 DDRB |= 1<<PB5;

 //PIN 10 of arduino as ZERO
 PORTB &= ~(1<<PB2);
 //enable pin10 as output
 DDRB |= 1<<PB2;
 
 //PULL UP pin 11 of arduino
 PORTB |= 1<<PB3;
 //enable pin11 as input.
 DDRB &= ~(1<<PB3);

 _delay_ms(10);

 //check if 10 and 11 is shorted while MCU is reset.
 
 //LED ON
 PORTB |= 1<<PB5;

 if((PINB & (1<<PB3)) == 0) {
 
	uint8_t buff2[128];
	uint8_t buff1[128];
	_delay_ms(1000);
	PORTB &= ~(1<<PB5);
	if((PINB & (1<<PB3)) == 0) {
			app_start();
 }


	i2c_init();

	watchdogConfig(WATCHDOG_OFF);
	uint16_t addr = 0;
		for(int i = 0; i < 224; i++) {
			while(i2c_start(EW));
			i2c_write(addr >> 8);
			i2c_write(addr);
			i2c_start(ER);
			//read one page from i2c eeprom
			//read one page from internal flash
			for(int j = 0; j < 128; j++) {
				buff2[j] = i2c_read_ack();
				buff1[j] = pgm_read_byte_near(addr + j);
			}
			i2c_stop();
			i2c_start(EW);
			i2c_write(addr >> 8);
			i2c_write(addr);
			//write the page read from internal flash to i2c eeprom. (Swap write)
			for(int j = 0; j < 128; j++) {
				i2c_write(buff1[j]);
			}

			i2c_stop();

			//write the page read from external eeprom to internal flash.
			writebuffer(0, buff2, addr, 128);
	
			_delay_ms(2);
			
			//increment page address. Here both flash and eeprom are of same page size.
			addr += 128;
	
			//toggle LED
			PORTB ^= 1<<PB5;
		}
	
	
	
// wait while dualboot enable pin is LOW
 while((PINB & (1<<PB3)) == 0);

	//forcefull watchdog reset for app start.
	watchdogConfig(WATCHDOG_16MS);
  while(1);	
 }

 


 uart_init();	
 watchdogConfig(WATCHDOG_1S);
 watchdogReset();

  for (;;) {
    /* get character from UART */
    ch = getch();
	
    if(ch == STK_GET_PARAMETER) {
      unsigned char which = getch();
      verifySpace();
      /*
       * Send optiboot version as "SW version"
       * Note that the references to memory are optimized away.
       */
      if (which == STK_SW_MINOR) {
	 		putch((256*6 + 2) & 0xFF);
      } else if (which == STK_SW_MAJOR) {
	  putch((256*6 + 2) >> 8);
      } else {
	/*
	 * GET PARAMETER returns a generic 0x03 reply for
         * other parameters - enough to keep Avrdude happy
	 */
	putch(0x03);
      }
    }
    else if(ch == STK_SET_DEVICE) {
      // SET DEVICE is ignored
      getNch(20);
    }
    else if(ch == STK_SET_DEVICE_EXT) {
      // SET DEVICE EXT is ignored
      getNch(5);
    }
    else if(ch == STK_LOAD_ADDRESS) {
      // LOAD ADDRESS
      uint16_t newAddress;

      newAddress = getch();
      newAddress = (newAddress & 0xff) | (getch() << 8);

      newAddress += newAddress; // Convert from word address to byte address
      address = newAddress;
      verifySpace();
    }
    else if(ch == STK_UNIVERSAL) {
      // UNIVERSAL command is ignored
      getNch(4);
      putch(0x00);
    }
    /* Write memory, length is big endian and is in bytes */
    else if(ch == STK_PROG_PAGE) {
      // PROGRAM PAGE - we support flash programming only, not EEPROM
      uint8_t desttype;
      uint8_t *bufPtr;
      pagelen_t savelength;

      GETLENGTH(length);
      savelength = length;
      desttype = getch();
	    
      // read a page worth of contents
      bufPtr = buff;
      do *bufPtr++ = getch();
      while (--length);

      // Read command terminator, start reply
      verifySpace();

      writebuffer(desttype, buff, address, savelength);
		

		}
    /* Read memory block mode, length is big endian.  */
    else if(ch == STK_READ_PAGE) {
      uint8_t desttype;
      GETLENGTH(length);

      desttype = getch();

      verifySpace();

      read_mem(desttype, address, length);
    }

    /* Get device signature bytes  */
    else if(ch == STK_READ_SIGN) {
      // READ SIGN - return what Avrdude wants to hear
      verifySpace();
      putch(SIGNATURE_0);
      putch(SIGNATURE_1);
      putch(SIGNATURE_2);
    }
		
		
    else if (ch == STK_LEAVE_PROGMODE) { /* 'Q' */
      // Adaboot no-wait mod
      watchdogConfig(WATCHDOG_1S);
      verifySpace();
    }
    else {
      // This covers the response to commands like STK_ENTER_PROGMODE
      verifySpace();
    }
    putch(STK_OK);
  }


}




/*
 * void writebuffer(memtype, buffer, address, length)
 */
static inline void writebuffer(int8_t memtype, uint8_t *mybuff,
			       uint16_t address, pagelen_t len)
{
	    // Copy buffer into programming buffer
	    uint8_t *bufPtr = mybuff;
	    uint16_t addrPtr = (uint16_t)(void*)address;
	   watchdogReset();
	    /*
	     * Start the page erase and wait for it to finish.  There
	     * used to be code to do this while receiving the data over
	     * the serial link, but the performance improvement was slight,
	     * and we needed the space back.
	     */
	    __boot_page_erase_short((uint16_t)(void*)address);
	    boot_spm_busy_wait();

	    /*
	     * Copy data from the buffer into the flash write buffer.
	     */
	    do {
		uint16_t a;
		a = *bufPtr++;
		a |= (*bufPtr++) << 8;
		__boot_page_fill_short((uint16_t)(void*)addrPtr,a);
		addrPtr += 2;
	    } while (len -= 2);

	    /*
	     * Actually Write the buffer to flash (and wait for it to finish.)
	     */
	    __boot_page_write_short((uint16_t)(void*)address);
	    boot_spm_busy_wait();

__boot_rww_enable_alternate();

}

static inline void read_mem(uint8_t memtype, uint16_t address, pagelen_t length)
{
  uint8_t cc;
	//watchdogReset();
	  // read a Flash byte and increment the address

	do {
					__asm__ ("lpm %0,Z+\n" : "=r" (cc), "=z" (address): "1" (address));
					putch(cc);
	} while (--length);

}

void verifySpace() {
  if (getch() != CRC_EOP) {
    watchdogConfig(WATCHDOG_1S);    // shorten WD timeout
    while (1)			      // and busy-loop so that WD causes
      ;				      //  a reset and app start.
  }
  putch(STK_INSYNC);
}



void getNch(uint8_t count) {
  do getch(); while (--count);
  verifySpace();
}




void watchdogConfig(uint8_t x) {
				   WDTCSR = _BV(WDCE) | _BV(WDE);
				  WDTCSR = x;
				 }


// Watchdog functions. These are only safe with interrupts turned off.
void watchdogReset() {
				  __asm__ __volatile__ (
													    "wdr\n"
															  );
}
