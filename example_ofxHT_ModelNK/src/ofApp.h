#pragma once

#include "ofMain.h"

#include "ofxHandModel.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

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
		
		void factorChanged(float & factor); // gui event for factor
		void sizeChanged(float & _factor);

		ofxHandModel		hand;

		ofxPanel			gui;
		ofParameter<float>	factor;
		ofParameter<float>  size;
		//ofParameter<int>	mask;
		ofParameter<string> fps;
		
		short fingerMask;
		ofParameter<bool>   fingerEnabled[NUM_FINGERS];
};
