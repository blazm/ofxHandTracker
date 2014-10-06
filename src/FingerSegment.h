#pragma once

#include "ofMain.h"

namespace ofxHT {

class FingerSegment
{
	public:
		FingerSegment(ofPoint _origin);
		FingerSegment(void);
		~FingerSegment(void);

		void update();
		void updateWithX();
		void draw();

		ofPoint origin;
		ofVec3f direction;
		
		float angleZ;
		float angleX;
		float length;

		// important beacuse of thumb finger which needs special handling
		// also important cause replaces rotation by constant 90 around X
		float refAngleZ; //reference angle -> initial & constant startup angleZ 
		float refAngleX; //reference angle -> initial & constant startup angleX

		//ofMatrix4x4 rotationMatrix;
		ofMatrix4x4 rotateByX, rotateByZ;
};

}