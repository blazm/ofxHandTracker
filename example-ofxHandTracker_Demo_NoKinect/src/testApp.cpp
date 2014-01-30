#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	// setup multi keys flags
	for(int i=0; i<256; i++)
		activeKeys[i] = false;

    ofSetBackgroundAuto(false); // removes fullscreen flickering (on some PCs)
	ofBackground(ofColor::white);

	h.origin.x = ofGetWidth()/2;
	h.origin.y = ofGetHeight()/2;

	ofSetFrameRate(30);
	ofSetVerticalSync(true);

	wasGrabbed = false;
	wasReleased = true;
}

//--------------------------------------------------------------
void testApp::update(){

	thesisPresentation.update();

	// multiple key update
	for(int i=0; i<256; i++)
		if(activeKeys[i])
			h.keyPressed(i);
	
	h.update();

	int x = ofGetMouseX();
	int y = ofGetMouseY();

	h.origin.x = h.origin.x + ((x - h.origin.x)*0.3f);
	h.origin.y = h.origin.y + ((y - h.origin.y)*0.3f);
}

//--------------------------------------------------------------
void testApp::draw(){
	
	ofClear(ofColor::white); // prevents uncleared pixels (caused by auto background set to false in setup)

	ofPushMatrix();
	
	thesisPresentation.draw();

	ofImage proj = h.getProjection();

	ofEnableAlphaBlending();
	ofPushStyle();
	ofPushMatrix();

	ofTranslate(h.origin.x - IMG_DIM/2, h.origin.y - IMG_DIM/2);
	ofSetColor(0,255,0, 200);
	proj.draw(0, 0);

	ofPopMatrix();
	ofPopStyle();
	ofDisableAlphaBlending();

	ofPopMatrix();

	ofPushStyle();
	ofSetColor(200, 200, 255);
	ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate()), 20, -40, 0);
	ofPopStyle();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if(key < 256)
		activeKeys[key] = true;
	
	if(key == 'p')
		ofSetFullscreen(true);
	else if(key == 'l')
		ofSetFullscreen(false);
	else if(key == 'r')
		thesisPresentation.reset();
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	if(key < 256)
		activeKeys[key] = false;
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	h.mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	h.mousePressed(x, y, button);

	activeKeys['q'] = true;
	activeKeys['w'] = true;
	activeKeys['e'] = true;
	activeKeys['r'] = true;
	activeKeys['t'] = true;
	activeKeys['z'] = true;

	activeKeys['a'] = false;
	activeKeys['s'] = false;
	activeKeys['d'] = false;
	activeKeys['f'] = false;
	activeKeys['g'] = false;
	activeKeys['h'] = false;
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	activeKeys['q'] = false;
	activeKeys['w'] = false;
	activeKeys['e'] = false;
	activeKeys['r'] = false;
	activeKeys['t'] = false;
	activeKeys['z'] = false;

	activeKeys['a'] = true;
	activeKeys['s'] = true;
	activeKeys['d'] = true;
	activeKeys['f'] = true;
	activeKeys['g'] = true;
	activeKeys['h'] = true;
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	thesisPresentation.windowResized(w,h);
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void testApp::exit(){ 
}