
#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    //// WARNING: ALWAYS SET OFMAP TO "TRUE" TO CLAMP WHEN WORKING WITH AUDIO AND CAREFUL WEARING HEADPHONES OTHERWISE YOU CAN DAMAGE YOUR EARS ////
    
    
    ofBackground(50);
    
    // 2 output channels,
    // 0 input channels
    // 22050 samples per second
    // 512 samples per buffer
    // 4 num buffers (latency)
    
    int bufferSize		= 512;
    sampleRate 			= 44100;
    audioOutLeft.resize(bufferSize);
    audioOutRight.resize(bufferSize);
    
    soundStream.listDevices();
    
    soundStream.setDeviceID(1);
    soundStream.setup(this, 2, 0, sampleRate, bufferSize, 4);
    
    pan = 1.0;
    
    // Set up one oscillator but passing in sample rate and buffersize //
    // Set intial oscillator frequency //
    // Set envelope parameters //
    
    triWave.setup(soundStream.getSampleRate(), soundStream.getBufferSize());
    triWave.setFrequency(440);
    
    sineWave.setup(soundStream.getSampleRate(), soundStream.getBufferSize());
    sineWave.setFrequency(220);
    
    sawWave.setup(soundStream.getSampleRate(), soundStream.getBufferSize());
    sawWave.setFrequency(220);
    
    env.set(0.9, 0.8 ); //these are both 0.0 - 1.0 anything above is dangerous
    //attack 0.0 - 1.0
    //release 0.0 - 1.0
    
    
    //setup ofxGui
    gui.setup("ofxGenSound");
    gui.add(isCreateMode.set("isCreateMode", true));
    gui.add(boxSize.set("boxSize", 100, 0, 800));
    gui.add(boxAudioScale.set("boxAudioScale", 100, 0, 800));
    gui.add(freq1.set("Triangle Wave Freq", 440, 50, 4000));
    gui.add(freq2.set("Sine Wave Freq", 220, 50, 4000));
    gui.add(freq3.set("Saw Wave Freq", 220, 50, 4000));
    gui.add(filterCutoff.set("Filter Cutoff", 0.5, 0.0, 0.5));
    gui.add(delayFeedback.set("Delay Feedback", 0.8, 0.0, 0.9));
    
    parameters.push_back(&freq1);
    parameters.push_back(&freq2);
    parameters.push_back(&freq3);
    parameters.push_back(&filterCutoff);
    parameters.push_back(&delayFeedback);
    
    
    ofSetWindowTitle("ofxGenSound Example");
    
    
    // print input ports to console
    midiIn.listPorts(); // via instance
    //ofxMidiIn::listPorts(); // via static as well
    
    // open port by number (you may need to change this)
    midiIn.openPort(3);
    //midiIn.openPort("IAC Pure Data In");	// by name
    //midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port
    
    // don't ignore sysex, timing, & active sense messages,
    // these are ignored by default
    midiIn.ignoreTypes(false, false, false);
    
    // add ofApp as a listener
    midiIn.addListener(this);
    
    // print received messages to the console
    midiIn.setVerbose(true);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    //trigger an envelope
    bool isTrigger = midiMessage.velocity==127 ? true : false;
    env.trigger(isTrigger);
    prevVelocity = midiMessage.velocity;
    if(!isCreateMode){
        int i = 0;
        for(auto &p : parameters){
            *p = *p * 0.9 + 0.1 * paramTargets[curParamSet][i];
            i++;
        }
    }
    freq1 = midiMessage.pitch*2;
    freq2 = midiMessage.pitch*2;
    freq3 = midiMessage.pitch*2;
    //control frequency
    float freqOffset = ofMap(mouseY, 0, ofGetHeight(), 0, 800, true);
    triWave.setFrequency(freq1+ freqOffset);
    sineWave.setFrequency(freq2+ freqOffset);
    sawWave.setFrequency(freq3+ freqOffset);
    
    //control filter cutoff
    filter.setCutoff(filterCutoff);
    
    delay.setFeedback(delayFeedback);
    delay.setMix(0.5);
}

