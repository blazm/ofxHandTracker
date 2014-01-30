#pragma once

#include "ofMain.h"
#include <string>

class ofxUtilityTimer
{
	public:
        ofxUtilityTimer(void);
		~ofxUtilityTimer(void);
        
        bool                    update();
        void					draw(float _x, float _y);
		void					draw(float _x, float _y, float _w, float _h, ofColor _color);

        void                    start(float millis);
        void                    stop();
        
        float                   getDuration(); // difference between duration and startDuration getters is? idk
		float					getStartDuration();
        float                   getPercent();
		float					getElapsed();

        bool					isZero();

        bool                    isCounting;
                
       // int                     countdownSize;
        
    private:
		string					millisToString(int millis);

		//not used
       // char                    min[5], sec[5], mil[5];
        float                   minutes, seconds, miliseconds;
        float                   durationTime, step, startTime;

		float					durationMillis;
		float					elapsedMillis;

      //  string                  fontPath;
       // int                     growingSize;
      //  string                  previousSec;

		//TODO: add event
		ofEvent<float>			finished; // maybe find better descriptor of finished counting
};

