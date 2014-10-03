#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	//hand = ofxHandModel();

	gui.setup("Model parameters:"); // most of the time you don't need a name but don't forget to call setup
	gui.add(closeFactor.set( "open/close:", 0.0, 0.0, 1.0));
	gui.add(spreadFactor.set( "spread/narrow:", 0.0, 0.0, 1.0));
	gui.add(size.set("scale size:", 0.5, 0.33, 1.5));
	//gui.add(mask.set("mask", 1, 1, 31));

	closeFactor.addListener(this, &ofApp::factorChanged);
	spreadFactor.addListener(this, &ofApp::factorChanged);
	size.addListener(this, &ofApp::sizeChanged);

	string fingerNames[] = {"thumb", "index", "middle", "ring", "pinky"};
	for (int i=0; i<NUM_FINGERS; i++) {
		gui.add(fingerEnabled[i].set(fingerNames[i], true));
	}
	fingerMask = 0;

	gui.add(fps.set("FPS:"));

	ofBackground(0);
}

void ofApp::factorChanged(float & factor) {
	// TODO: later listener could be avoken for detecting mask changes
	for (int i=0; i<NUM_FINGERS; i++) {
		if (fingerEnabled[i].get()) fingerMask |= (1 << (i));
		else						fingerMask &=  ~(1 << i);
	}
}

void ofApp::sizeChanged(float & _factor) {
	hand.setScale(_factor);
}

//--------------------------------------------------------------
void ofApp::update(){
	hand.spread(spreadFactor.get(), fingerMask);
	hand.close(closeFactor.get(), fingerMask);
	hand.update();

	fps = "FPS: " + ofToString(ofGetFrameRate());
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofPushMatrix();
		ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
		hand.draw();
	ofPopMatrix();

	ofPushStyle();
		//ofSetColor(ofColor::green);
		hand.drawFboProjection(ofPoint(0, ofGetHeight()*0.5));
	ofPopStyle();
		
	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	hand.mouseDragged(x,y,button);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	hand.mousePressed(x,y,button);
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
