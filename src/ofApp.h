#pragma once

#include "ofMain.h"
#include "ofxGenSound.h"
#include "ofxGui.h"
#include "ofxMidi.h"

class ofApp : public ofBaseApp, public ofxMidiListener{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    // midi
    void newMidiMessage(ofxMidiMessage& eventArgs);
    ofxMidiIn midiIn;
    ofxMidiMessage midiMessage;
    int prevVelocity = 0;
    
    void audioOut(float * input, int bufferSize, int nChannels);
    
    ofSoundStream soundStream;
    int sampleRate;
    vector <float> audioOutLeft;
    vector <float> audioOutRight;
    float pan;
    void drawWaveform(ofVec2f& position);
    
    //ofxGenSound
    ofPolyline line1;
    
    ofxGenSoundOsc triWave, sineWave, sawWave;
    ofxGenSoundEnvelope env;
    ofxGenSoundDelay delay;
    ofxGenSoundFilter filter;
    
    // settings
    vector<ofParameter<float> *> parameters;
    map<int, float * > paramTargets;
    int curParamSet = 0;
    //GUI
    ofxPanel gui;
    ofParameter<bool> isCreateMode;
    ofParameter<float> freq1, freq2, freq3, filterCutoff, delayFeedback;
    ofParameter<float> boxSize, boxAudioScale;
};

/*
 Example:
 
 - Press any key to generate a tone.
 
 - Move the mouse left and right to change frequency.
 
 - Move the mouse up and down to change filter.
 
 */
