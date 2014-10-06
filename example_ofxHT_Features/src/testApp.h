// @author: blazm
// @about: this example requires kinect, showing features of hand tracker class

#pragma once

#include "ofMain.h"
#include "ofxHT.h"
#include "ofxOpenNI.h"
#include "ofxOpenCv.h"
#include "HUD.h"

#define USE_KINECT
#define NUM_DEMO_PARAMS_X		6
#define NUM_DEMO_PARAMS_Y		4

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void exit(); // cleanup on exit

		ofxHT::HandModel h;
		ofPoint boxPos;
		HUD hud;

		// for multiple keys to be active at the same time
		bool activeKeys[256];

#ifdef USE_KINECT
		ofxHT::HandTracker *tracker;
		ofxHT::HandTracker *tracker2; // if not pointer it causes heap corruption (too much data to be stored on heap i guess)
#endif

#ifdef USE_KINECT
		// OpenNI init
		ofxOpenNIContext	oniContext;
		ofxDepthGenerator	depthGen;
		ofxIRGenerator		imageGen;
		//ofxImageGenerator   imageGen;
		//ofxOpenNIRecorder	oniRecorder;
		ofxUserGenerator	userGen;
		ofxHandGenerator	handGen;
#endif

		// points for grabbing & releasing references
		ofPoint				pointGrabbed;
		ofPoint				pointReleased;

		bool				wasGrabbed;
		bool				wasReleased;
};
