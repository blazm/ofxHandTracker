#pragma once

#include "ofMain.h"

class HUD
{
public:
	HUD(void);
	HUD(ofPoint _mapPos, ofColor _mapColor, int _w, int _h);
	HUD(ofPoint _mapPos, int _mapWidth, int _mapHeight, float _wPages, float _hPages, bool _lockDim = false);
	HUD(ofPoint _mapPos, int _mapWidth, int _mapHeight, int _realWidth, int _realHeight, bool _lockDim = true);
	HUD(ofPoint _mapPos);
	~HUD(void);


	void draw();
	void update();
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);

	void setColor(ofColor _mapColor, ofColor _locColor);
	void setMapSize(int _mapWidth, int _mapHeight);
	ofPoint getTranslation();

private:

	ofColor	mapColor;
	ofColor locColor;
	ofPoint mapPos;
	ofPoint locPos;
	ofPoint nextLocPos;
	
	float	mapWidth, mapHeight;
	float	realWidth, realHeight;
	float	hPages, wPages;

	bool	lockDim;

	float wRatio, hRatio;
	ofPoint initMapPos;
	int		initMapWidth, initMapHeight;
};

