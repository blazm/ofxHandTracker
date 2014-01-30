#include "HUD.h"


HUD::HUD(void)
{
	mapWidth = ofGetWidth()/2;
	mapHeight = ofGetHeight()/2;

	initMapWidth = mapWidth;
	initMapHeight = mapHeight;

	realWidth = ofGetWidth() * 3;
	realHeight = ofGetHeight() * 3;

	mapPos = ofPoint(10, 10, 0);
	initMapPos = mapPos;
	locPos = mapPos;

	mapColor = ofColor(64, 128, 96, 128);
	locColor = ofColor(255, 64, 64);

	lockDim = false;
}

HUD::HUD(ofPoint _mapPos)
{
	mapWidth = ofGetWidth()/2;
	mapHeight = ofGetHeight()/2;
	
	initMapWidth = mapWidth;
	initMapHeight = mapHeight;

	realWidth = ofGetWidth() * 3;
	realHeight = ofGetHeight() * 3;

	mapPos = _mapPos;
	initMapPos = mapPos;
	locPos = mapPos;

	mapColor = ofColor(64, 128, 96, 128);
	locColor = ofColor(255, 64, 64);

	lockDim = false;
}

HUD::HUD(ofPoint _mapPos, ofColor _mapColor, int _mapWidth, int _mapHeight)
{
	mapColor = _mapColor;
	locColor = ofColor(255, 176, 32);

	mapWidth = _mapWidth;
	mapHeight = _mapHeight;
	
	initMapWidth = mapWidth;
	initMapHeight = mapHeight;

	realWidth = ofGetWidth() * 3;
	realHeight = ofGetHeight() * 3;

	mapPos = _mapPos;
	initMapPos = mapPos;
	locPos = _mapPos;

	lockDim = false;
}

HUD::HUD(ofPoint _mapPos, int _mapWidth, int _mapHeight, float _wPages, float _hPages, bool _lockDim) {
	// init size of map
	mapWidth = _mapWidth;
	mapHeight = _mapHeight;
	
	initMapWidth = mapWidth;
	initMapHeight = mapHeight;

	// num of pages from params
	hPages = _hPages;
	wPages = _wPages;

	// actual area available to user
	realWidth = ofGetWidth() * wPages;
	realHeight = ofGetHeight() * hPages;

	mapPos = _mapPos;
	initMapPos = mapPos;
	locPos = _mapPos;

	lockDim = _lockDim;
}

// TODO: update dimensions when screen resized (based on realW/H or pagesW/H -> depends how we need hud to be resized)
HUD::HUD(ofPoint _mapPos, int _mapWidth, int _mapHeight, int _realWidth, int _realHeight, bool _lockDim) {
	mapWidth = _mapWidth;
	mapHeight = _mapHeight;

	initMapWidth = mapWidth;
	initMapHeight = mapHeight;

	realWidth = (float)(_realWidth);
	realHeight = (float)(_realHeight);

	wPages = (float)(realWidth)/(float)(ofGetWidth());
	hPages = (float)(realHeight)/(float)(ofGetHeight());

	// fix when HUD is created with greater resolution than existing screen resolution 
	// (prevents shaking which happens beacuse of autocorrecting & border checking)
	if(wPages < 1)
		realWidth = ofGetWidth();
	if(hPages < 1)
		realHeight = ofGetHeight();
	
	//realWidth = ofGetWidth() * hPages;
	//realHeight = ofGetHeight() * wPages;

	mapPos = _mapPos;
	initMapPos = mapPos;
	locPos = _mapPos;

	lockDim = _lockDim;

//	wRatio = (ofGetWidth()/1920.0);
//    hRatio = (ofGetHeight()/1080.0);
}

