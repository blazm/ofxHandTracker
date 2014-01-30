#include "ofxFingerModel.h"


ofxFingerModel::ofxFingerModel(ofPoint _origin) 
{
	root = ofxFingerSegment(_origin);

	// origin + direction changes
	mid.origin = root.origin + root.direction;
	mid.direction = root.direction;
	top.origin = mid.origin + mid.direction;
	top.direction = mid.direction;

	// x angle changes
	mid.angleX = root.angleX;
	top.angleX = mid.angleX;

	root.length = 100;
	mid.length = 90;
	top.length = 80;

	root.update();
	mid.update();
	top.update();

	angleDiff = 10; // 1 deg per keypress
}

ofxFingerModel::ofxFingerModel(void)
{
	angleDiff = 10; // 1 deg per keypress

	// origin + direction changes
	mid.origin = root.origin + root.direction;
	mid.direction = root.direction;
	top.origin = mid.origin + mid.direction;
	top.direction = mid.direction;

	// x angle changes
	mid.angleX = root.angleX;
	top.angleX = mid.angleX;

	root.length = 100;
	mid.length = 90;
	top.length = 80;

	root.update();
	mid.update();
	top.update();
}


ofxFingerModel::~ofxFingerModel(void)
{
}

void ofxFingerModel::update(){
	root.update();
	mid.update();
	top.update();

	// origin changes
	mid.origin = root.origin + root.direction;
	top.origin = mid.origin + mid.direction;
	fingerTip = top.origin + top.direction;

	// x angle changes
	mid.angleX = root.angleX;
	top.angleX = mid.angleX;
}

void ofxFingerModel::draw()
{
	ofSetColor(ofColor::cyan);
	ofSphere(fingerTip, 15);
	top.draw();
	mid.draw();
	root.draw();
}

void ofxFingerModel::keyPressed(int key){

	//cout << "Angle:" << root.angleZ << "name: " << name << endl;

	if(key == '+' && getAngleZ() < FINGER_MAX_ANGLE_Z) {
		/*
		root.angleZ += angleDiff;
		mid.angleZ += 2*angleDiff;
		mid.update();
		top.angleZ += 3*angleDiff;
		top.update();
		*/
		setAngleZ(getAngleZ() + angleDiff);
	}
	else if(key == '-' && getAngleZ() > FINGER_MIN_ANGLE_Z /*-PI/2*/) {
		/*
		root.angleZ -= angleDiff;
		mid.angleZ -= 2*angleDiff;
		mid.update();
		top.angleZ -= 3*angleDiff;
		top.update();
		*/
		setAngleZ(getAngleZ() - angleDiff);
	}

	/*if(key == '/' && root.angleX < maxAngleX) {
		root.angleX += PI/30;
		mid.angleX += 2*PI/30;
		mid.update();
		top.angleX += 3*PI/30;
		top.update();
	}
	else if(key == '*' && root.angleX > minAngleX) {
		root.angleX -= PI/30;
		mid.angleX -= 2*PI/30;
		mid.update();
		top.angleX -= 3*PI/30;
		top.update();
	}*/
}

void ofxFingerModel::setLength(float rootL, float midL, float topL) {
	root.length = rootL;
	mid.length = midL;
	top.length = topL;
}

void ofxFingerModel::setAngleZ(float _angle) {
	root.angleZ = _angle;
	mid.angleZ = 2*_angle;
	top.angleZ = 3*_angle;
	update();
}

float ofxFingerModel::getAngleZ() {
	return root.angleZ;
}

void ofxFingerModel::setAngleX(float _angle) {
	root.angleX = _angle;
	mid.angleX = _angle;
	top.angleX = _angle;
	update();
}

float ofxFingerModel::getAngleX() {
	return root.angleX;
}

/*
float ofxFingerModel::degToRad(float deg) {
	return (deg*PI)/180;
}

float ofxFingerModel::radToDeg(float rad) {
	return (rad*180)/PI;
}*/