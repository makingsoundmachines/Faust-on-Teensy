/* Receive Incoming USB MIDI using functions.  As usbMIDI
   reads incoming messages, handler functions are run.

   You must select "Tools > USB Type > Serial + MIDI"

*/

// AudioShield libraries and code from Faust
#include <Audio.h>
#include "faustAdditive.h"

AudioOutputI2S out;
AudioControlSGTL5000 audioShield;

// Create 8 instances of the Faust Harmonic Oscillator
// n and n+4 make up one voice, so 0+4, 1+5, 2+6, 3+7
// These pairs are slighty detuned to give a chorus impression
// and levelled differently on L and R outputs.

faustAdditive faustAdditiveSynth0;
faustAdditive faustAdditiveSynth1;
faustAdditive faustAdditiveSynth2;
faustAdditive faustAdditiveSynth3;

faustAdditive faustAdditiveSynth4;
faustAdditive faustAdditiveSynth5;
faustAdditive faustAdditiveSynth6;
faustAdditive faustAdditiveSynth7;


// Teensy Audio Library stuff - Routing the individual instances to the L+R channels.
// mixer0 and mixer2 are summed into mixer4 for the Left output
// mixer1 and mixer3 are summed into mixer5 for the Right output

AudioMixer4     mixer0; // L
AudioMixer4     mixer1; // R
AudioMixer4     mixer2; // L
AudioMixer4     mixer3; // R
AudioMixer4     mixer4; // L
AudioMixer4     mixer5; // R

AudioConnection patchCord0(faustAdditiveSynth0,0,mixer0,0);
AudioConnection patchCord1(faustAdditiveSynth1,0,mixer0,1);
AudioConnection patchCord2(faustAdditiveSynth2,0,mixer0,2);
AudioConnection patchCord3(faustAdditiveSynth3,0,mixer0,3);

AudioConnection patchCord4(faustAdditiveSynth0,0,mixer1,0);
AudioConnection patchCord5(faustAdditiveSynth1,0,mixer1,1);
AudioConnection patchCord6(faustAdditiveSynth2,0,mixer1,2);
AudioConnection patchCord7(faustAdditiveSynth3,0,mixer1,3);

AudioConnection patchCord8(faustAdditiveSynth4,0,mixer2,0);
AudioConnection patchCord9(faustAdditiveSynth5,0,mixer2,1);
AudioConnection patchCord10(faustAdditiveSynth6,0,mixer2,2);
AudioConnection patchCord11(faustAdditiveSynth7,0,mixer2,3);

AudioConnection patchCord12(faustAdditiveSynth4,0,mixer3,0);
AudioConnection patchCord13(faustAdditiveSynth5,0,mixer3,1);
AudioConnection patchCord14(faustAdditiveSynth6,0,mixer3,2);
AudioConnection patchCord15(faustAdditiveSynth7,0,mixer3,3);

AudioConnection patchCord16(mixer0,0,mixer4,0);
AudioConnection patchCord17(mixer2,0,mixer4,1);

AudioConnection patchCord18(mixer1,0,mixer5,0);
AudioConnection patchCord19(mixer3,0,mixer5,1);

AudioConnection patchCord20(mixer4,0,out,0);
AudioConnection patchCord21(mixer5,0,out,1);

// This array stores tempered frequencies for all 127 MIDI Note numbers.
// These values will be used as quantized pitch for the fundamental harmonic.
// Note that the harmonic partials are set in the Faust code and, are integer multipliers of the fundamental frequency.

//     C         C#        D         D#        E         F         F#        G         G#        A         A#        B

const float NoteNumToFreq[] = {
    8.18,     8.66,     9.18,     9.72,    10.30,    10.91,    11.56,    12.25,    12.98,    13.75,    14.57,    15.43,    
   16.35,    17.32,    18.35,    19.45,    20.60,    21.83,    23.12,    24.50,    25.96,    27.50,    29.14,    30.87,    
   32.70,    34.65,    36.71,    38.89,    41.20,    43.65,    46.25,    49.00,    51.91,    55.00,    58.27,    61.74,   
   65.41,    69.30,    73.42,    77.78,    82.41,    87.31,    92.50,    98.00,   103.82,   110.00,   116.54,   123.47, 
  130.81,   138.59,   146.83,   155.56,   164.81,   174.61,   184.99,   195.99,   207.65,   220.00,   233.08,   246.94,   
  261.63,   277.18,   293.66,   311.13,   329.63,   349.23,   369.99,   391.99,   415.31,   440.00,   466.16,   493.88,   
  523.25,   554.37,   587.33,   622.25,   659.26,   698.46,   739.99,   783.99,   830.61,   880.00,   932.32,   987.77,   
 1046.50,  1108.73,  1174.66,  1244.51,  1318.51,  1396.91,  1479.98,  1567.98,  1661.22,  1760.00,  1864.66,  1975.53,  
 2093.00,  2217.46,  2349.32,  2489.02,  2637.02,  2793.83,  2959.96,  3135.96,  3322.44,  3520.00,  3729.31,  3951.07,  
 4186.01,  4434.92,  4698.64,  4978.03,  5274.04,  5587.65,  5919.91,  6271.93,  6644.88,  7040.00,  7458.62,  7902.13,  
 8372.02,  8869.84,  9397.27,  9956.06, 10548.08, 11175.30, 11839.82, 12543.85 };

