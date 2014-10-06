#pragma once

#include "ofMain.h"
#include "FingerSegment.h"
#include "FingerParameters.h"

namespace ofxHT {
	using namespace Const;

class FingerModel
{
	public:
		FingerModel(ofPoint origin);
		FingerModel(void);
		~FingerModel(void);

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

		FingerSegment palm; // segment inside palm
		FingerSegment root;
		FingerSegment mid;
		FingerSegment top;
		
		ofPoint fingerTip; 

		float angleDiff;
};

}