#define LOG_OUT 1 // use the log output function
#define FHT_N 256 // set to 256 point fht

#include <FHT.h>
#include <MD_MAX72xx.h>

int bins = FHT_N/2;

#define CS_PIN    10  //SS
#define NUM_DEV   12
MD_MAX72XX led = MD_MAX72XX(CS_PIN, NUM_DEV); //use SPI hardware

byte cols[] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF};

void setup() {
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode, div 32 prescaler
  ADMUX = 0x40; // use adc0 w/ 1.1V ref
  DIDR0 = 0x01; // turn off the digital input for adc0
  led.begin();
}

void loop() {
  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fht_input[i] = k; // put real data into bins
    }
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_log(); // magnitude response dB
    sei();
    //the display is written backwards, so bins-1-i
    for(int i=0; i<bins; i++) {
      led.setColumn(i,cols[fht_log_out[bins-1-i]/32]);
      //mx.setColumn(i,cols[fht_log_out[bins-33-i]/32]); //offset 32
    }
  }
}
