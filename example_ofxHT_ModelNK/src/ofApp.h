#pragma once

#include "ofMain.h"

#include "HandModel.h"
#include "TrackerConstants.h"

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

		ofxHT::HandModel	hand;

		ofxPanel			gui;
		ofParameter<float>	closeFactor;
		ofParameter<float>	spreadFactor;
		ofParameter<float>  size;
		//ofParameter<int>	mask;
		ofParameter<string> fps;
		
		short fingerMask;
		ofParameter<bool>   fingerEnabled[ofxHT::Const::NUM_FINGERS];
};
