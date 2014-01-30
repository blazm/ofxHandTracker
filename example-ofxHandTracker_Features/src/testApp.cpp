#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	boxPos = h.origin - ofPoint(-ofGetWidth()/2, 0, 0);

	for(int i=0; i<256; i++)
		activeKeys[i] = false;

    ofSetBackgroundAuto(false); // removes fullscreen flickering (on some PCs)
	ofBackground(ofColor::black);

	//hud = HUD(ofPoint(10,10,0), ofColor(64, 128, 96, 128), 100, 100);
	//hud = HUD(ofPoint(10, ofGetHeight() - ofGetHeight()/2 - 20, 0));
	hud = HUD(ofPoint(10,10,0), 120, 60, 2.0f, 2.0f); // init hud with number of pages - always locks number of pages (no matter of resolution)
	//hud = HUD(ofPoint(10, 870, 0), 250, 150, 2000, 7000); // init hud with locked resolution to be available (you want 10k x 8k res or even more? fine, just set it up)
	//hud = HUD(ofPoint(0, 0, 0), 100, 1000, 1000, 10000); // setup for ppt
	//hud = HUD(ofPoint(0, 0, 0), 1, 1, 1.0f, 1.0f);
	hud.setColor(ofColor(64, 128, 96, 64), ofColor(255, 96, 32, 150));
	
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

#ifdef USE_KINECT
	// demo drawing app setup
	/*drawBuffer.allocate(640, 480);
	prevIndexPos = ofPoint(-1,-1,-1);
	prevMidPos = ofPoint(-1,-1,-1);
	prevPinkyPos = ofPoint(-1,-1,-1);*/
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

	hud.update();
	
	for(int i=0; i<256; i++)
		if(activeKeys[i])
			h.keyPressed(i);
	
	h.update();
	
	// quick box moving demo 
	if(abs(h.getIndexFingerWorldCoord().z - boxPos.z) < 20
	&& abs(h.getIndexFingerWorldCoord().y - boxPos.y) < 20
	&& abs(h.getIndexFingerWorldCoord().x - boxPos.x) < 20){
		boxPos.x = h.getIndexFingerWorldCoord().x;
		boxPos.y = h.getIndexFingerWorldCoord().y;
		boxPos.z = h.getIndexFingerWorldCoord().z;
	}

	int x = ofGetMouseX();
	int y = ofGetMouseY();

	x += hud.getTranslation().x;
	y += hud.getTranslation().y;

	h.origin.x = h.origin.x + ((x - h.origin.x)*0.3f);
	h.origin.y = h.origin.y + ((y - h.origin.y)*0.3f);
}

