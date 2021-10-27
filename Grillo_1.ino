// based on Tim Barras codes
// 2021 La Giusta Distanza Festival
// idea by Francesco Ganassin
// for a swarm of insects 
// sonic presence in the village of Stramare - Segusino (TV) - Italy
// ------------------------------------------------------------------

#include <MozziGuts.h>
#include <Oscil.h> // oscillator
#include <tables/cos2048_int8.h>
#include <tables/sin2048_int8.h>
#include <Smooth.h>
#include <AutoMap.h>
#include <EventDelay.h>
#include <mozzi_rand.h>

const int MIN_CARRIER_FREQ = 10; // se gli insetti sono in un posto non molto luminoso meglio aumentare
const int MAX_CARRIER_FREQ = 70;

const int MIN_INTENSITY = 0; // idem
const int MAX_INTENSITY = 80;

const int MIN_MOD_SPEED = 40; // idem
const int MAX_MOD_SPEED = 6000;

AutoMap kMapCarrierFreq(0,1023,MIN_CARRIER_FREQ,MAX_CARRIER_FREQ);
AutoMap kMapIntensity(0,1023,MIN_INTENSITY,MAX_INTENSITY);
AutoMap kMapModSpeed(0,1023,MIN_MOD_SPEED,MAX_MOD_SPEED);

const int LDR0_PIN=0; // queste sono le letture dalle fotoresistenze
const int LDR1_PIN=1; 
const int LDR2_PIN=2; 

Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aCarrier(COS2048_DATA); // si possono usare anche oscillatori sinusoidali
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aModulator(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, CONTROL_RATE> kIntensityMod(COS2048_DATA);

EventDelay kGainChangeDelay;
char gain = 1;

int mod_ratio = 30; // brightness (harmonics) - più alto il numero, più acuto e penetrante il verso del grillo
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
    kGainChangeDelay.start(); // manda a zero il volume periodicamente
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

// qui appaiono un po' di parametri ambientali 
// e altri parametri random

  float mod_speed = (float)kMapModSpeed(LDR2_value)/(1200*(rand((byte) 20))); Serial.print("   mod_speed = ");
  Serial.print(mod_speed);
  kIntensityMod.setFreq(mod_speed);

  Serial.println(); 
  
  // anche qui si può inserire un fattore timidezza del grillo oltre al random
  
   kGainChangeDelay.set((rand((byte) 400))*200); 
}

AudioOutput_t updateAudio(){
  long modulation = aSmoothIntensity.next(fm_intensity) * aModulator.next()*gain;
  return MonoOutput::from8Bit(aCarrier.phMod(modulation));
}

void loop(){
  audioHook();
}
