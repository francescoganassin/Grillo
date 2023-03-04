// based on Tim Barras codes
// 2021 La Giusta Distanza Festival
// idea by Francesco Ganassin
// for a swarm of insects 
// sonic presence in the village of Stramare - Segusino (TV) - Italy
//
// ideal breakout is very compact, 
// powered by a LiPo battery, recharged by mini solar panel
// Leonardo Micro Pro and MozzyByte shield
// mini speaker 40mm 4 Ohm 3-5W
// ------------------------------------------------------------------

#include <MozziGuts.h>
#include <Oscil.h> // oscillator
#include <tables/cos2048_int8.h>
#include <tables/sin2048_int8.h>
#include <Smooth.h>
#include <AutoMap.h>
#include <EventDelay.h>
#include <mozzi_rand.h>

// if in low light environment consider 
// increasing min paramenters: 
// min_carrier_freq
// min_intensity
// min-mod_speed
//

const int MIN_CARRIER_FREQ = 10; // increase in low light
const int MAX_CARRIER_FREQ = 70;

const int MIN_INTENSITY = 0; // increase in low light
const int MAX_INTENSITY = 80;

const int MIN_MOD_SPEED = 40; // increase in low light
const int MAX_MOD_SPEED = 6000;

AutoMap kMapCarrierFreq(0,1023,MIN_CARRIER_FREQ,MAX_CARRIER_FREQ);
AutoMap kMapIntensity(0,1023,MIN_INTENSITY,MAX_INTENSITY);
AutoMap kMapModSpeed(0,1023,MIN_MOD_SPEED,MAX_MOD_SPEED);

const int LDR0_PIN=0; // queste sono le letture dalle fotoresistenze
const int LDR1_PIN=1; 
const int LDR2_PIN=2; 

// here also SIN oscillators could be used

Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aCarrier(COS2048_DATA); 
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aModulator(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, CONTROL_RATE> kIntensityMod(COS2048_DATA);

EventDelay kGainChangeDelay;
char gain = 1;

// brightness (harmonics) 
// the higher the number, the brighter the sound
int mod_ratio = 30; 
long fm_intensity; // carries control info from updateControl to updateAudio

float smoothness = 0.01f;
Smooth <long> aSmoothIntensity(smoothness);

void setup(){
  Serial.begin(115200); 
  startMozzi();
}

void updateControl(){
 if(kGainChangeDelay.ready()){
    gain = 1-gain; 
    kGainChangeDelay.start(); // sends periodically the volume to zero
 }

  int LDR0_value = mozziAnalogRead(LDR0_PIN); // value is 0-1023
  int carrier_freq = kMapCarrierFreq(LDR0_value);
  
  int mod_freq = carrier_freq * mod_ratio;
  aCarrier.setFreq(carrier_freq);
  aModulator.setFreq(mod_freq);

  int LDR1_value= mozziAnalogRead(LDR1_PIN); // value is 0-1023
  Serial.print("LDR1 = ");
  Serial.print(LDR1_value);
  Serial.print("\t"); 

  int LDR1_calibrated = kMapIntensity(LDR1_value);
  Serial.print("LDR1_calibrated = ");
  Serial.print(LDR1_calibrated);
  Serial.print("\t"); 

  fm_intensity = ((long)LDR1_calibrated * (kIntensityMod.next()+128))>>8; 
  Serial.print("fm_intensity = ");
  Serial.print(fm_intensity);
  Serial.print("\t"); 

  int LDR2_value= mozziAnalogRead(LDR2_PIN); // value is 0-1023
  Serial.print("LDR2 = ");
  Serial.print(LDR2_value);
  Serial.print("\t"); 

// here are both
// environmental paramenters 
// random parameters

  float mod_speed = (float)kMapModSpeed(LDR2_value)/(1200*(rand((byte) 20))); Serial.print("   mod_speed = ");
  Serial.print(mod_speed);
  kIntensityMod.setFreq(mod_speed);

  Serial.println(); 
  
  // here there is the possibility of changing the shyness of the insect
  // there is a randomic shyness, a straight parameter could be added
  
   kGainChangeDelay.set((rand((byte) 400))*200); 
}

AudioOutput_t updateAudio(){
  long modulation = aSmoothIntensity.next(fm_intensity) * aModulator.next()*gain;
  return MonoOutput::from8Bit(aCarrier.phMod(modulation));
}

void loop(){
  audioHook();
}
