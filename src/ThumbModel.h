#pragma once

#include "ofMain.h"
#include "FingerModel.h"

namespace ofxHT {
	using namespace Const;

class ThumbModel :
	public FingerModel
{
public:
	ThumbModel(ofPoint _origin);
	ThumbModel(void);
	~ThumbModel(void);

	void	update();
	void	draw();
	void	keyPressed(int key);
	
	void	setAngleX(float _angle);
	void	setAngleZ(float _angle);

	float	getAngleX();
	float	getAngleZ();
};

}