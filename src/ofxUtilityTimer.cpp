#include "ofxUtilityTimer.h"


ofxUtilityTimer::ofxUtilityTimer(void)
{
    /*isCounting = false;
    minutes=0; seconds=0; miliseconds=0;
    durationTime=0; step=0;*/
	stop();
}

bool ofxUtilityTimer::update()
{
    elapsedMillis = ofGetElapsedTimeMillis() - startTime;

    if(!isZero())
    {
        step = durationTime - ofGetElapsedTimeMillis();
                
		//miliseconds = (int)step % 1000;
		//seconds = (int) (miliseconds / 1000) % 60 ;
		//minutes = (int) ((int)(miliseconds / (1000*60)) % 60);
        
        /*miliseconds=(float)((int)step%1000)*0.1;
        minutes=step/1000;
        seconds=((int)minutes)%60;
        minutes/=60;*/
                
       // if(minutes<0) minutes=0;
       // if(miliseconds<0) miliseconds=0;

        return true;
    }
	stop();
	//send event here
    return false;
}

bool ofxUtilityTimer::isZero() {
	return !(durationTime > 0 && step > 0 && isCounting);
}

void ofxUtilityTimer::start(float millis)
{
	isCounting = true;

    durationMillis = millis;
    durationTime = ofGetElapsedTimeMillis() + durationMillis;
      
    step = durationMillis;
    startTime = ofGetElapsedTimeMillis();
}

void ofxUtilityTimer::stop()
{
    isCounting = false;
        
    minutes=0; seconds=0; miliseconds=0;
    durationTime=0; step=0;
	durationMillis = 0;
}

void ofxUtilityTimer::draw(float _x, float _y) {
	// draw counting up
	//ofDrawBitmapString(millisToString(getElapsed()), _x, _y, 0);
	// draw countdown
	ofDrawBitmapString(millisToString(getDuration() - getElapsed()), _x, _y, 0);
}

void ofxUtilityTimer::draw(float _x, float _y, float _w, float _h, ofColor _color) {
	// draw counting up
	//ofDrawBitmapString(millisToString(getElapsed()), _x, _y, 0);
	// draw countdown
	//ofDrawBitmapString(millisToString(getDuration() - getElapsed()), _x, _y, 0);

	ofPushStyle();
	ofSetColor(_color);
	ofRect(_x, _y, getPercent() * _w, _h);
	ofPopStyle();
}

string ofxUtilityTimer::millisToString(int millis) {
    int timerMin, timerSec, timerMil;
    timerMin = millis / 1000 / 60;
    timerSec = (millis / 1000) % 60;
    timerMil = (millis % 1000) / 10;

    string timeMessage = ofToString(timerMin) + ":" + (timerSec<10?"0":"") +
                         ofToString(timerSec) + ":" + (timerMil<10?"0":"") +
                         ofToString(timerMil);
    
    return timeMessage;
}

float ofxUtilityTimer::getDuration()
{
    return durationMillis;
}

//get starting timer delay
float ofxUtilityTimer::getStartDuration()
{
    return durationTime;
}

float ofxUtilityTimer::getPercent()
{
    return elapsedMillis / durationMillis;
}

float ofxUtilityTimer::getElapsed() {
	return elapsedMillis;
}

ofxUtilityTimer::~ofxUtilityTimer(void)
{
}
