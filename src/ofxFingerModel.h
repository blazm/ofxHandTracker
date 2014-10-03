#pragma once

#include "ofMain.h"
#include "ofxFingerSegment.h"
#include "ofxFingerParameters.h"

static float _GOLDEN_RATIO = 1.618f; //033988749894848204586834;
static float _GOLDEN_RATIO_x2 = _GOLDEN_RATIO * _GOLDEN_RATIO;
static float _GOLDEN_RATIO_x3 = _GOLDEN_RATIO_x2 * _GOLDEN_RATIO;

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
		virtual void setTipLength(float _tipLength);

		//float degToRad(float deg);
		//float radToDeg(float rad);

		virtual void setAngleZ(float _angle);
		virtual float getAngleZ();
	
		virtual void setAngleX(float _angle);
		virtual float getAngleX();

		ofxFingerSegment palm; // segment inside palm
		ofxFingerSegment root;
		ofxFingerSegment mid;
		ofxFingerSegment top;
		
		ofPoint fingerTip; 

		float angleDiff;
};