// MIDI polyphony - store the last note, and an array of all current notes. 
// Value -1 means the note is off (not sounding).
byte lastNote = 0;
int StoredNotes[4] = { -1, -1, -1, -1 };

void setup() { 

  // Slightly different levels are being set per Oscillator on L and R outputs.
  // This gives a more lively tonal result, similar to parts tolerance in analogue amplifiers
  
  mixer0.gain(0,0.25);
  mixer0.gain(1,0.20);
  mixer0.gain(2,0.22);
  mixer0.gain(3,0.26);

  mixer1.gain(0,0.15);
  mixer1.gain(1,0.17);
  mixer1.gain(2,0.12);
  mixer1.gain(3,0.18);
    
  mixer2.gain(0,0.15);
  mixer2.gain(1,0.12);
  mixer2.gain(2,0.14);
  mixer2.gain(3,0.17);
  
  mixer3.gain(0,0.26);
  mixer3.gain(1,0.22);
  mixer3.gain(2,0.28);
  mixer3.gain(3,0.20);
    
  mixer4.gain(0,1);
  mixer4.gain(1,1);
  mixer4.gain(2,1);
  mixer4.gain(3,1);
    
  mixer5.gain(0,1);
  mixer5.gain(1,1);
  mixer5.gain(2,1);
  mixer5.gain(3,1); 

  // Enable the AudioShield
  AudioMemory(40);
  audioShield.enable();
  audioShield.volume(0.7);

  // Handles for the USB MIDI callbacks
  usbMIDI.setHandleNoteOn(myNoteOn);
  usbMIDI.setHandleNoteOff(myNoteOff);
  usbMIDI.setHandleControlChange(myControlChange);

  // Init Serial at MIDI Baud Rate, should you want to add DIN Midi later
  Serial.begin(38400);
}


// Only looking for incoming MIDI events in the loop()
// myNoteOn(), myNoteOff() and myControlChange() will be processed on incoming MIDI messages.

void loop() {

  usbMIDI.read(); 
  
}

// Called from myControlChange() in case the MIDI CC # is 5.
// The integer CC value 0 .. 127 gets converted to float
// Setting the Attack time for harmonics A0 .. A7 of 8x faustAdditive instances.

