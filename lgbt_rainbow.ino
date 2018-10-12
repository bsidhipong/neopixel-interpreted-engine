/*
 * PROGRAM: lgbt_rainbow
 * Engine to display neopixel animations via simple sequence of interpreterd
 * instructions.
 *
 * Included with this implementation is a sample sequence to display the LGBT rainbow
 * in recognition and celebration of the SCOTUS ruling on Obergefell v. Hodges.
 *
 * Copyright (C) 2015 Bill Sidhipong <bsidhipong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(16, 0);

uint8_t *pc, encoded_instruction; /* address of next instruction */
uint8_t counter; /* generic counter */

enum state {
	__INITIALIZE__,
	reset_pattern,
	decode,
	tick,
} interpreter_state = __INITIALIZE__;

#define OPCODE_MASK 0xC0 /* xx00:0000 */
#define OPCODE_SET_PIXEL_COLOR 0x00 /* 00[counter = xxxxxx], r, g, b */
#define OPCODE_SET_DELAY_SPEED 0x40 /* 01000000, delay */

const uint8_t lgbt_rainbow_pattern[] PROGMEM = {
	OPCODE_SET_DELAY_SPEED, 100,
	3, 0xf0, 0x00, 0x00, /* same as OPCODE_SET_PIXEL_COLOR+3, ... */
	3, 0xff, 0x80, 0x00,
	3, 0xff, 0xff, 0x00,
	3, 0x00, 0x79, 0x40,
	2, 0x40, 0x40, 0xff,
	2, 0xa0, 0x00, 0xc0,

	3, 0xf0, 0x00, 0x00,
	3, 0xff, 0x80, 0x00,
	3, 0xff, 0xff, 0x00,
	3, 0x00, 0x79, 0x40,
	2, 0x40, 0x40, 0xff,
	2, 0xa0, 0x00, 0xc0,

	OPCODE_SET_DELAY_SPEED, 65,
	3, 0xf0, 0x00, 0x00,
	3, 0xff, 0x80, 0x00,
	3, 0xff, 0xff, 0x00,
	3, 0x00, 0x79, 0x40,
	2, 0x40, 0x40, 0xff,
	2, 0xa0, 0x00, 0xc0,

	3, 0xf0, 0x00, 0x00,
	3, 0xff, 0x80, 0x00,
	3, 0xff, 0xff, 0x00,
	3, 0x00, 0x79, 0x40,
	2, 0x40, 0x40, 0xff,
	2, 0xa0, 0x00, 0xc0,

	OPCODE_SET_DELAY_SPEED, 45,
	3, 0xf0, 0x00, 0x00,
	3, 0xff, 0x80, 0x00,
	3, 0xff, 0xff, 0x00,
	3, 0x00, 0x79, 0x40,
	2, 0x40, 0x40, 0xff,
	2, 0xa0, 0x00, 0xc0,

	3, 0xf0, 0x00, 0x00,
	3, 0xff, 0x80, 0x00,
	3, 0xff, 0xff, 0x00,
	3, 0x00, 0x79, 0x40,
	2, 0x40, 0x40, 0xff,
	2, 0xa0, 0x00, 0xc0,

	OPCODE_SET_DELAY_SPEED, 32,
	3, 0xf0, 0x00, 0x00,
	3, 0xff, 0x80, 0x00,
	3, 0xff, 0xff, 0x00,
	3, 0x00, 0x79, 0x40,
	2, 0x40, 0x40, 0xff,
	2, 0xa0, 0x00, 0xc0,

	3, 0xf0, 0x00, 0x00,
	3, 0xff, 0x80, 0x00,
	3, 0xff, 0xff, 0x00,
	3, 0x00, 0x79, 0x40,
	2, 0x40, 0x40, 0xff,
	2, 0xa0, 0x00, 0xc0,

	3, 0xf0, 0x00, 0x00,
	3, 0xff, 0x80, 0x00,
	3, 0xff, 0xff, 0x00,
	3, 0x00, 0x79, 0x40,
	2, 0x40, 0x40, 0xff,
	2, 0xa0, 0x00, 0xc0,

	3, 0xf0, 0x00, 0x00,
	3, 0xff, 0x80, 0x00,
	3, 0xff, 0xff, 0x00,
	3, 0x00, 0x79, 0x40,
	2, 0x40, 0x40, 0xff,
	2, 0xa0, 0x00, 0xc0,

	3, 0xf0, 0x00, 0x00,
	3, 0xff, 0x80, 0x00,
	3, 0xff, 0xff, 0x00,
	3, 0x00, 0x79, 0x40,
	2, 0x40, 0x40, 0xff,
	2, 0xa0, 0x00, 0xc0,

	OPCODE_SET_DELAY_SPEED, 16,
	16, 0xf0, 0x00, 0x00,
	16, 0xff, 0x80, 0x00,
	16, 0xff, 0xff, 0x00,
	16, 0x00, 0x79, 0x40,
	16, 0x40, 0x40, 0xff,
	16, 0xa0, 0x00, 0xc0,

	0
};

void loop( void )
{
	static uint8_t r, g, b, delay_speed;
	switch( interpreter_state ) {
		case __INITIALIZE__:
			pixels.begin( );
			pixels.setBrightness( 32 );
			/* fallthrough */

		case reset_pattern:
			pc = (uint8_t *)lgbt_rainbow_pattern;
			/* fallthrough */

		case decode:
			interpreter_state = tick;
			encoded_instruction = pgm_read_byte( pc++ );
			if ( encoded_instruction == 0 ) interpreter_state = reset_pattern;
			else {
				switch ( encoded_instruction & OPCODE_MASK ) {
					case OPCODE_SET_PIXEL_COLOR:
						counter = encoded_instruction;
						/* operands */
						r = pgm_read_byte( pc++ );
						g = pgm_read_byte( pc++ );
						b = pgm_read_byte( pc++ );
						break;

					case OPCODE_SET_DELAY_SPEED:
						delay_speed = pgm_read_byte( pc++ );
						interpreter_state = decode;
						break;
				}
			}
			/* fallthrough */

		case tick:
			pixels.shift_right( );
			pixels.setPixelColor( 0, r, g, b );
			pixels.show( );
			delay( delay_speed );
			if ( --counter == 0 ) interpreter_state = decode;
			break;
	}
}

void setup( void ) { }
