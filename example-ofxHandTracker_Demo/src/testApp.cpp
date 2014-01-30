#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

	for(int i=0; i<256; i++)
		activeKeys[i] = false;

	ofSetBackgroundAuto(false); // removes fullscreen flickering (on some PCs)
	ofBackground(ofColor::white);

	h.origin.x = ofGetWidth()/2;
	h.origin.y = ofGetHeight()/2;

	//ofSetFullscreen(true);
	ofSetFrameRate(30);
	ofSetVerticalSync(true);

	wasGrabbed = false;
	wasReleased = true;

#ifdef USE_KINECT
	oniContext.setup();
	//oniContext.setMirror(true);

	depthGen.setup(&oniContext);
	imageGen.setup(&oniContext);

	userGen.setup(&oniContext);
	userGen.setSmoothing(0.1);				// built in openni skeleton smoothing...
	//userGen.setUseMaskPixels(true);
	userGen.setUseCloudPoints(true); // must be enabled to be able to draw cloud points
	userGen.setMaxNumberOfUsers(2);					// use this to set dynamic max number of users (NB: that a hard upper limit is defined by MAX_NUMBER_USERS in ofxUserGenerator)
	
	handGen.setup(&oniContext, 2);	
	handGen.setSmoothing(0.02);

	oniContext.toggleRegisterViewport();
	oniContext.toggleMirror();

	tracker = new ofxHandTracker(&userGen, &handGen, &depthGen, 0);
	//tracker2 = new ofxHandTracker(&userGen, &handGen, &depthGen, 1); // use when code is ready to track and handle multiple hands
#endif
}

//--------------------------------------------------------------
void testApp::update(){
#ifdef USE_KINECT
	oniContext.update();
	imageGen.update();
	depthGen.update();

	userGen.update(); //crashes app if imageGen not created
	
	try {
		tracker->update();
	}
	catch(const char *e)
	{
		std::cerr << "CRITICAL ERR (update): " << e;
	}
	//tracker2->update();

#endif

	thesisPresentation.update();

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

#ifdef USE_KINECT

	ofSetColor(ofColor::white);
	try {
		ofPoint hPos = tracker->getHandModel()->origin; // position in boundary dimensions 640x480

		float wRatioKinect = ofGetWidth()/640.0;
		float hRatioKinect = ofGetHeight()/480.0;
		ofPoint dispPos = ofPoint(hPos.x * wRatioKinect, hPos.y * hRatioKinect, hPos.z);

		/*if(thesisPresentation.hasFinished()) {	
				ofPushMatrix();
				ofPushStyle();
				ofSetColor(ofColor::white);
				//ofTranslate(0, ofGetHeight());
				imageGen.draw(640, 0, 640, 480);
				depthGen.draw(0, 0, 640, 480);
				userGen.draw();	
				//handGen.drawHands();
				//ofSetColor(255,255,255);
				tracker->draw();
				ofPopStyle();
				ofPopMatrix();
		}*/
		/*if(tracker->getNumFingerTips() == 0) {
			hud.mouseDragged(100*((hPos.x)/640.0)-10, 100*((hPos.y)/480.0)-10, 1);
		}*/
		
		/*ofPushStyle();
		ofSetColor(0,0,0);
		ofDrawBitmapString("PARAMS: " + ofToString(tracker->getHandModel()->saveFingerParameters().params), 100, 100);
		ofPopStyle();*/

		/*if (tracker->getHandModel()->saveFingerParameters().params == 16) {
			thesisPresentation.slideToPage(-1);
		}
		else if (tracker->getHandModel()->saveFingerParameters().params == 6) {
			thesisPresentation.slideToPage(1);
		}
		*/
		if (tracker->getNumFingerTips() == 1) {
			if(!wasGrabbed && dispPos.distance(ofPoint(0,0,0)) > 5 &&
				(dispPos.x > ofGetWidth()*0.25 && dispPos.x < ofGetWidth()*0.75)) {
				wasGrabbed = true;
				wasReleased = false;
				pointGrabbed = dispPos;
			}
		}
		else {
			if(!wasReleased) {
				wasReleased = true;
				wasGrabbed = false;

				if(abs(pointGrabbed.x - pointReleased.x) > ofGetWidth()/10) {
					if(pointGrabbed.x > pointReleased.x)
						thesisPresentation.slideToPage(-1);
					else
						thesisPresentation.slideToPage(1);
				}

				pointGrabbed = ofPoint(0,0,0);
				pointReleased = pointGrabbed;
			}
		}

		if (wasGrabbed) {
			pointReleased = dispPos;
			if (pointReleased.distance(pointGrabbed) > ofGetWidth()/2) {
				wasGrabbed = false;
				wasReleased = true;
				
				pointGrabbed = ofPoint(0,0,0);
				pointReleased = pointGrabbed;
			}
		}

		ofPushStyle();
		ofFill();
		ofSetLineWidth(4.0);
		ofSetColor(120, 120, 120, 64);
		ofLine(pointGrabbed, pointReleased);
		//ofDrawArrow(pointGrabbed, pointReleased, 10.0f);
		ofPopStyle();


		ofEnableAlphaBlending();
		ofPushMatrix();
		//ofScale(1.5, 1.5);
		//ofTranslate(hud.getTranslation().x, hud.getTranslation().y);
		ofPushStyle();
		//ofSetLineWidth(3.0);
		//ofNoFill();
		ofSetColor(128, 255, 0);
		//ofCircle(dispPos, 50);
		//ofSetColor(255, 240, 100);
		tracker->getHandModel()->getProjection().draw(dispPos - ofPoint(75, 75));
		ofPopStyle();
		ofPopMatrix();
		ofDisableAlphaBlending();
	}
	catch(const char *e) {
		std::cerr << "CRITICAL ERR (draw): " << e;
	}
	//tracker2->draw();
#endif

#ifdef	USE_KINECT
	// if no hands tracked draw demo counting or shifting
	/*if(handGen.getNumTrackedHands() == 0) {
		if(ofGetFrameNum()%10 == 0) {
			int shift = (ofGetFrameNum()/10)%10;
			int val = 1 << shift; // demo shifting

			ofxFingerParameters p = ofxFingerParameters(val); //(ofGetFrameNum()/10)%32 - counting
			h.restoreFrom(p);
		}
		h.origin = ofPoint(ofGetWidth()/2, ofGetHeight()/2, 0);
		h.scaling = ofPoint(.45,.45,.45);
		h.draw();
	}*/
#endif

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

#ifdef USE_KINECT
	if (tracker != NULL) {
		delete tracker;
		tracker = NULL;
	}

	// for another tracker
	//delete tracker2;
	//tracker2 = NULL;
#endif
}