#include <avr/io.h>
#include <avr/eeprom.h>

#define SPEED_ADDRESS ((uint8_t*) 0)
#define SPEED_COMPLIMENT_ADDRESS ((uint8_t*) 1)
#define MAX_SPEED 3
#define DEBOUNCE_DELAY 4096

const uint8_t DISPLAY[] = {0x08, 0x05, 0x01, 0x0C};

int main()
{
	DDRB = 0x0F; // Set B0-B3 as outputs

	OCR0A = 47; // Set top value for counter (9.6 MHz / 8 / 25 kHz - 1)

	TCCR0A = 0x23; // Use fast PWM mode with OCR0A as maximum and positive polarity for OC0B
	TCCR0B = 0x09; // Enable timer without prescaler

	uint8_t speed = eeprom_read_byte(SPEED_ADDRESS);

	if ((uint8_t) ~speed != eeprom_read_byte(SPEED_COMPLIMENT_ADDRESS))
	{
		speed = MAX_SPEED; // Use maximum speed as a fallback if the EEPROM is corrupt
	}

	while (1)
	{
		PORTB = DISPLAY[speed];

		OCR0B = 11 + 12 * speed; // Set output compare to approximation of speed as a percentage of 47
		
		uint16_t delay = DEBOUNCE_DELAY;

		while (1)
		{
			if (PINB & 0x10)
			{
				if (delay)
				{
					delay--;
				}
			}
			else
			{
				if (!delay)
				{
					if (++speed > MAX_SPEED)
					{
						speed = 0;
					}

					eeprom_write_byte(SPEED_ADDRESS, speed);
					eeprom_write_byte(SPEED_COMPLIMENT_ADDRESS, ~speed);

					break;
				}

				delay = DEBOUNCE_DELAY;
			}
		}
	}
}