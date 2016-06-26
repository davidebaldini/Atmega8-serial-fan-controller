#define F_CPU 2000000UL
#include <stdlib.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <compat/deprecated.h>

#define RS232_INVERT   true  // expect negative logic coming into UART RX
#define grid_threshold 50    // low-pass threshold: 50:255 =~ 1V:5V (AVcc = 5V)

typedef unsigned char uchar;
typedef unsigned int  uint;

class trigger {
public:
  trigger();
  trigger(uint fan_speed);
  bool get(uint peak);

private:
  void set(uint peak, bool bit);
  uchar bitmap[0x100 / (sizeof(uchar) * 8)];
};

trigger::trigger() {
  // all zero
  for (uint i = 0; i <= 0xff; i++) set(i, 0);
}

trigger::trigger(uint fan_speed) {
  float const step = float(0xff) / fan_speed;
  float tour = step;
  bool intr = 0;  // interleave positive/negative half-waves

  // evenly-distributed triggers
  for (uint i = 0; i <= 0xff; i++) {
    if (i + 1 >= tour) {
      if (intr == (i % 2 == 0)) {
	set(i, 0);
	continue;
      }

      set(i, 1);
      tour += step;
      intr = (i % 2 == 0);
    }
    else set(i, 0);
  }
}

bool trigger::get(uint peak) {
  uint i, j;

  i = peak / (sizeof(uchar) * 8);
  j = peak % (sizeof(uchar) * 8);

  if (peak > 0xff) return false;
  return bool(bitmap[i] & 1<<j);
}

void trigger::set(uint peak, bool bit) {
  uint i, j;
  uchar newmap = 0x00, monomap;
  
  i = peak / (sizeof(uchar) * 8);
  j = peak % (sizeof(uchar) * 8);

  for (int k = sizeof(uchar) * 8 - 1; k >= 0; k--) {
    newmap <<= 1;
    
    if (k == j) {
      monomap = bit;
    } else {
      monomap = bitmap[i];
      monomap <<= (sizeof(uchar) * 8 - 1 - k);
      monomap >>= (sizeof(uchar) * 8 - 1);
    }
    newmap |= monomap;
  }
  
  bitmap[i] = newmap;
}

int main() {
  uint fan_speed = 0,
       grid0 = 0, grid1 = 0, grid2 = 0xff,
       peaks = 0;
  trigger trig;
  
  DDRB = 0x02;  //  PB1: optotriac gate
  DDRD = 0x02;  //  NOT port; PD2 –>○ PD1 (to negate rs232 signal)

  // configure interrupt for NOT port
  sei();             // enable IRQs (global flag)
  GICR |= 1<<INT0;   // use PD2
  MCUCR = 1<<ISC00;  // generate IRQs when port bits toggle
  
  // configure rs232
  UCSRA = 0x00;
  UCSRB = 1<<RXEN;                        // UART rx enabled
  UCSRC = 1<<URSEL | 1<<UCSZ1 | 1<<UCSZ0; // 8 data, 1 stop, 0 parity bits
  UBRRH = 0x00;
  UBRRL = 0x0c;     // 9600 baud @ 2MHz

  // configure ADC
  ADMUX = 1<<ADLAR | 1<<MUX0;  // use ADC channel 1, result in ADCH
  ADCSRA = 1<<ADEN | 1<<ADSC | 1<<ADFR | 1<<ADPS2 | 1<<ADPS0;  // ADC 60ĸHz @ 2MHz CPU, free-running
  
  while (true) {
    if (UCSRA & 1<<RXC) {
      fan_speed = UDR;
      trigger trig_new(fan_speed);
      trig = trig_new;
    }

    grid0 = trunc(round(ADCH / 4.0) * 4.0);  // round against noise

    // detect wave valley on rectified grid voltage
    if (grid0 < grid_threshold) {
      if (grid0 > grid1 && grid1 <= grid2) {
	peaks = (peaks < 0xff) ? peaks + 1 : 0;  // controlled overflow
	if (trig.get(peaks)) PORTB = 0x02;       // pin 15, PB1
      }
      
      grid2 = grid1;
      grid1 = grid0;
    } else {
      PORTB = 0X00;  // switch off triac's gate after wave rises
    }
  }
}

// IRQ NOT port
ISR(INT0_vect) {
  bool out = (PIND & 1<<PD2);

  if (RS232_INVERT) out = !out;
  if (out) PORTD = 0x02;
  else     PORTD = 0x00;
}