void setAttack(int val) {

  // 1 / 127 * 222 = 1.749; as to not exceed 400, the max value set in Faust code, with a multiplier of *1.8
  
  float attackValue = float(val) * 1.749 ; 

  // setting slightly different values to mimics analogue parts tolerances
  // the fundamental gets assigned the longest Attack time
  
  faustAdditiveSynth0.setParamValue("A0",attackValue*1.8);
  faustAdditiveSynth0.setParamValue("A1",attackValue*1.6);
  faustAdditiveSynth0.setParamValue("A2",attackValue*1.5);
  faustAdditiveSynth0.setParamValue("A3",attackValue*1.4);
  faustAdditiveSynth0.setParamValue("A4",attackValue*1.3);
  faustAdditiveSynth0.setParamValue("A5",attackValue*1.2);
  faustAdditiveSynth0.setParamValue("A6",attackValue*1.1);
  faustAdditiveSynth0.setParamValue("A7",attackValue);

  faustAdditiveSynth1.setParamValue("A0",attackValue*1.8);
  faustAdditiveSynth1.setParamValue("A1",attackValue*1.7);
  faustAdditiveSynth1.setParamValue("A2",attackValue*1.6);
  faustAdditiveSynth1.setParamValue("A3",attackValue*1.5);
  faustAdditiveSynth1.setParamValue("A4",attackValue*1.4);
  faustAdditiveSynth1.setParamValue("A5",attackValue*1.3);
  faustAdditiveSynth1.setParamValue("A6",attackValue*1.2);
  faustAdditiveSynth1.setParamValue("A7",attackValue);  

  faustAdditiveSynth2.setParamValue("A0",attackValue*1.8);
  faustAdditiveSynth2.setParamValue("A1",attackValue*1.7);
  faustAdditiveSynth2.setParamValue("A2",attackValue*1.6);
  faustAdditiveSynth2.setParamValue("A3",attackValue*1.5);
  faustAdditiveSynth2.setParamValue("A4",attackValue*1.4);
  faustAdditiveSynth2.setParamValue("A5",attackValue*1.3);
  faustAdditiveSynth2.setParamValue("A6",attackValue*1.2);
  faustAdditiveSynth2.setParamValue("A7",attackValue); 

  faustAdditiveSynth3.setParamValue("A0",attackValue*1.8);
  faustAdditiveSynth3.setParamValue("A1",attackValue*1.7);
  faustAdditiveSynth3.setParamValue("A2",attackValue*1.6);
  faustAdditiveSynth3.setParamValue("A3",attackValue*1.5);
  faustAdditiveSynth3.setParamValue("A4",attackValue*1.4);
  faustAdditiveSynth3.setParamValue("A5",attackValue*1.3);
  faustAdditiveSynth3.setParamValue("A6",attackValue*1.2);
  faustAdditiveSynth3.setParamValue("A7",attackValue);  
  
  faustAdditiveSynth4.setParamValue("A0",attackValue*1.8);
  faustAdditiveSynth4.setParamValue("A1",attackValue*1.6);
  faustAdditiveSynth4.setParamValue("A2",attackValue*1.5);
  faustAdditiveSynth4.setParamValue("A3",attackValue*1.4);
  faustAdditiveSynth4.setParamValue("A4",attackValue*1.3);
  faustAdditiveSynth4.setParamValue("A5",attackValue*1.2);
  faustAdditiveSynth4.setParamValue("A6",attackValue*1.1);
  faustAdditiveSynth4.setParamValue("A7",attackValue);

  faustAdditiveSynth5.setParamValue("A0",attackValue*1.8);
  faustAdditiveSynth5.setParamValue("A1",attackValue*1.7);
  faustAdditiveSynth5.setParamValue("A2",attackValue*1.6);
  faustAdditiveSynth5.setParamValue("A3",attackValue*1.5);
  faustAdditiveSynth5.setParamValue("A4",attackValue*1.4);
  faustAdditiveSynth5.setParamValue("A5",attackValue*1.3);
  faustAdditiveSynth5.setParamValue("A6",attackValue*1.2);
  faustAdditiveSynth5.setParamValue("A7",attackValue);  

  faustAdditiveSynth6.setParamValue("A0",attackValue*1.8);
  faustAdditiveSynth6.setParamValue("A1",attackValue*1.7);
  faustAdditiveSynth6.setParamValue("A2",attackValue*1.6);
  faustAdditiveSynth6.setParamValue("A3",attackValue*1.5);
  faustAdditiveSynth6.setParamValue("A4",attackValue*1.4);
  faustAdditiveSynth6.setParamValue("A5",attackValue*1.3);
  faustAdditiveSynth6.setParamValue("A6",attackValue*1.2);
  faustAdditiveSynth6.setParamValue("A7",attackValue); 

  faustAdditiveSynth7.setParamValue("A0",attackValue*1.8);
  faustAdditiveSynth7.setParamValue("A1",attackValue*1.7);
  faustAdditiveSynth7.setParamValue("A2",attackValue*1.6);
  faustAdditiveSynth7.setParamValue("A3",attackValue*1.5);
  faustAdditiveSynth7.setParamValue("A4",attackValue*1.4);
  faustAdditiveSynth7.setParamValue("A5",attackValue*1.3);
  faustAdditiveSynth7.setParamValue("A6",attackValue*1.2);
  faustAdditiveSynth7.setParamValue("A7",attackValue);    
}

// Called from myControlChange() in case the MIDI CC # is 6.
// The integer CC value 0 .. 127 gets converted to float
// Setting the Decay time for harmonics D0 .. D7 of 8x faustAdditive instances.

