//
//  Presentation.h
//  BSystem
//
//  Created by Blaz on 9/17/13.
//
//

#pragma once

#include <iostream>
#include "ofMain.h"
#include "ofxUtilityTimer.h"

//#define		FULL_HD_W   1920.0
//#define     FULL_HD_H	1080.0

#define		FULL_HD_W   1000.0
#define     FULL_HD_H	800.0

#define TIMER_SLIDE_DURATION	60000
#define TIMER_TOTAL_DURATION	600000

class Presentation
{

 public:
    Presentation();
    Presentation(string _assetsFilename, string _menusFilename, string _vizualType);
    ~Presentation();
    
    void setup();
    void update();
    void draw();
    
    void        windowResized(int w, int h);

    void        enable();
    void        disable();

    // mouse events (for timeout reset)
    void        mousePressed(ofMouseEventArgs& args);
    void        mouseDragged(ofMouseEventArgs& args){};
    void        mouseReleased(ofMouseEventArgs& args){};
    void        mouseMoved(ofMouseEventArgs& args){};
    
	void		slideToPage(int _step);
	void		reset(); 
	bool		hasFinished();

 private:
	// presentation specific methods
    

    // presentation specific components
    vector<ofImage> images;
    int             imageIndex;
    ofPoint         imagePos;
    ofPoint         imageNextPos;
    ofFbo           galleryArea;
    ofFbo           galleryImages;

	// scale ratios
	float			wRatio, hRatio;

    // determines if presentation is in process of animating from
    // one image to another (when clicked by next or prev button)
    bool            isSliding; 

	// informative timers
	ofxUtilityTimer			timerSlide; // 1 minute for each slide
	ofxUtilityTimer			timerTotal; // 10 minutes total for presentation

};