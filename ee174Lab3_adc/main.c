#include <msp430.h> 

/*
 *  Lab 3 Task 2
 *  EE174
 *  Group 11
 *  Janice Pham, Andrew Vo
 */

volatile unsigned int g_adcValue = 0;

void init_adc();

int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	init_adc();

	while(1) {
		ADC12CTL0 |= ADC12SC;
		__bis_SR_register(LPM0_bits + GIE);
	}
}

#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR (void) {
	switch(__even_in_range(ADC12IV, 34)) {
	case 6:							//ADC12MEM0 IFG
		break;
	case 8:							// ADC12MEM1 ready
		g_adcValue = ADC12MEM1;		// Store ADC12 value
		ADC12CTL0 &= ~ADC12SC;		// Stop sampling
		__bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
		break;
	default: break; 				// ignore all other interrupts
	}
}

void init_adc() {
	P6SEL |= BIT1;					// select PIN6.0 as ADC input
	ADC12CTL1 = ADC12SHP + ADC12CONSEQ_0 + ADC12CSTARTADD_1;	// Config ADC12CTL1
	ADC12CTL0 |= ADC12SHT0_15 + ADC12ON + ADC12REFON;			// Config ADC12CTL0
	ADC12IE |= ADC12IE1;						// Enable ADC12MEM1 interrupt
	ADC12MCTL1 |= ADC12INCH_1 + ADC12SREF_0; 	// MEM1, AN1 & Vref = VCC-VSS
	ADC12CTL0 |= ADC12ENC;						// Enable ADC Conversions
}