void setDecay(int val) {

  float decayValue = float(val) * 1.749 ; // / 127 * 222;
  
  faustAdditiveSynth0.setParamValue("D0",decayValue);
  faustAdditiveSynth0.setParamValue("D1",decayValue*1.1);
  faustAdditiveSynth0.setParamValue("D2",decayValue*1.2);
  faustAdditiveSynth0.setParamValue("D3",decayValue*1.3);
  faustAdditiveSynth0.setParamValue("D4",decayValue*1.4);
  faustAdditiveSynth0.setParamValue("D5",decayValue*1.5);
  faustAdditiveSynth0.setParamValue("D6",decayValue*1.6);
  faustAdditiveSynth0.setParamValue("D7",decayValue*1.8);

  faustAdditiveSynth1.setParamValue("D0",decayValue);
  faustAdditiveSynth1.setParamValue("D1",decayValue*1.2);
  faustAdditiveSynth1.setParamValue("D2",decayValue*1.3);
  faustAdditiveSynth1.setParamValue("D3",decayValue*1.4);
  faustAdditiveSynth1.setParamValue("D4",decayValue*1.5);
  faustAdditiveSynth1.setParamValue("D5",decayValue*1.6);
  faustAdditiveSynth1.setParamValue("D6",decayValue*1.7);
  faustAdditiveSynth1.setParamValue("D7",decayValue*1.8); 

  faustAdditiveSynth2.setParamValue("D0",decayValue);
  faustAdditiveSynth2.setParamValue("D1",decayValue*1.2);
  faustAdditiveSynth2.setParamValue("D2",decayValue*1.3);
  faustAdditiveSynth2.setParamValue("D3",decayValue*1.4);
  faustAdditiveSynth2.setParamValue("D4",decayValue*1.5);
  faustAdditiveSynth2.setParamValue("D5",decayValue*1.6);
  faustAdditiveSynth2.setParamValue("D6",decayValue*1.7);
  faustAdditiveSynth2.setParamValue("D7",decayValue*1.8); 

  faustAdditiveSynth3.setParamValue("D0",decayValue);
  faustAdditiveSynth3.setParamValue("D1",decayValue*1.2);
  faustAdditiveSynth3.setParamValue("D2",decayValue*1.3);
  faustAdditiveSynth3.setParamValue("D3",decayValue*1.4);
  faustAdditiveSynth3.setParamValue("D4",decayValue*1.5);
  faustAdditiveSynth3.setParamValue("D5",decayValue*1.6);
  faustAdditiveSynth3.setParamValue("D6",decayValue*1.7);
  faustAdditiveSynth3.setParamValue("D7",decayValue*1.8); 
  
  faustAdditiveSynth4.setParamValue("D0",decayValue);
  faustAdditiveSynth4.setParamValue("D1",decayValue*1.1);
  faustAdditiveSynth4.setParamValue("D2",decayValue*1.2);
  faustAdditiveSynth4.setParamValue("D3",decayValue*1.3);
  faustAdditiveSynth4.setParamValue("D4",decayValue*1.4);
  faustAdditiveSynth4.setParamValue("D5",decayValue*1.5);
  faustAdditiveSynth4.setParamValue("D6",decayValue*1.6);
  faustAdditiveSynth4.setParamValue("D7",decayValue*1.8);

  faustAdditiveSynth5.setParamValue("D0",decayValue);
  faustAdditiveSynth5.setParamValue("D1",decayValue*1.2);
  faustAdditiveSynth5.setParamValue("D2",decayValue*1.3);
  faustAdditiveSynth5.setParamValue("D3",decayValue*1.4);
  faustAdditiveSynth5.setParamValue("D4",decayValue*1.5);
  faustAdditiveSynth5.setParamValue("D5",decayValue*1.6);
  faustAdditiveSynth5.setParamValue("D6",decayValue*1.7);
  faustAdditiveSynth5.setParamValue("D7",decayValue*1.8); 

  faustAdditiveSynth6.setParamValue("D0",decayValue);
  faustAdditiveSynth6.setParamValue("D1",decayValue*1.2);
  faustAdditiveSynth6.setParamValue("D2",decayValue*1.3);
  faustAdditiveSynth6.setParamValue("D3",decayValue*1.4);
  faustAdditiveSynth6.setParamValue("D4",decayValue*1.5);
  faustAdditiveSynth6.setParamValue("D5",decayValue*1.6);
  faustAdditiveSynth6.setParamValue("D6",decayValue*1.7);
  faustAdditiveSynth6.setParamValue("D7",decayValue*1.8); 

  faustAdditiveSynth7.setParamValue("D0",decayValue);
  faustAdditiveSynth7.setParamValue("D1",decayValue*1.2);
  faustAdditiveSynth7.setParamValue("D2",decayValue*1.3);
  faustAdditiveSynth7.setParamValue("D3",decayValue*1.4);
  faustAdditiveSynth7.setParamValue("D4",decayValue*1.5);
  faustAdditiveSynth7.setParamValue("D5",decayValue*1.6);
  faustAdditiveSynth7.setParamValue("D6",decayValue*1.7);
  faustAdditiveSynth7.setParamValue("D7",decayValue*1.8);
}

// Called from myControlChange() in case the MIDI CC # is 7.
// The integer CC value 0 .. 127 gets converted to float
// Setting the Sustain amount for harmonics S0 .. S7 of 8x faustAdditive instances.

