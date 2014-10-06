#include "FingerModel.h"

namespace ofxHT {

FingerModel::FingerModel(ofPoint _origin) 
{
	root = FingerSegment(_origin);

	// origin + direction changes
	mid.origin = root.origin + root.direction;
	mid.direction = root.direction;
	top.origin = mid.origin + mid.direction;
	top.direction = mid.direction;

	palm.angleX = 0;
	palm.angleZ = 0;
	palm.origin = root.origin;
	palm.direction = -root.direction;
	palm.length = -160;

	// x angle changes
	mid.angleX = root.angleX;
	top.angleX = mid.angleX;
	
	root.length = 100;
	mid.length = 90;
	top.length = 80;

	palm.update();
	root.update();
	mid.update();
	top.update();

	angleDiff = 10; // 1 deg per keypress
}

FingerModel::FingerModel(void)
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

	palm.angleX = 0;
	palm.angleZ = 0;
	palm.origin = root.origin;
	palm.direction = -root.direction;
	palm.length = -160;

	root.length = 100;
	mid.length = 90;
	top.length = 80;

	palm.update();
	root.update();
	mid.update();
	top.update();
}

FingerModel::~FingerModel(void){}

void FingerModel::update(){
	palm.update();
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

void FingerModel::draw()
{
	ofSetColor(ofColor::cyan);
	ofSphere(fingerTip, 10);
	top.draw();
	mid.draw();
	root.draw();
	palm.draw();
}

void FingerModel::keyPressed(int key){

	if(key == '+' && getAngleZ() < FINGER_MAX_ANGLE_Z) {
		setAngleZ(getAngleZ() + angleDiff);
	}
	else if(key == '-' && getAngleZ() > FINGER_MIN_ANGLE_Z /*-PI/2*/) {
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

void FingerModel::setLength(float rootL, float midL, float topL) {
	root.length = rootL;
	mid.length = midL;
	top.length = topL;
}

void FingerModel::setTipLength(float _len) {
	setLength(_len * _GOLDEN_RATIO_x2, _len * _GOLDEN_RATIO, _len);
	palm.length = -_len * _GOLDEN_RATIO_x3 * 1.2;
}

void FingerModel::setAngleZ(float _angle) {
	root.angleZ = _angle;
	mid.angleZ = 2*_angle;
	top.angleZ = 3*_angle;
	update();
}

float FingerModel::getAngleZ() {
	return root.angleZ;
}

void FingerModel::setAngleX(float _angle) {
	root.angleX = _angle;
	mid.angleX = _angle;
	top.angleX = _angle;
	update();
}

float FingerModel::getAngleX() {
	return root.angleX;
}

/*
float FingerModel::degToRad(float deg) {
	return (deg*PI)/180;
}

float FingerModel::radToDeg(float rad) {
	return (rad*180)/PI;
}*/

}