//--------------------------------------------------------------
void testApp::draw(){

	ofClear(ofColor::black); // prevents uncleared pixels (caused by auto background set to false in setup)

	ofPushMatrix();
	ofTranslate(-hud.getTranslation().x, -hud.getTranslation().y);

#ifdef USE_KINECT

	ofSetColor(ofColor::white);
	try {
		ofPoint hPos = tracker->getHandModel()->origin;
		
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
		
		// this enables user to move around using fist
		if(tracker->getNumFingerTips() == 0) {
			hud.mouseDragged(100*((hPos.x)/640.0)-10, 100*((hPos.y)/480.0)-10, 1);
		}
		
		/*
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
		*/

		float wRatioKinect = ofGetWidth()/640.0;
		float hRatioKinect = ofGetHeight()/480.0;
		ofPoint dispPos = ofPoint(hPos.x * wRatioKinect, hPos.y * hRatioKinect, hPos.z); // position in boundary dimensions 640x480

		ofPushMatrix();
		//ofScale(1.5, 1.5);
		ofTranslate(hud.getTranslation().x, hud.getTranslation().y);
		ofPushStyle();
		//ofSetLineWidth(3.0);
		//ofNoFill();
		ofSetColor(128, 255, 0);
		//ofCircle(dispPos, 50);
		//ofSetColor(255, 240, 100);
		tracker->getHandModel()->getProjection().draw(dispPos - ofPoint(75, 75));
		ofPopStyle();
		ofPopMatrix();
	}
	catch(const char *e) {
		std::cerr << "CRITICAL ERR (draw): " << e;
	}

	//tracker2->draw();
#endif
	
#ifdef	USE_KINECT
	// if no hands tracked draw demo counting or shifting
	// this is parameters saving/restoring demo
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

#ifdef USE_KINECT	
	// demo drawing app logic
	/*
	vector<ofPoint> activeFingers = tracker->getActiveFingerTips();

	ofxHandModel *hRef = tracker->getHandModel();

	float indexAngleZ = hRef->f[1]->getAngleZ();
	float midAngleZ = hRef->f[2]->getAngleZ();
	//float ringAngleZ = hRef->f3.getAngleZ();
	float pinkyAngleZ = hRef->f[4]->getAngleZ();

	ofSetColor(ofColor::white);

	if(indexAngleZ < 10) {
		ofSetLineWidth(5.0f);
		
		ofSetColor(ofColor::blue);

		ofPoint indexPos = hRef->getFingerWorldCoord(1)[3];
		//fingerPos.z = hRef->origin.z;
		if(prevIndexPos.distance(ofPoint(-1,-1,-1)) > 0.5) {
			drawBuffer.begin();
				ofLine(indexPos, prevIndexPos);
			drawBuffer.end();
		}

		ofSetLineWidth(1.0f);
	
		prevIndexPos = indexPos;
	}	
	else {
		prevIndexPos = ofPoint(-1,-1,-1);
	}

	if(midAngleZ < 10) {
		ofSetLineWidth(5.0f);
		
		//ofSetColor(ofRandom(255));
		ofSetColor(ofColor::cyan);

		ofPoint midPos = hRef->getFingerWorldCoord(2)[3];
		if(prevMidPos.distance(ofPoint(-1,-1,-1)) > 0.5) {
			drawBuffer.begin();
				ofLine(midPos, prevMidPos);
			drawBuffer.end();
		}

		ofSetLineWidth(1.0f);
	
		prevMidPos = midPos;
	}
	else {
		prevMidPos = ofPoint(-1,-1,-1);
	}

	if(pinkyAngleZ < 10) {
		ofSetLineWidth(5.0f);
		
		//ofSetColor(ofRandom(255));
		ofSetColor(ofColor::magenta);

		ofPoint pinkyPos = hRef->getFingerWorldCoord(4)[3];
		if(prevPinkyPos.distance(ofPoint(-1,-1,-1)) > 0.5) {
			drawBuffer.begin();
				ofLine(pinkyPos, prevPinkyPos);
			drawBuffer.end();
		}

		ofSetLineWidth(1.0f);
	
		prevPinkyPos = pinkyPos;
	}
	else {
		prevPinkyPos = ofPoint(-1,-1,-1);
	}
	
	if(pinkyAngleZ > 80 && indexAngleZ > 80 && midAngleZ > 80) {
		prevIndexPos = ofPoint(-1,-1,-1);
		prevMidPos = ofPoint(-1,-1,-1);
		prevPinkyPos = ofPoint(-1,-1,-1);
		drawBuffer.begin(); 
			//glClearColor(0.0, 0.0, 0.0, 255.0);    
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
			//ofClear(0, 255);
			ofClearAlpha();
			ofSetColor(0,0,0,64);
			ofRect(0, 0, 640, 480);
		drawBuffer.end();  
	}

	drawBuffer.draw(0, 0, 640, 480);*/
	// demo drawing app logic end
#endif

	ofPopMatrix();
	hud.draw();

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
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	if(key < 256)
		activeKeys[key] = false;
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	hud.mouseMoved(x, y);
	
	x += hud.getTranslation().x;
	y += hud.getTranslation().y;
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	hud.mouseDragged(x, y, button);

	x += hud.getTranslation().x;
	y += hud.getTranslation().y;

	h.mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	hud.mousePressed(x, y, button);

	x += hud.getTranslation().x;
	y += hud.getTranslation().y;

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
	hud.mouseReleased(x, y, button);

	x += hud.getTranslation().x;
	y += hud.getTranslation().y;

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
	//hud.setMapSize(w/10, h/10);
	hud.windowResized(w,h);
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

	// if using another tracker
	//delete tracker2;
	//tracker2 = NULL;
#endif
}