void setSustain(int val) {

  // sustain doesn't like to be 0 so we offset it by 0.001
  // 127 * 0.007866 = 0.998; as to not exceed 1.0, with an offset of +0.001
  float sustainValue = (float(val) * 0.007866) + 0.001;
  
  faustAdditiveSynth0.setParamValue("S0",sustainValue);
  faustAdditiveSynth0.setParamValue("S1",sustainValue);
  faustAdditiveSynth0.setParamValue("S2",sustainValue);
  faustAdditiveSynth0.setParamValue("S3",sustainValue);
  faustAdditiveSynth0.setParamValue("S4",sustainValue);
  faustAdditiveSynth0.setParamValue("S5",sustainValue);
  faustAdditiveSynth0.setParamValue("S6",sustainValue);
  faustAdditiveSynth0.setParamValue("S7",sustainValue);
  
  faustAdditiveSynth1.setParamValue("S0",sustainValue);
  faustAdditiveSynth1.setParamValue("S1",sustainValue);
  faustAdditiveSynth1.setParamValue("S2",sustainValue);
  faustAdditiveSynth1.setParamValue("S3",sustainValue);
  faustAdditiveSynth1.setParamValue("S4",sustainValue);
  faustAdditiveSynth1.setParamValue("S5",sustainValue);
  faustAdditiveSynth1.setParamValue("S6",sustainValue);
  faustAdditiveSynth1.setParamValue("S7",sustainValue);

  faustAdditiveSynth2.setParamValue("S0",sustainValue);
  faustAdditiveSynth2.setParamValue("S1",sustainValue);
  faustAdditiveSynth2.setParamValue("S2",sustainValue);
  faustAdditiveSynth2.setParamValue("S3",sustainValue);
  faustAdditiveSynth2.setParamValue("S4",sustainValue);
  faustAdditiveSynth2.setParamValue("S5",sustainValue);
  faustAdditiveSynth2.setParamValue("S6",sustainValue);
  faustAdditiveSynth2.setParamValue("S7",sustainValue);
  
  faustAdditiveSynth3.setParamValue("S0",sustainValue);
  faustAdditiveSynth3.setParamValue("S1",sustainValue);
  faustAdditiveSynth3.setParamValue("S2",sustainValue);
  faustAdditiveSynth3.setParamValue("S3",sustainValue);
  faustAdditiveSynth3.setParamValue("S4",sustainValue);
  faustAdditiveSynth3.setParamValue("S5",sustainValue);
  faustAdditiveSynth3.setParamValue("S6",sustainValue);
  faustAdditiveSynth3.setParamValue("S7",sustainValue);
  
  faustAdditiveSynth4.setParamValue("S0",sustainValue);
  faustAdditiveSynth4.setParamValue("S1",sustainValue);
  faustAdditiveSynth4.setParamValue("S2",sustainValue);
  faustAdditiveSynth4.setParamValue("S3",sustainValue);
  faustAdditiveSynth4.setParamValue("S4",sustainValue);
  faustAdditiveSynth4.setParamValue("S5",sustainValue);
  faustAdditiveSynth4.setParamValue("S6",sustainValue);
  faustAdditiveSynth4.setParamValue("S7",sustainValue);
  
  faustAdditiveSynth5.setParamValue("S0",sustainValue);
  faustAdditiveSynth5.setParamValue("S1",sustainValue);
  faustAdditiveSynth5.setParamValue("S2",sustainValue);
  faustAdditiveSynth5.setParamValue("S3",sustainValue);
  faustAdditiveSynth5.setParamValue("S4",sustainValue);
  faustAdditiveSynth5.setParamValue("S5",sustainValue);
  faustAdditiveSynth5.setParamValue("S6",sustainValue);
  faustAdditiveSynth5.setParamValue("S7",sustainValue);

  faustAdditiveSynth6.setParamValue("S0",sustainValue);
  faustAdditiveSynth6.setParamValue("S1",sustainValue);
  faustAdditiveSynth6.setParamValue("S2",sustainValue);
  faustAdditiveSynth6.setParamValue("S3",sustainValue);
  faustAdditiveSynth6.setParamValue("S4",sustainValue);
  faustAdditiveSynth6.setParamValue("S5",sustainValue);
  faustAdditiveSynth6.setParamValue("S6",sustainValue);
  faustAdditiveSynth6.setParamValue("S7",sustainValue);
  
  faustAdditiveSynth7.setParamValue("S0",sustainValue);
  faustAdditiveSynth7.setParamValue("S1",sustainValue);
  faustAdditiveSynth7.setParamValue("S2",sustainValue);
  faustAdditiveSynth7.setParamValue("S3",sustainValue);
  faustAdditiveSynth7.setParamValue("S4",sustainValue);
  faustAdditiveSynth7.setParamValue("S5",sustainValue);
  faustAdditiveSynth7.setParamValue("S6",sustainValue);
  faustAdditiveSynth7.setParamValue("S7",sustainValue);
}

// Called from myControlChange() in case the MIDI CC # is 8.
// The integer CC value 0 .. 127 gets converted to float
// Setting the Release time for harmonics R0 .. R7 of 8x faustAdditive instances.

