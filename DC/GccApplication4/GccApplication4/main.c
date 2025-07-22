#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#define BAUD_RATE 9600
#define MY_UBRR ((F_CPU / (16UL * BAUD_RATE)) - 1)

void ADC_init(void) {
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
}

uint16_t ADC_read(uint8_t channel) {
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADCW;
}

void PWM_init(void) {
	DDRD |= (1 << PD5);
	TCCR1A = (1 << COM1A1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12);
	TCCR1B |= (1 << CS11) | (1 << CS10);
	ICR1 = 249;
	OCR1A = 0;
}

void UART_init(void) {
	UBRRH = (unsigned char)(MY_UBRR >> 8);
	UBRRL = (unsigned char)MY_UBRR;
	UCSRB = (1 << RXEN) | (1 << TXEN);
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

void UART_transmit(unsigned char data) {
	while (!(UCSRA & (1 << UDRE)));
	UDR = data;
}

void UART_send_string(const char* str) {
	while(*str) {
		UART_transmit(*str++);
	}
}

int main(void) {
	uint16_t adc_raw_value;
	uint16_t pwm_duty_value;
	uint8_t speed_percentage;
	char output_buffer[40];

	DDRD |= (1 << PD6) | (1 << PD7);
	PORTD |= (1 << PD6);
	PORTD &= ~(1 << PD7);

	ADC_init();
	PWM_init();
	UART_init();

	UART_send_string("DC Motor Speed Control Started.\r\n");
	UART_send_string("Potentiometer Controls Speed (0-100%).\r\n");
	UART_send_string("---------------------------------------\r\n");

	while(1) {
		adc_raw_value = ADC_read(0);
		pwm_duty_value = (uint16_t)(((long)adc_raw_value * ICR1) / 1023);
		OCR1A = pwm_duty_value;
		speed_percentage = (uint8_t)(((long)adc_raw_value * 100) / 1023);
		sprintf(output_buffer, "ADC: %04u | Speed: %03u%% | PWM: %04u/%04u\r\n",
		adc_raw_value, speed_percentage, pwm_duty_value, ICR1);
		UART_send_string(output_buffer);
		_delay_ms(200);
	}

	return 0;
}