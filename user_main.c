// Configure ESP8266 to blink an LED

// Compile, link and then convert to a bin using the Makefile:  
//      make clean 
//      make
// Then flash to device using the Makefile.  Device must be in program mode
// which means you have to press and hold RST, press and hold PROGRAM, release
// RST and finally release PROGRAM.  PROGRAM is nothing more than GPIO0 connected
// to ground via a switch and resistor.
//      make flash

// IMPORTANT: Remember that the ESP8266 NON-OS firmware does not contain an
// operating system! No OS means no task scheduler. This means we have to use
// some other means of putting our code on the CPU for execution. In this
// example we'll use a software timer (which the documentation says isn't very
// accurate). 

// Includes:
// ets_sys.h: this includes a whole lot of types, structs, and other stuff
// osapi.h: this is where we get our memory, string, and timer functions 
// gpio.h: you guessed it ... GPIO functions such as gpio_init
// os_type.h: this includes our defines for signal, event, and timer types 

#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"

// Create software timer ... make it a static variable so it can't be called
// from any other source file. Remember that LOCAL is a macro for static.
LOCAL os_timer_t the_timer;

// Timer function ... read the status of GPIO2 ... if it is HIGH set it
// to LOW and vice versa. 
// NOTE: Make sure you call os_delay_us to give the SoC time to do other 
// things (like handle incoming WiFi connections). Also, do not write a
// timer funcation that will occupy the SoC for more than 15 ms ... if you
// code is on the SoC for more than 15ms the Watch Dog Timer (WDT) will think
// the SoC is "hung" and reset the system!
LOCAL void timer_function (void) {
  if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & BIT2)
    gpio_output_set(0, BIT2, BIT2, 0);
  else
    gpio_output_set(BIT2, 0, BIT2, 0); 
  os_delay_us(100);
}

// Entry function ... execution starts here.  Note the use of attribute
// ICACHE_FLASH_ATTR which directs the ESP to store the user code in flash
// instead of RAM.
// NOTE: Unlike a normal C main function, user_init does not execute sequentially.
// Instead, it is executed asynchronously. In other words, when user_init is run
// it returns immediately and the SoC goes on to do other things (like setup the 
// WiFi based on stored parameters). In fact, there is no guarantee that the SoC
// has completed it's setup tasks by the time you complete user_init!
void ICACHE_FLASH_ATTR user_init (void) {

  // Initialize the GPIO sub-system
  gpio_init();  

  // Set GPIO2 to be GPIO2 ... yeah it sounds stupid to do this but you
  // don't know how GPIO2 was previously configured ... it could have been
  // configured for something completely different
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2); 
  
  // Set GPIO2 as output and set it to LOW
  gpio_output_set(0, BIT2, BIT2, 0);

  // Disarm the timer ... it takes a pointer so pass the address of the_timer
  os_timer_disarm(&the_timer);

  // Setup the timer ... the timer will call timer_function when it runs 
  // Pass timer_function to the procedure by casting it as a pointer to a timer function
  // Our timer_function doesn't take any parameters so use NULL as the last argument
  os_timer_setfn(&the_timer, (os_timer_func_t *)timer_function, NULL);

  // Arm the timer ... 1000ms = 1 second and 1 means it repeats
  // Our timer will kick off every second and run the function bound to it in the above
  // statement (os_timer_setfn)
  os_timer_arm(&the_timer, 1000, 1);

}