void setRelease(int val) {

  float releaseValue = float(val) * 1.749 ; // / 127 * 222;
  
  faustAdditiveSynth0.setParamValue("R0",releaseValue);
  faustAdditiveSynth0.setParamValue("R1",releaseValue*1.1);
  faustAdditiveSynth0.setParamValue("R2",releaseValue*1.2);
  faustAdditiveSynth0.setParamValue("R3",releaseValue*1.3);
  faustAdditiveSynth0.setParamValue("R4",releaseValue*1.4);
  faustAdditiveSynth0.setParamValue("R5",releaseValue*1.5);
  faustAdditiveSynth0.setParamValue("R6",releaseValue*1.6);
  faustAdditiveSynth0.setParamValue("R7",releaseValue*1.8);
  
  faustAdditiveSynth1.setParamValue("R0",releaseValue);
  faustAdditiveSynth1.setParamValue("R1",releaseValue*1.2);
  faustAdditiveSynth1.setParamValue("R2",releaseValue*1.3);
  faustAdditiveSynth1.setParamValue("R3",releaseValue*1.4);
  faustAdditiveSynth1.setParamValue("R4",releaseValue*1.5);
  faustAdditiveSynth1.setParamValue("R5",releaseValue*1.6);
  faustAdditiveSynth1.setParamValue("R6",releaseValue*1.7);
  faustAdditiveSynth1.setParamValue("R7",releaseValue*1.8);
    
  faustAdditiveSynth2.setParamValue("R0",releaseValue);
  faustAdditiveSynth2.setParamValue("R1",releaseValue*1.2);
  faustAdditiveSynth2.setParamValue("R2",releaseValue*1.3);
  faustAdditiveSynth2.setParamValue("R3",releaseValue*1.4);
  faustAdditiveSynth2.setParamValue("R4",releaseValue*1.5);
  faustAdditiveSynth2.setParamValue("R5",releaseValue*1.6);
  faustAdditiveSynth2.setParamValue("R6",releaseValue*1.7);
  faustAdditiveSynth2.setParamValue("R7",releaseValue*1.8);
    
  faustAdditiveSynth3.setParamValue("R0",releaseValue);
  faustAdditiveSynth3.setParamValue("R1",releaseValue*1.2);
  faustAdditiveSynth3.setParamValue("R2",releaseValue*1.3);
  faustAdditiveSynth3.setParamValue("R3",releaseValue*1.4);
  faustAdditiveSynth3.setParamValue("R4",releaseValue*1.5);
  faustAdditiveSynth3.setParamValue("R5",releaseValue*1.6);
  faustAdditiveSynth3.setParamValue("R6",releaseValue*1.7);
  faustAdditiveSynth3.setParamValue("R7",releaseValue*1.8);
  
  faustAdditiveSynth4.setParamValue("R0",releaseValue);
  faustAdditiveSynth4.setParamValue("R1",releaseValue*1.1);
  faustAdditiveSynth4.setParamValue("R2",releaseValue*1.2);
  faustAdditiveSynth4.setParamValue("R3",releaseValue*1.3);
  faustAdditiveSynth4.setParamValue("R4",releaseValue*1.4);
  faustAdditiveSynth4.setParamValue("R5",releaseValue*1.5);
  faustAdditiveSynth4.setParamValue("R6",releaseValue*1.6);
  faustAdditiveSynth4.setParamValue("R7",releaseValue*1.8);
  
  faustAdditiveSynth5.setParamValue("R0",releaseValue);
  faustAdditiveSynth5.setParamValue("R1",releaseValue*1.2);
  faustAdditiveSynth5.setParamValue("R2",releaseValue*1.3);
  faustAdditiveSynth5.setParamValue("R3",releaseValue*1.4);
  faustAdditiveSynth5.setParamValue("R4",releaseValue*1.5);
  faustAdditiveSynth5.setParamValue("R5",releaseValue*1.6);
  faustAdditiveSynth5.setParamValue("R6",releaseValue*1.7);
  faustAdditiveSynth5.setParamValue("R7",releaseValue*1.8);
    
  faustAdditiveSynth6.setParamValue("R0",releaseValue);
  faustAdditiveSynth6.setParamValue("R1",releaseValue*1.2);
  faustAdditiveSynth6.setParamValue("R2",releaseValue*1.3);
  faustAdditiveSynth6.setParamValue("R3",releaseValue*1.4);
  faustAdditiveSynth6.setParamValue("R4",releaseValue*1.5);
  faustAdditiveSynth6.setParamValue("R5",releaseValue*1.6);
  faustAdditiveSynth6.setParamValue("R6",releaseValue*1.7);
  faustAdditiveSynth6.setParamValue("R7",releaseValue*1.8);
    
  faustAdditiveSynth7.setParamValue("R0",releaseValue);
  faustAdditiveSynth7.setParamValue("R1",releaseValue*1.2);
  faustAdditiveSynth7.setParamValue("R2",releaseValue*1.3);
  faustAdditiveSynth7.setParamValue("R3",releaseValue*1.4);
  faustAdditiveSynth7.setParamValue("R4",releaseValue*1.5);
  faustAdditiveSynth7.setParamValue("R5",releaseValue*1.6);
  faustAdditiveSynth7.setParamValue("R6",releaseValue*1.7);
  faustAdditiveSynth7.setParamValue("R7",releaseValue*1.8);
}

// Macro code for assigning the volume of the harmonic partials per oscillator;
// a way to dynamically assign a lot of parameters at once, with just one knob.
//
// The idea is to map a different value curve for each parameter to the actual 
// input range of the knob (here MIDI CC2 - values 0 -127).
//
// The simplest example is one partial’s volume starting full volume at value 0 and 
// dropping to completely silent at value 127, while another partial does the opposite: 
// start completely silent at value 0 and rise to full volume at value 127.

