// @author blazm
// @about this example works without kinect, just to see kinematic hand model and demo presentation in action

#pragma once

#include "ofMain.h"
#include "ofxHandModel.h"
// these are not needed in this example
//#include "ofxHandTracker.h"
//#include "ofxOpenNI.h"
//#include "ofxOpenCv.h"
#include "Presentation.h"

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
	
		void exit(); // cleanup on exit (not needed yet here)

		ofxHandModel h; // for demo of hand model
		Presentation		thesisPresentation;

		// for multiple keys to be active at the same time
		bool activeKeys[256];
		
		// points for grabbing & releasing references
		ofPoint				pointGrabbed;
		ofPoint				pointReleased;

		bool				wasGrabbed;
		bool				wasReleased;
};
