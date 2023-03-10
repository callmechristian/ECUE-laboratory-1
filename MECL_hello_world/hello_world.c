/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "altera_avalon_pio_regs.h"
#include "system.h"
#include "sys/alt_timestamp.h"

// global vars
// button capture
volatile int edge_capture;
// dict for hex numbers
static int LUT[10] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10};
// define int size
#define INT_SIZE sizeof(int) * 8 /* Total number of bits in integer */

// helper funcs
// program interrupts
static void handle_button_interrupts(void* context, alt_u32 id) {
	/* Cast context to edge_capture's type. It is important that this
	be declared volatile to avoid unwanted compiler optimization. */
	volatile int* edge_capture_ptr = (volatile int*) context;
	/* Read the edge capture register on the button PIO. Store value. */
	*edge_capture_ptr =
	IORD_ALTERA_AVALON_PIO_EDGE_CAP (KEY_BASE);
	/* Write to the edge capture register to reset it. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP (KEY_BASE, 0);
	/* Read the PIO to delay ISR exit. This is done to prevent a
	spurious interrupt in systems with high processor -> pio
	latency and fast interrupts. */
	IORD_ALTERA_AVALON_PIO_EDGE_CAP (KEY_BASE);
}
// registering the interrupt handler
static void init_button_pio() {
	/* Recast the edge_capture pointer to match the alt_irq_register() function
	prototype. */
	void* edge_capture_ptr = (void*) &edge_capture;
	/* Enable all 4 button interrupts. */
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK (KEY_BASE, 0xf);
	/* Reset the edge capture register. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP (KEY_BASE, 0x0);
	/* Register the ISR. */
	alt_irq_register(KEY_IRQ, edge_capture_ptr,handle_button_interrupts );
}
// display number on the seven segment display
void display_seven(int number) {
	int i = 0, numlen = (int)log10(number)+1;
	if(number == 0) {
		numlen = 1;
	}
	for (i = 0; i < 6; i++){
		if(numlen <= i) {
			IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE-0x20*i, 0xff);
		} else {
//			printf("%d\n", number);
			IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE-0x20*i, LUT[number%10]);
		}
		number = number/10;
	}
}

// display number with milliseconds
void display_seven_time(double number) {
	int i = 0;
	int seconds = floor(number);
	int remaining_ms = (number - seconds)*1000;

	int numlen1 = log10(seconds) + 1;
	int numlen2 = 3;

	if(seconds == 0) {
		numlen1 = 1;
	}

	for(i=0; i<6; i++) {
		IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE-0x20*i, 0xff);
	}

	// write ms to display
	for (i = 0; i < numlen2; i++){
			IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE-0x20*i, LUT[remaining_ms%10]);
			remaining_ms = remaining_ms/10;
	}

	// write s to display
	for (i = 4; i < 6 - (numlen1 == 1); i++){
			IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE-0x20*i, LUT[seconds%10]);
			seconds = seconds/10;
	}
}

// count number of 1s in the binary structure of the number
int count_ones(int nr) {
	int zeros, ones, i;

	zeros = 0;
	ones = 0;

	for(i=0; i<INT_SIZE; i++)
	{
		/* If LSB is set then increment ones otherwise zeros */
		if(nr & 1)
			ones++;
		else
			zeros++;

		/* Right shift bits of nr to one position */
		nr >>= 1;
	}

	return ones;
}

// read number of HIGH switches
int read_nr_switches() {
	int data;
	data = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
	printf("Our number %d\n", data);
	printf("Nr of ones %d\n", count_ones(data));
	return count_ones(data);
}



// exercises
void ex_c1() {
	// READ SWITCHES AND WRITE TO LEDS
	int data;
	data = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
	//  printf("%d",data);
	IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, data);
	// ---------------------------------

	// TEST 7 SEGMENT DISPLAY AND ENCODE NUMBERS
	//  IOWR_ALTERA_AVALON_PIO_DATA(HEX5_BASE, LUT[0]);
	//  IOWR_ALTERA_AVALON_PIO_DATA(HEX4_BASE, 0x79);
	//  IOWR_ALTERA_AVALON_PIO_DATA(HEX3_BASE, 0x24);
	//  IOWR_ALTERA_AVALON_PIO_DATA(HEX2_BASE, 0x30);
	//  IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE, 0x19);
	//  IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, 0x12);
	//  IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE, 0x02);
	//  IOWR_ALTERA_AVALON_PIO_DATA(HEX2_BASE, 0x78);
	//  IOWR_ALTERA_AVALON_PIO_DATA(HEX3_BASE, 0x00);
	//  IOWR_ALTERA_AVALON_PIO_DATA(HEX4_BASE, 0x10);

}

void ex_c2() {
	// C2 7 segment display counter value
	int i = 0;
	for (i = 0; i < 10000; i++) {
	  display_seven(i);
	  printf("Wrote %d to display\n", i);
	  usleep(20000);
	}
	// ---------------------------------
}

void ex_c3() {
	init_button_pio();
	int nr_switches = read_nr_switches();
	while(1) {
		if(edge_capture == 1) {
			display_seven(0);
		} else {
			display_seven(log2(edge_capture));
		}

		usleep(nr_switches*1000000);
	}
}

void ex_c4() {
	init_button_pio();
	double avg_stats_sum = 0;
	double avg_stats = 0;
	volatile int game_started = 0;
	int	counter = 1;
	while(1) {
		float my_time;

		if(edge_capture == 1) {
			// Returns a pseudo-random integer between 0 and RAND_MAX. // multiply by 1e6 to get the desired ms
			int r = 1 + (rand()%read_nr_switches());
			game_started = 1;

			printf("r is %d\n", r);
			// intialize the timer
			alt_timestamp_start();


			//start the game
//			game_started = true;
			while((alt_timestamp()/alt_timestamp_freq()) < r) {
				printf("%d\n", (alt_timestamp()/alt_timestamp_freq()));
				IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0b01010101010);
				usleep(1000000/2);
				IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0b10101010101);
				usleep(1000000/2);
				printf("%d\n", (alt_timestamp()/alt_timestamp_freq()));
			}

//			alt_timestamp(); // read the timer
//			alt_timestamp_freq() // read the frequence of the timer

			IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0xff);

			// at this point, timer ended and all LEDs are ON, waiting for user to press button 4
			// start a timer
			alt_timestamp_start();

			 edge_capture = 9; // ???
		}

		// if user presses 4, stop the timer and display
		if((log2(edge_capture) == 3 )&& (game_started == 1)) {
			printf("We pressed button 4");
			// save current time into a var
			printf("My time is: %f\n", (double)alt_timestamp()/(double)alt_timestamp_freq());
			// if we don't have stats, don't compute the mean

			if(counter > 1) {
				avg_stats_sum += (double)alt_timestamp()/(double)alt_timestamp_freq();
				avg_stats = avg_stats_sum / counter;
			} else {
				avg_stats_sum = (double)alt_timestamp()/(double)alt_timestamp_freq();
				avg_stats = (double)alt_timestamp()/(double)alt_timestamp_freq();
			}


			// display the current time
			IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, 0x00);
			display_seven_time((double)alt_timestamp()/(double)alt_timestamp_freq());
			game_started = 0;
			counter += 1;
		}

		// display the stats if user presses button 2 outside of the game
		if(log2(edge_capture) == 1) {
			display_seven_time(avg_stats);
		}
	}
}

int main()
{
  printf("Upload successful. :)\n");

  // for random numbers
  srand(time(NULL));   // Initialization, should only be called once.
  ex_c4();
  return 0;
}