void setVolMacro(int val) {

    float val1 = float(map(val, 0, 127, 1000, 0))*0.001;
    float val2 = float(map(val, 0, 127, 0, 1000))*0.001;
    float val3;
      if (val < 64) { val3 = float(map(val, 0, 63, 0, 1000))*0.001; } else { val3 = float(map(val, 64, 127, 1000, 0))*0.001; }
    float val4;
      if (val < 43) { val4 = float(map(val, 0, 42, 0, 1000))*0.001; } else { val4 = float(map(val, 43, 127, 1000, 0))*0.001; }
    float val5;
      if (val < 86) { val5 = float(map(val, 0, 85, 0, 1000))*0.001; } else { val5 = float(map(val, 86, 127, 1000, 0))*0.001; }   
    float val6;
      if (val < 32)                    { val6 = float(map(val,   0,  31,    0, 1000))*0.001; } 
      else if (val >= 32 && val < 64)  { val6 = float(map(val,  32,  63, 1000,  500))*0.001; }
      else if (val >= 64 && val < 97)  { val6 = float(map(val,  64,  96,  500, 1000))*0.001; }
      else if (val >= 97)              { val6 = float(map(val,  97, 127, 1000,    0))*0.001; }  
    float val7;
      if (val < 17)                    { val7 = float(map(val,   0,  16,    0,  500))*0.001; } 
      else if (val >= 17 && val < 64)  { val7 = float(map(val,  17,  63,  500,    0))*0.001; }
      else if (val >= 64 && val < 111) { val7 = float(map(val,  64, 110,    0, 1000))*0.001; }
      else if (val >= 111)             { val7 = float(map(val, 111, 127, 1000,    0))*0.001; }             
    
    faustAdditiveSynth0.setParamValue("vol1",val1);
    faustAdditiveSynth0.setParamValue("vol2",val2);
    faustAdditiveSynth0.setParamValue("vol3",val3);
    faustAdditiveSynth0.setParamValue("vol4",val4);
    faustAdditiveSynth0.setParamValue("vol5",val5);
    faustAdditiveSynth0.setParamValue("vol6",val6);
    faustAdditiveSynth0.setParamValue("vol7",val7);

    faustAdditiveSynth1.setParamValue("vol1",val7);
    faustAdditiveSynth1.setParamValue("vol2",val6);
    faustAdditiveSynth1.setParamValue("vol3",val5);
    faustAdditiveSynth1.setParamValue("vol4",val4);
    faustAdditiveSynth1.setParamValue("vol5",val3);
    faustAdditiveSynth1.setParamValue("vol6",val2);
    faustAdditiveSynth1.setParamValue("vol7",val1);
  
    faustAdditiveSynth2.setParamValue("vol1",val7);
    faustAdditiveSynth2.setParamValue("vol2",val2);
    faustAdditiveSynth2.setParamValue("vol3",val4);
    faustAdditiveSynth2.setParamValue("vol4",val5);
    faustAdditiveSynth2.setParamValue("vol5",val3);
    faustAdditiveSynth2.setParamValue("vol6",val1);
    faustAdditiveSynth2.setParamValue("vol7",val6);
    
    faustAdditiveSynth3.setParamValue("vol1",val6);
    faustAdditiveSynth3.setParamValue("vol2",val1);
    faustAdditiveSynth3.setParamValue("vol3",val3);
    faustAdditiveSynth3.setParamValue("vol4",val5);
    faustAdditiveSynth3.setParamValue("vol5",val4);
    faustAdditiveSynth3.setParamValue("vol6",val2);
    faustAdditiveSynth3.setParamValue("vol7",val7);

    faustAdditiveSynth4.setParamValue("vol1",val1);
    faustAdditiveSynth4.setParamValue("vol2",val2);
    faustAdditiveSynth4.setParamValue("vol3",val3);
    faustAdditiveSynth4.setParamValue("vol4",val4);
    faustAdditiveSynth4.setParamValue("vol5",val5);
    faustAdditiveSynth4.setParamValue("vol6",val6);
    faustAdditiveSynth4.setParamValue("vol7",val7);

    faustAdditiveSynth5.setParamValue("vol1",val7);
    faustAdditiveSynth5.setParamValue("vol2",val6);
    faustAdditiveSynth5.setParamValue("vol3",val5);
    faustAdditiveSynth5.setParamValue("vol4",val4);
    faustAdditiveSynth5.setParamValue("vol5",val3);
    faustAdditiveSynth5.setParamValue("vol6",val2);
    faustAdditiveSynth5.setParamValue("vol7",val1);
  
    faustAdditiveSynth6.setParamValue("vol1",val7);
    faustAdditiveSynth6.setParamValue("vol2",val2);
    faustAdditiveSynth6.setParamValue("vol3",val4);
    faustAdditiveSynth6.setParamValue("vol4",val5);
    faustAdditiveSynth6.setParamValue("vol5",val3);
    faustAdditiveSynth6.setParamValue("vol6",val1);
    faustAdditiveSynth6.setParamValue("vol7",val6);
    
    faustAdditiveSynth7.setParamValue("vol1",val6);
    faustAdditiveSynth7.setParamValue("vol2",val1);
    faustAdditiveSynth7.setParamValue("vol3",val3);
    faustAdditiveSynth7.setParamValue("vol4",val5);
    faustAdditiveSynth7.setParamValue("vol5",val4);
    faustAdditiveSynth7.setParamValue("vol6",val2);
    faustAdditiveSynth7.setParamValue("vol7",val7);    
}


// Callback for incoming NoteOn messages
// Handling the voice allocation here.
// Oscillator pairs are slighty detuned to give a chorus impression, mimicking analogue oscillator drift.

