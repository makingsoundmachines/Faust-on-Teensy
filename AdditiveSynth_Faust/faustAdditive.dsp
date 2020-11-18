
import("stdfaust.lib");

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Additive synthesizer (harmonic oscillator), originally for BELA, adapted for Teensy.
//
// Original project here: 
// https://github.com/grame-cncm/faust/blob/master-dev/examples/bela/AdditiveSynth.dsp
//
// It has 8 harmonics, each with its own ADSR volume envelope.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Parameters available in Arduino export:
//
// General Parameters - Gate, Pitch, Velocity
// gate : Note on or off
// freq : The fundametal pitch of the harmonic oscillator
// gain : The volume / velocity
//
// For each harmonic (%rang indicates harmonic number, starting at 0):
//
// vol%rang : Volume of harmonic - vol0 (fundamental), vol1 (first harmonic) .. vol7
// A%rang : Attack
// D%rang : Decay
// S%rang : Sustain
// R%rang : Release
//
// For the filter:
//
// ctFreq     : center Frequency (cutoff)
// filterQ    : quality of the filter
// filterGain : makeup gain
//
///////////////////////////////////////////////////////////////////////////////////////////////////


// Teensy Oscillator
// lowering the resolution of the sine oscillators' lookup table 
// in order to make computation easier on the Teensy MCU and memory

oscTeensy(f) = rdtable(tablesize, os.sinwaveform(tablesize), int(os.phasor(tablesize,f)))
with{
    tablesize = 1 << 15; // instead of 1 << 16
};

// gate, freq (pitch), gain parameters
gate = button("gate"); 
freq = hslider("freq[unit:Hz]", 440, 20, 20000, 1);
gain = hslider("gain", 0.5, 0, 10, 0.01);

// Filter Parameters
ctFreq = hslider("ctFreq",500,20,10000,0.01) : si.smoo;
filterQ = hslider("filterQ", 5, 0, 10, 1);
filterGain = hslider("filterGain",1,0,1,0.01) : si.smoo;

// Parameters for the harmonic oscillator
harmonic(rang) = oscTeensy(freq*(rang+1))*volume
    with {
        // UI
        vol = hslider("vol%rang", 1, 0, 1, 0.001) : si.smooth(ba.tau2pole(0.01));
     
        a = 0.01 * hslider("A%rang", 1, 0, 400, 0.001) : si.smoo;
        d = 0.01 * hslider("D%rang", 1, 0, 400, 0.001) : si.smoo;
        s = hslider("S%rang", 1, 0, 1, 0.001) : si.smoo;
        r = 0.01 * hslider("R%rang", 1, 0, 800, 0.001) : si.smoo;

        volume = ((en.adsr(a,d,s,r,gate))*vol) : max (0) : min (1);
    };

process = par(i, 8, harmonic(i)) :> / (8) : fi.resonlp(ctFreq, filterQ, filterGain);

