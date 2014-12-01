//---------------------------------
// EE174 Lab 1 - Stoplight
// Group 11 - Fall 2014
// Members: Janice Pham, Andrew Vo
//---------------------------------

#include <msp430.h>

#define LED_GRN 	BIT4
#define LED_YEL 	BIT3
#define LED_RED 	BIT2
#define BIT_ALL		0xFFFF

void init_timer();
void init_ports();
void delay_ms(unsigned int time);

volatile unsigned int g_tick_ms = 0;

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer
	init_timer();					// Configure TimerA0
	init_ports();					// Configure ports
	__bis_SR_register(GIE);			// Enable interrupts

	while(1) {
		P1OUT |= LED_GRN;			// Turn on GRN
		delay_ms(55000);			// Wait 55 sec
		P1OUT &= ~LED_GRN;			// Turn off GRN
		P1OUT |= LED_YEL;			// Turn on YEL
		delay_ms(5000);				// Wait 5 sec
		P1OUT &= ~LED_YEL;			// Turn off YEL
		P1OUT |= LED_RED;			// Turn on RED
		delay_ms(10000);			// Wait 10 sec
		P1OUT &= ~LED_RED;			// Turn off RED
	}
}

void init_ports() {
	// initialize all ports to minimize wasted power
	P1DIR &= ~BIT_ALL;
	P2DIR &= ~BIT_ALL;
	P3DIR &= ~BIT_ALL;
	P4DIR &= ~BIT_ALL;

	// GRN,YEL,RED pins as output
	P1DIR |= LED_GRN + LED_YEL + LED_RED;

	P1OUT &= ~BIT_ALL;
	P2OUT &= ~BIT_ALL;
	P3OUT &= ~BIT_ALL;
	P4OUT &= ~BIT_ALL;
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER_A (void) {
	g_tick_ms = g_tick_ms + 1;		// ticks every 1 ms
}

void init_timer() {
	TA0CCTL0 = CCIE;					// Enable Timer0 interrupts
	TA0CTL |= TASSEL_2 + ID_3 + MC_1;	// SMCLK/8 (1MHz/8 = 125KHz), upmode
	TA0CCR0 = 125;						// OVF every 1 ms
}

void delay_ms(unsigned int time) {		// relies on TimerA0 int. ~msec
	volatile unsigned t = time;			// prevents optimization
	TA0R = 0;							// reset timer counter
	g_tick_ms = 0;						// reset counter
	while(g_tick_ms <= t) {;}			// watches g_tick_ms until desired time
}
