#pragma once

#include "ofMain.h"
#include "ofxFingerSegment.h"
#include "ofxHandParameters.h"

class ofxFingerModel
{
	public:
		ofxFingerModel(ofPoint origin);
		ofxFingerModel(void);
		~ofxFingerModel(void);

		virtual void update();
		virtual void draw();
		virtual void keyPressed(int key);
		virtual void setLength(float rootL, float midL, float topL);

		//float degToRad(float deg);
		//float radToDeg(float rad);

		virtual void setAngleZ(float _angle);
		virtual float getAngleZ();
	
		virtual void setAngleX(float _angle);
		virtual float getAngleX();

		ofxFingerSegment root;
		ofxFingerSegment mid;
		ofxFingerSegment top;

		ofPoint fingerTip; 

		float angleDiff;
};