//--------------------------------------------------------------
void ofApp::draw(){

    
    ofVec2f vec = ofVec2f(0, ofGetHeight() * .5);
    drawWaveform(vec);
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::audioOut(float * output, int bufferSize, int nChannels){
    
    // This is your left and right speakers //
    line1.clear();
    float leftScale = -1.0;
    float rightScale = 1.0;
    
    // This runs through the audio buffer at the rate of the audioOut core audio event //
    
    oscType type[4]{ OF_SINE_WAVE, OF_TRIANGLE_WAVE, OF_SAWTOOTH_WAVE, OF_PHASOR_WAVE};
    for (int i = 0; i < bufferSize; i++){
        
        // Signal Chain //
        float count = ofMap(midiMessage.deltatime, 0, 500, 0, 10);
        int step = midiMessage.pitch;
        float waveOutl = 0;
        for (int j = 0; j < count; j++) {
            waveOutl += triWave.setOscillatorType(OF_SAWTOOTH_WAVE);
        }
        waveOutl *= env.addEnvelope();
        float waveOutr = 0;
        for (int j = 0; j < count; j++) {
            waveOutr += sineWave.setOscillatorType(OF_SINE_WAVE);
        }
        waveOutr *= env.getVolume();
        
         
        float delayOut = delay.addDelay(waveOutl);
        float delayOut2 = delay.addDelay(waveOutr);

        float filterOutl = filter.addFilter(OF_FILTER_LP, delayOut);
        float filterOutr = filter.addFilter(OF_FILTER_LP, delayOut2);
        
        // Output Sound //
        audioOutRight[i] = output[ i * nChannels    ] = ofClamp(filterOutl * rightScale, -1,1);
        audioOutLeft[i] = output[ i * nChannels + 1 ] = ofClamp(filterOutr * leftScale, -1, 1);
        
    }
    
}
void ofApp::newMidiMessage(ofxMidiMessage& msg) {
    // make a copy of the latest message
    midiMessage = msg;
}

void ofApp::drawWaveform(ofVec2f& position) {
    float size = boxSize;
    ofPolyline line;
    line.addVertex(0, 0);
    line.addVertex(size, 0);
    line.addVertex(size, size);
    line.addVertex(0, size);
    line.addVertex(0, 0);
    
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
    ofColor c = c.turquoise;
    ofSetColor(c);
    ofNoFill();
    ofBeginShape();
    
//    float lineStep = audioOutRight.size()/line.getPerimeter();
//    line = line.getResampledBySpacing(lineStep);
//    for (int i = 0; i < line.getVertices().size(); i++) {
//        ofPoint p = line.getVertices()[i];
//        ofVec2f tangent = line.getTangentAtIndex(i);
//        ofVec2f p1 = p + tangent.getRotated(90);
//        p1.x = p1.x + audioOutLeft[i]*boxAudioScale;
//        p1.y = p1.y + audioOutRight[i]*boxAudioScale;
//        ofVertex(p1.x, p1.y);
//    }
    
    float step = audioOutRight.size()/TWO_PI;
    for (int i = 0; i < audioOutRight.size(); i++) {
        float y = cos(i/step - audioOutLeft[i]) * 200;
        float x = sin(i/step + audioOutRight[i]) * 200 ;
        ofVertex(x, y);
    }
    ofEndShape();
    

//    line.draw();
    
    ofPopMatrix();
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if(key-49>-1 && key-49<9){
    
        if(isCreateMode){
            int i = 0;
            paramTargets[key-49] = new float[9];
            for(auto &p : parameters){
                paramTargets[key-49][i] = *p;
                i++;
            }
        }else{
            curParamSet = key-49;
        }
    }
    if(key == 'a'){ freq1 = 82.41; freq2 = freq1; freq3 = freq2;}
    if(key == 's') {freq1 = 87.31; freq2 = freq1; freq3 = freq2;}
    if(key == 'd'){ freq1 = 92.50; freq2 = freq1; freq3 = freq2;}
if(key == 'f'){ freq1 = 98.00; freq2 = freq1; freq3 = freq2;}
if(key == 'g'){ freq1 = 103.83; freq2 = freq1; freq3 = freq2;}
if(key == 'h'){ freq1 = 110.00; freq2 = freq1; freq3 = freq2;}
if(key == 'j'){ freq1 = 116.54; freq2 = freq1; freq3 = freq2;}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
