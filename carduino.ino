#include <EnableInterrupt.h>

#define SERIAL_PORT_SPEED 9600
#define RC_NUM_CHANNELS  6

#define RC_CH1  0
#define RC_CH2  1
#define RC_CH3  2
#define RC_CH4  3
#define RC_CH5  4
#define RC_CH6  5

#define RC_CH1_INPUT  A0
#define RC_CH2_INPUT  A1
#define RC_CH3_INPUT  A3
#define RC_CH4_INPUT  A6
#define RC_CH5_INPUT  A4
#define RC_CH6_INPUT  A5

#define ENG_A1 2
#define ENG_A2 3
#define ENG_AEN 10

#define ENG_B1 4
#define ENG_B2 5
#define ENG_BEN 11

uint16_t rc_values[RC_NUM_CHANNELS];
uint32_t rc_start[RC_NUM_CHANNELS];
volatile uint16_t rc_shared[RC_NUM_CHANNELS];

void rc_read_values() {
	noInterrupts();
	memcpy(rc_values, (const void *)rc_shared, sizeof(rc_shared));
	interrupts();
}

void calc_input(uint8_t channel, uint8_t input_pin) {
	if (digitalRead(input_pin) == HIGH) {
		rc_start[channel] = micros();
	} else {
		uint16_t rc_compare = (uint16_t)(micros() - rc_start[channel]);
		rc_shared[channel] = rc_compare;
	}
}

void calc_ch1() { calc_input(RC_CH1, RC_CH1_INPUT); }
void calc_ch2() { calc_input(RC_CH2, RC_CH2_INPUT); }
void calc_ch3() { calc_input(RC_CH3, RC_CH3_INPUT); }
void calc_ch4() { calc_input(RC_CH4, RC_CH4_INPUT); }
void calc_ch5() { calc_input(RC_CH5, RC_CH5_INPUT); }
void calc_ch6() { calc_input(RC_CH6, RC_CH6_INPUT); }

void setup() {
	Serial.begin(SERIAL_PORT_SPEED);
  pinMode(ENG_A1,OUTPUT);
  pinMode(ENG_A2,OUTPUT);
  pinMode(ENG_AEN,OUTPUT);

  pinMode(ENG_B1,OUTPUT);
  pinMode(ENG_B2,OUTPUT);
  pinMode(ENG_BEN,OUTPUT);

	pinMode(RC_CH1_INPUT, INPUT);
	pinMode(RC_CH2_INPUT, INPUT);
	pinMode(RC_CH3_INPUT, INPUT);
	pinMode(RC_CH4_INPUT, INPUT);
	pinMode(RC_CH5_INPUT, INPUT);
	pinMode(RC_CH6_INPUT, INPUT);

	enableInterrupt(RC_CH1_INPUT, calc_ch1, CHANGE);
	enableInterrupt(RC_CH2_INPUT, calc_ch2, CHANGE);
	enableInterrupt(RC_CH3_INPUT, calc_ch3, CHANGE);
	enableInterrupt(RC_CH4_INPUT, calc_ch4, CHANGE);
	enableInterrupt(RC_CH5_INPUT, calc_ch5, CHANGE);
	enableInterrupt(RC_CH6_INPUT, calc_ch6, CHANGE);
}

void loop() {
	rc_read_values();
	
	int speedValue = map(rc_values[RC_CH3],1000,2000,10,255);
	int frontBack = rc_values[RC_CH5]>1500 ? 1:-1; //"1"-> ileri "-1" -> geri
	uint16_t engineStatus = 0;

	//800-1000 (1)-> sadece on
  //1000-1600 (0)-> her ikisi
  //1600-2000 (2)-> sadece arka
	if(rc_values[RC_CH6] > 800  && rc_values[RC_CH6] < 1000){
		engineStatus = 1;
	}else if(rc_values[RC_CH6] > 1000 && rc_values[RC_CH6] < 1600){
		engineStatus = 0;
	}else	{
		engineStatus = 2;	
	}
 Serial.println(engineStatus );
 Serial.println(rc_values[RC_CH6]);

	if(frontBack == 1){
    //ileri
		digitalWrite(ENG_A1, HIGH);
		digitalWrite(ENG_A2,  LOW);  

		digitalWrite(ENG_B1, HIGH);
		digitalWrite(ENG_B2,  LOW);  
	}else{
      //geri
		digitalWrite(ENG_A1, LOW);
		digitalWrite(ENG_A2,  HIGH);

    digitalWrite(ENG_B1, LOW);
    digitalWrite(ENG_B2,  HIGH); 
	}

	if(engineStatus == 0){
		analogWrite(ENG_AEN,  speedValue); 
		analogWrite(ENG_BEN,  speedValue); 
	}else if(engineStatus == 1){
	analogWrite(ENG_AEN,  0); 
    analogWrite(ENG_BEN,  speedValue); 
	}else if(engineStatus == 2){
	  analogWrite(ENG_AEN,  speedValue); 
    analogWrite(ENG_BEN,  0); 
    
	}


	//servo kullanacağı için buraya değer gelmeyecek
	Serial.print("CH1:"); Serial.print(rc_values[RC_CH1]); Serial.print("\t"); 
	
	Serial.print("CH2:"); Serial.print(rc_values[RC_CH2]); Serial.print("\t");
	//Throttle Bar 0-255 arası ileri hangi hızla gidilecek.
	Serial.print("CH3:"); Serial.print(rc_values[RC_CH3]); Serial.print("\t");
	Serial.print("CH4:"); Serial.print(rc_values[RC_CH4]); Serial.print("\t");

	//İleri-geri seçimi
	Serial.print("CH5:"); Serial.print(rc_values[RC_CH5]); Serial.print("\t");

	//Hangi motorlar çekiş sağlayacak. 3 tık switch e bağlanılmalı.
	//800-1000-> sadece on
	//1000-1600-> her ikisi
	//1600-2000-> sadece arka
	Serial.print("CH6:"); Serial.print(rc_values[RC_CH6]);  Serial.println("\t");
	
	delay(100);
}