void HUD::update() {

	//cout << "MAP WIDTH: " << mapWidth << " HEIGHT: " << mapHeight << endl;

	//float locW = (ofGetWidth()*mapWidth)/realWidth;
	//float locH = (ofGetHeight()*mapHeight)/realHeight;

	// transform from window dimensions to dimensions of hud map
	float locW = (float)(ofGetWidth()) * (mapWidth/realWidth);
	float locH = (float)(ofGetHeight()) * (mapHeight/realHeight);

	if((locPos.x - locW/2) < mapPos.x)
		locPos.x = mapPos.x + locW/2;
	else if((locPos.x + locW/2) > (mapWidth + mapPos.x))
		locPos.x = (mapWidth + mapPos.x - locW/2);

	if((locPos.y - locH/2) < mapPos.y)
		locPos.y = mapPos.y + locH/2;
	else if((locPos.y + locH/2) > (mapHeight + mapPos.y))
		locPos.y = (mapHeight + mapPos.y - locH/2);

	
	//locPos = prevLocPos + ((locPos - prevLocPos)*0.1f);
	nextLocPos = nextLocPos + ((locPos - nextLocPos)*0.3f);// ((locPos - prevLocPos)*0.1f);

	//TODO: maybe implement option of automatic sliding over whole scene
	//locPos.x += (mapWidth/25); //mapPos.x + locW/2;//
}

void HUD::draw() {
	ofEnableAlphaBlending();
	ofPushMatrix();
	ofSetColor(mapColor);
	ofRect(mapPos, mapWidth, mapHeight);
	ofSetColor(locColor);
	ofNoFill();

	// transform from window dimensions to dimensions of hud map
	float locW = (float)(ofGetWidth()) * (mapWidth/realWidth);
	float locH = (float)(ofGetHeight()) * (mapHeight/realHeight);

	ofRect(nextLocPos.x - locW/2, nextLocPos.y - locH/2, locW, locH); //- ofPoint((mapWidth/10)/2, (mapHeight/10)/2, 0)
	ofFill();
	ofPopMatrix();
	ofDisableAlphaBlending();
	ofSetColor(ofColor::white);
}

//--------------------------------------------------------------
void HUD::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void HUD::mouseDragged(int x, int y, int button){
	if(x >= mapPos.x && x < (mapPos.x + mapWidth) && y >= mapPos.y && y < (mapPos.y + mapHeight)) {
		locPos.x = x;
		locPos.y = y;
	}
}

//--------------------------------------------------------------
void HUD::mousePressed(int x, int y, int button){
	if(x >= mapPos.x && x < (mapPos.x + mapWidth) && y >= mapPos.y && y < (mapPos.y + mapHeight)) {
		locPos.x = x;
		locPos.y = y;
	}
}

//--------------------------------------------------------------
void HUD::mouseReleased(int x, int y, int button){

}

ofPoint HUD::getTranslation() {
	// transform from window dimensions to dimensions of hud map
	float locW = (float)(ofGetWidth()) * (mapWidth/realWidth);
	float locH = (float)(ofGetHeight()) * (mapHeight/realHeight);
	ofPoint tXY = (nextLocPos - ofPoint(locW/2, locH/2, 0)) - mapPos;
	
	tXY.x *= (float)(realWidth/mapWidth);
	tXY.y *= (float)(realHeight/mapHeight);

	return tXY;
}

//--------------------------------------------------------------
void HUD::windowResized(int w, int h){
	// calculate new ratios for scaling
	wRatio = (w/1920.0);
    hRatio = (h/1080.0);

	// calculate new position of map (based on ratio)
	mapPos.x = initMapPos.x * wRatio;
	mapPos.y = initMapPos.y * hRatio;
	// calculate scaled map width & height (based on ratio)
	mapWidth = initMapWidth * wRatio;
	mapHeight = initMapHeight * hRatio;

	if(!lockDim) {
		// real width & height is the actual area
		realWidth = w * wPages;
		realHeight = h * hPages;
	}
	else {

		//if(realWidth > w)
		//	realWidth = w;

		wPages = (float)(realWidth)/(float)(w);
		hPages = (float)(realHeight)/(float)(h);

		if(wPages < 1)
			realWidth = w;
		if(hPages < 1)
			realHeight = h;
		/*realWidth = w;
		realHeight = h;*/
	}
}

void HUD::setColor(ofColor _mapColor, ofColor _locColor) {
	mapColor = _mapColor;
	locColor = _locColor;
}

void HUD::setMapSize(int _mapWidth, int _mapHeight) {
	mapWidth = _mapWidth;
	mapHeight = _mapHeight;
}

HUD::~HUD(void)
{
}
