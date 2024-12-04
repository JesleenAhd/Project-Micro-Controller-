#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

#define SEG_A PORTD0
#define SEG_B PORTD1
#define SEG_C PORTD2
#define SEG_D PORTD3
#define SEG_E PORTD4
#define SEG_F PORTD5
#define SEG_G PORTD6

#define INCREMENT_BUTTON (PINC & (1<<PC0))
#define DECREMENT_BUTTON (PINC & (1<<PC1))
#define TOGGLE_BUTTON (PINC & (1<<PC2))
#define MODE_BUTTON (PINC & (1<<PC3))
#define STOP_ALARM_BUTTON (PINC & (1<<PC4))

#define ALARM_LED_PORT PORTC
#define ALARM_LED_PIN PC5

// Define the display digit pins
#define DIGIT1 PB0
#define DIGIT2 PB1
#define DIGIT3 PB2

// Define the LED pins for temperature range indication
#define LED1 PB4
#define LED2 PB5
#define LED3 PD7
#define LED4 PB3

int temperature = 0;
int mode = 0;
int display_mode = 0; // 0 for Celsius, 1 for Fahrenheit
int alarm_triggered = 0;

void init() {
	// Set data direction registers
	DDRC &= ~(1<<PC0 | 1<<PC1 | 1<<PC2 | 1<<PC3 | 1<<PC4); // Set PC0-PC4 as input for buttons
	DDRC |= (1<<PC5); // Set PC5 as output for alarm LED
	DDRD = 0xFF; // Set PORTD as output for 7-segment display
	DDRB = 0xFF; // Set PB0-PB7 as output for display digit control and LEDs

	// Enable pull-up resistors for buttons
	PORTC |= (1<<PC0 | 1<<PC1 | 1<<PC2 | 1<<PC3 | 1<<PC4);

	// Set initial state
	PORTC &= ~(1<<PC5); // Turn off alarm LED


	// Enable interrupts
	sei();
}

void displayDigit(uint8_t digit, uint8_t value) {
	// Array for 7-segment display digits (common cathode)
	uint8_t digits[10] = {
		0b00111111, // 0
		0b00000110, // 1
		0b01011011, // 2
		0b01001111, // 3
		0b01100110, // 4
		0b01101101, // 5
		0b01111101, // 6
		0b00000111, // 7
		0b01111111, // 8
		0b01101111  // 9
	};

	PORTB = (1 << digit);
	PORTD = digits[value];
	_delay_ms(10);
	PORTB = 0x00;
}

void displayNumber(int num) {
	// Extract digits
	int d1 = num / 100;
	int d2 = (num / 10) % 10;
	int d3 = num % 10;

	// Display each digit
	displayDigit(DIGIT1, d1);
	displayDigit(DIGIT2, d2);
	displayDigit(DIGIT3, d3);
}

void toggle_display_mode() {
	display_mode = !display_mode;
}

void check_alarm() {
	if (temperature > 40) {
		PORTC |= (1<<PC5); // Turn on alarm LED
		alarm_triggered = 1;
		} else {
		PORTC &= ~(1<<PC5); // Turn off alarm LED
		alarm_triggered = 0;
	}
}

void stop_alarm() {
	PORTC = (1<<PC4); // Turn off alarm LED
	alarm_triggered = 0;
}

void increment_temperature() {
	if (temperature < 99) {
		temperature++;
		check_alarm();
	}
}

void decrement_temperature() {
	if (temperature > 0) {
		temperature--;
		check_alarm();
	}
}

void change_mode() {
	mode = !mode;
}

void update_leds() {
	// Turn off all LEDs
	PORTB &= ~((1 << LED1) | (1 << LED2) | (1 << LED4));
	PORTD &= ~(1 << LED3);

	// Update LEDs based on temperature range
	if (temperature < 15) {
		PORTB |= (1 << LED1);
		} else if (temperature  < 26) {
		PORTB |= (1 << LED2);
		} else if (temperature < 36) {
		PORTD |= (1 << LED3);
		} else if (temperature < 41) {
		PORTB |= (1 << LED4);
	}
}

int main(void) {
	init();

	while (1) {
		if (!INCREMENT_BUTTON) {
			_delay_ms(140); // Debounce
			if (!INCREMENT_BUTTON) {
				increment_temperature();
			}
		}
		if (!DECREMENT_BUTTON) {
			_delay_ms(130); // Debounce
			if (!DECREMENT_BUTTON) {
				decrement_temperature();
			}
		}
		if (!TOGGLE_BUTTON) {
			_delay_ms(100); // Debounce
			if (!TOGGLE_BUTTON) {
				toggle_display_mode();
			}
		}
		if (!STOP_ALARM_BUTTON) {
			_delay_ms(50); // Debounce
			if (!STOP_ALARM_BUTTON) {
				stop_alarm();
			}
		}
		if (!MODE_BUTTON) {
			_delay_ms(100); // Debounce
			if (!MODE_BUTTON) {
				change_mode();
			}
		}

		if (mode == 0) {
			int display_temp = display_mode == 0 ? temperature : (temperature * 9 / 5) + 32;
			displayNumber(display_temp); // Display temperature
			} else {
			// Implement LED range indication logic
			update_leds();
		}

		_delay_ms(1);
	}
}