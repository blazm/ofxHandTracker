#pragma once

#include "ofMain.h"
#include "ofxFingerModel.h"

class ofxThumbModel :
	public ofxFingerModel
{
public:
	ofxThumbModel(ofPoint _origin);
	ofxThumbModel(void);
	~ofxThumbModel(void);

	void	update();
	void	draw();
	void	keyPressed(int key);
	
	void	setAngleX(float _angle);
	void	setAngleZ(float _angle);

	float	getAngleX();
	float	getAngleZ();
};

