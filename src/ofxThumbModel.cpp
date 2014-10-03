#include "ofxThumbModel.h"


ofxThumbModel::ofxThumbModel(ofPoint _origin)
{
	root = ofxFingerSegment(_origin);
	
	//root.direction = ofPoint(-100, -100, 0);
	
	// it looks like this fixes problem with thumb initialization (strange big x angle vals)
	// this init also fixes debug build issues with thumb
	root.angleX = THUMB_MAX_ANGLE_X;
	mid.angleX = THUMB_MAX_ANGLE_X;
	top.angleX = THUMB_MAX_ANGLE_X;
	
	//thumb special ref angles setup
	root.refAngleX = -45;
	mid.refAngleX = -45;
	top.refAngleX = -45;

	mid.origin = root.origin + root.direction;
	mid.direction = root.direction;

	top.origin = mid.origin + mid.direction;
	top.direction = mid.direction;

	palm.origin = root.origin;

	fingerTip = top.origin + top.direction;

	root.length = 200;
	mid.length = 90;
	top.length = 70;

	root.update();
	mid.update();
	top.update();

	angleDiff = 10; // 1 deg per keypress
}

ofxThumbModel::ofxThumbModel(void)
{
}


ofxThumbModel::~ofxThumbModel(void)
{
}


void ofxThumbModel::update()
{
	root.update();
	mid.update();
	top.update();

	mid.origin = root.origin + root.direction;
	top.origin = mid.origin + mid.direction;
	fingerTip = top.origin + top.direction;
}

void ofxThumbModel::draw()
{
	ofSetColor(ofColor::cyan);
	ofSphere(fingerTip, 10);
	top.draw();
	mid.draw();
	root.draw();
}

void ofxThumbModel::keyPressed(int key){

	//cout << "Angle:" << root.angleX << "name: " << name << endl;

	// limit values by testing
	if(key == '-' && getAngleX() < THUMB_MAX_ANGLE_X) { // outer limit
		/*
		root.angleX += angleDiff;
		mid.angleX += 1.2*angleDiff;
		mid.update();
		top.angleX += 1.4*angleDiff;
		top.update();
		*/
		setAngleX(getAngleX() + angleDiff); // FIXED (by proper init of angleX): Debug build has issue here -> thumb rotates infinitely, Release working normally? -_-
		
	}
	else if(key == '+' && getAngleX() > THUMB_MIN_ANGLE_X) { // inner limit
		/*
		root.angleX -= angleDiff;
		mid.angleX -= 1.2*angleDiff;
		mid.update();
		top.angleX -= 1.4*angleDiff;
		top.update();
		*/
		setAngleX(getAngleX() - angleDiff);
	}
	/* defines angle a front limit - side view
			|
		¸\  |
		  \a|
	*/
	else if(key == '*'  && getAngleZ() < THUMB_MAX_ANGLE_Z /*-45*(180/PI)*/  /*-0.8f*/) {      
		/*
		root.angleZ += angleDiff;											
		mid.angleZ += angleDiff;
		mid.update();
		top.angleZ += angleDiff;
		top.update();
		*/
		setAngleZ(getAngleZ() + angleDiff);
	}
	/* defines angle back limit - side view
	      |
		¸ | /
		  |/a
	*/
	else if(key == '/' && getAngleZ() > THUMB_MIN_ANGLE_Z /*-103*(180/PI)*/ /*-1.8f*/) {
		/*
		root.angleZ -= angleDiff;
		mid.angleZ -= angleDiff;
		mid.update();
		top.angleZ -= angleDiff;
		top.update();
		*/
		setAngleZ(getAngleZ() - angleDiff);
	}
}

void ofxThumbModel::setAngleX(float _angle) {
	root.angleX = _angle;
	mid.angleX = 2*_angle;
	top.angleX = 3*_angle;
	update();
}

void ofxThumbModel::setAngleZ(float _angle) {
	root.angleZ = _angle;
	mid.angleZ = _angle;
	top.angleZ = _angle;
	update();
}

float ofxThumbModel::getAngleX() {
	return root.angleX /*+ root.refAngleX*/;
}

float ofxThumbModel::getAngleZ() {
	return root.angleZ /*+ root.refAngleZ*/;
}