void myNoteOn(byte channel, byte note, byte velocity) {
  // When using MIDIx4 or MIDIx16, usbMIDI.getCable() can be used
  // to read which of the virtual MIDI cables received this message.

  for (int i=0; i <= 3; i++){
    if (StoredNotes[i] == -1) {
      StoredNotes[i] = int(note);
      
      switch (i){
        case 0:
          faustAdditiveSynth0.setParamValue("freq",NoteNumToFreq[note]); 
          faustAdditiveSynth0.setParamValue("gate",1);

          faustAdditiveSynth4.setParamValue("freq",NoteNumToFreq[note]*1.005); 
          faustAdditiveSynth4.setParamValue("gate",1);
        break;
        case 1:
          faustAdditiveSynth1.setParamValue("freq",NoteNumToFreq[note]*1.001);
          faustAdditiveSynth1.setParamValue("gate",1); 

          faustAdditiveSynth5.setParamValue("freq",NoteNumToFreq[note]*1.008);
          faustAdditiveSynth5.setParamValue("gate",1);          
        break;
        case 2:
          faustAdditiveSynth2.setParamValue("freq",NoteNumToFreq[note]*1.002);
          faustAdditiveSynth2.setParamValue("gate",1);

          faustAdditiveSynth6.setParamValue("freq",NoteNumToFreq[note]*1.006);
          faustAdditiveSynth6.setParamValue("gate",1);          
        break;
        case 3:
          faustAdditiveSynth3.setParamValue("freq",NoteNumToFreq[note]*1.003);
          faustAdditiveSynth3.setParamValue("gate",1);

          faustAdditiveSynth7.setParamValue("freq",NoteNumToFreq[note]*1.009);
          faustAdditiveSynth7.setParamValue("gate",1);          
        break;
      }

    break;
    }
  }   
}

// Callback for incoming NoteOff messages
// Releasing voices to be re-allocated here.

void myNoteOff(byte channel, byte note, byte velocity) {
  for (int i=0; i <= 3; i++){
    int k = int(note);
    
    if (StoredNotes[i] == k) {
      StoredNotes[i] = -1;
      
      switch (i){
        case 0:
          faustAdditiveSynth0.setParamValue("gate",0);
          faustAdditiveSynth4.setParamValue("gate",0);
        break;
        case 1:
          faustAdditiveSynth1.setParamValue("gate",0);
          faustAdditiveSynth5.setParamValue("gate",0);
        break;
        case 2:
          faustAdditiveSynth2.setParamValue("gate",0);
          faustAdditiveSynth6.setParamValue("gate",0);
        break;
        case 3:
          faustAdditiveSynth3.setParamValue("gate",0);
          faustAdditiveSynth7.setParamValue("gate",0);
        break;
      }
    }
  }  
}


// Callback for incoming CC messages
// Setting voice parameters directly or calling functions to set parameters in bulk here.

void myControlChange(byte channel, byte control, byte value) {

  float tempVol = float(value) / 127;
  
  switch (control) {
    case 1:
      faustAdditiveSynth0.setParamValue("vol0",tempVol);
      faustAdditiveSynth1.setParamValue("vol0",tempVol);   
      faustAdditiveSynth2.setParamValue("vol0",tempVol);
      faustAdditiveSynth3.setParamValue("vol0",tempVol); 
      
      faustAdditiveSynth4.setParamValue("vol0",tempVol);
      faustAdditiveSynth5.setParamValue("vol0",tempVol);   
      faustAdditiveSynth6.setParamValue("vol0",tempVol);
      faustAdditiveSynth7.setParamValue("vol0",tempVol);        
    break;
    case 2:
      setVolMacro(value);
    break;
    case 3:
      faustAdditiveSynth0.setParamValue("ctFreq",tempVol*9600 +30);
      faustAdditiveSynth1.setParamValue("ctFreq",tempVol*9700 +30);
      
      faustAdditiveSynth2.setParamValue("ctFreq",tempVol*9800 +30);
      faustAdditiveSynth3.setParamValue("ctFreq",tempVol*9500 +30); 

      faustAdditiveSynth4.setParamValue("ctFreq",tempVol*9600 +32);
      faustAdditiveSynth5.setParamValue("ctFreq",tempVol*9700 +32);
      
      faustAdditiveSynth6.setParamValue("ctFreq",tempVol*9800 +32);
      faustAdditiveSynth7.setParamValue("ctFreq",tempVol*9500 +32);         
    break;
    case 4:
      faustAdditiveSynth0.setParamValue("filterQ",tempVol*49 +1);
      faustAdditiveSynth1.setParamValue("filterQ",tempVol*49 +1);
       
      faustAdditiveSynth2.setParamValue("filterQ",tempVol*49 +1);
      faustAdditiveSynth3.setParamValue("filterQ",tempVol*49 +1); 
       
      faustAdditiveSynth4.setParamValue("filterQ",tempVol*49 +1);
      faustAdditiveSynth5.setParamValue("filterQ",tempVol*49 +1); 
       
      faustAdditiveSynth6.setParamValue("filterQ",tempVol*49 +1);
      faustAdditiveSynth7.setParamValue("filterQ",tempVol*49 +1);   
    break;
    case 5:
      setAttack(value);
    break;
    case 6:
      setDecay(value);
    break;
    case 7:
      setSustain(value);
    break;
    case 8:
      setRelease(value);
    break;
  break;
}
}

