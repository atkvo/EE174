#include <msp430.h> 

/*
 * main.c
 * 74HC595N
 */

#define SHIFT_CLK 	BIT5
#define SHIFT_IN 	BIT4
#define SHIFT_PORT	P1OUT
#define MAX_BARS	8
#define ADC_SAMPLES	15

void barFormatter(unsigned int binary);
void shiftByteOut(unsigned int byteToShift);
void initPorts();
void initADC();
unsigned int findMaxADC();

volatile unsigned int g_adcSound[ADC_SAMPLES];
volatile unsigned int g_adcSoundPointer = 0;

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    initPorts();
    initADC();
    ADC12CTL0 |= ADC12SC;
    __bis_SR_register(LPM0_bits + GIE);
	while(1) {
		barFormatter(findMaxADC());
	    __bis_SR_register(LPM0_bits + GIE);
	}
}

#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR (void) {
	switch(__even_in_range(ADC12IV, 34)) {
	case 6:							//ADC12MEM0 IFG
		break;
	case 8:							// ADC12MEM1 ready
		if (g_adcSoundPointer < ADC_SAMPLES) {
			g_adcSound[g_adcSoundPointer] = ADC12MEM1;		// Store ADC12 value
			g_adcSoundPointer = g_adcSoundPointer + 1;
			ADC12CTL0 &= ~ADC12SC;							// Stop sampling
			ADC12CTL0 |= ADC12SC;
		}
		else
		{
			g_adcSoundPointer = 0;
			ADC12CTL0 &= ~ADC12SC;		// Stop sampling
			__bic_SR_register_on_exit(LPM0_bits); // Exit LPM0
		}
		break;
//	case 12:
//		g_adcInfrared = ADC12MEM3;
//		ADC12CTL0 &= ~ADC12SC;
	default: break; 				// ignore all other interrupts
	}
}

void initADC() {
	P6SEL |= BIT2 + BIT3;					// select PIN6.2 as ADC input and 6.3
//	P6SEL |= BIT2;
	ADC12CTL1 = ADC12SHP + ADC12CONSEQ_0 + ADC12CSTARTADD_1;	// Config ADC12CTL1
	ADC12CTL0 |= ADC12SHT0_15 + ADC12ON + ADC12REFON;			// Config ADC12CTL0
//	ADC12IE |= ADC12IE1 + ADC12IE3;						// Enable ADC12MEM1 and 3 interrupt
	ADC12IE |= ADC12IE1;
	ADC12MCTL1 |= ADC12INCH_2 + ADC12SREF_0; 	// MEM2, AN2 & Vref = VCC-VSS
//	ADC12MCTL3 |= ADC12INCH_3 + ADC12SREF_0;	// MEM3, AN3 & Vref = VCC-VSS
	ADC12CTL0 |= ADC12ENC;						// Enable ADC Conversions
}

void initPorts() {
	P1DIR |= SHIFT_CLK + SHIFT_IN;
	P1OUT &= ~(SHIFT_CLK + SHIFT_IN);
}

void barFormatter(unsigned int value) {
	// 1. gets binary number 0-4095 and finds percentage
	// 2. forms led bar representing percentage
	// e.g. 40% = 11100000 (8 bars)
	volatile float temp;
	if (value > 2048)
		temp = value - 2048;
	else
		temp = 2048 - value;

	volatile unsigned int bars = 0;
	volatile float percentage = 0;
	unsigned barByte = 0;
//	percentage = (temp/4095);
	percentage = (temp/1400);
	if (percentage > 0.90)
		bars = 8;
	else
		bars = percentage*MAX_BARS;
	int i = 0;
	for(i = 0; i < bars; i++) {
		barByte |= 0x01;
		barByte = barByte << 1;
	}
	barByte = barByte >> 1;		// shift back right 1 bit
	while(i < 8) {
		barByte = barByte << 1;
		i = i + 1;
	}



	shiftByteOut(barByte);

}
void shiftByteOut(unsigned int byteToShift) {
	unsigned int outBit;
	int i = 0;
	for(i = 0; i < 9; i++) {
		outBit = (0x01 & byteToShift);
		switch(outBit) {
		case 0:
			SHIFT_PORT &= ~SHIFT_IN;
			break;
		case 1:
			SHIFT_PORT |= SHIFT_IN;
			break;
		default:
			break;
		}

		SHIFT_PORT |= SHIFT_CLK;
//		__delay_cycles(5000);
		SHIFT_PORT &= ~SHIFT_CLK;
		byteToShift = byteToShift >> 1;
	}
	__delay_cycles(1000);
}

unsigned int findMaxADC() {
	int i = 0;
	unsigned int max = g_adcSound[0];

	for(i = 0; i < ADC_SAMPLES; i++) {
		if(max < g_adcSound[i])
			max = g_adcSound[i];
		else
			;
	}

	return max;
}
