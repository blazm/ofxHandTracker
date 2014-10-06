//
//  Presentation.cpp
//  BSystem?
//
//  Created by Blaz
//
//

#include "Presentation.h"

Presentation::Presentation() {
    setup();
}

Presentation::Presentation(string _assetsFilename, string _menusFilename, string _vizualType) {
    setup();
}
 
void Presentation::setup() {
    images.clear();
        
    int sumWidth = 0;
    int maxHeight = 0;
    int maxWidth = 0;

	ofDirectory dir = ofDirectory("pptx/");  
	dir.listDir(); // populates filelist
	//vector<ofFile> files = dir.getFiles();

    for (int i=0; i<dir.numFiles(); i++) {
		ofImage currentImage;

		currentImage.loadImage(dir.getPath(i));
        images.push_back(currentImage);
              
        if (maxHeight < currentImage.height) { maxHeight = currentImage.height; }            
        if (maxWidth < currentImage.width) { maxWidth = currentImage.width; }
    }
    
    sumWidth = maxWidth * images.size();
        
    galleryArea.allocate(maxWidth, maxHeight);
    galleryImages.allocate(sumWidth, maxHeight);
    imageIndex = 0;
        
    imagePos = ofPoint(0, 0);
    imageNextPos = imagePos;
    isSliding = false;
        
    galleryImages.begin();
    ofClear(0,0,0,0);
    for (int i=0; i<images.size(); i++) {
        images[i].draw((maxWidth * i) + maxWidth/2 - images[i].width/2, maxHeight/2 - images[i].height/2, images[i].width, images[i].height);
    }
    galleryImages.end();
    
	// TODO: include back img
    //backImg.loadImage(assetsLoader.getImage("background").URL);

	ofRegisterMouseEvents(this);

	timerTotal.start(TIMER_TOTAL_DURATION); // 10 mins
	timerSlide.start(TIMER_SLIDE_DURATION);  // 1 min
}

Presentation::~Presentation() {
    //cleanup if needed
}

void Presentation::update()
{
	bool timerTotalZero = !timerTotal.update();
	bool timerSlideZero = !timerSlide.update();

	//float distRatio = (imagePos.distance(imageNextPos))/imagePos.squareDistance(imageNextPos) * 10;

	imagePos = imagePos + (imageNextPos - imagePos)*0.1;
	if (imagePos.distance(imageNextPos) < galleryArea.getWidth()/10) {
       // imagePos = imageNextPos; // if this commented enable earlier sliding
        isSliding = false;
    }
}


void Presentation::draw()
{
    //backImg.draw(ofPoint(0,0,0), ofGetWidth(), ofGetHeight());
	
    galleryArea.begin();
    ofClear(0,0,0,0);
    galleryImages.draw(imagePos.x, 0);
    galleryArea.end();
    
    ofPushMatrix(); // scale accordingly to current w/h and full hd w/h -> this fixes problems with unscaled text not fitting into rectangles, overdrawing...
    ofScale(wRatio, hRatio);
    float gAreaY = (FULL_HD_H * 0.48) - galleryArea.getHeight()/2;
	float gAreaX = FULL_HD_W/2 - galleryArea.getWidth()/2;
    galleryArea.draw(gAreaX, gAreaY);

    ofNoFill();
	ofSetColor(0, 0, 0);
    ofRect(gAreaX, gAreaY, galleryArea.getWidth(), galleryArea.getHeight());
 
    ofSetColor(255, 255, 255);
	
    ofPushStyle();
    ofFill();
    for (int i=0; i<images.size(); i++) {
        if (i == imageIndex) { ofSetColor(64, 175, 64, 250); }
		else { ofSetColor(32, 32, 32, 150); }
        ofCircle(FULL_HD_W/2 - ((images.size()-1)*40)/2 + i*40, gAreaY + galleryArea.getHeight() + 10, 5);
    }

	timerTotal.draw(gAreaX, gAreaY, galleryArea.getWidth(), 5, ofColor::gray);
	timerSlide.draw(gAreaX, gAreaY + galleryArea.getHeight() - 2, galleryArea.getWidth(), 5, ofColor::gray);

    ofPopStyle();
    ofPopMatrix();
}

void Presentation::windowResized(int w, int h)
{
    wRatio = (ofGetWidth() / FULL_HD_W);
    hRatio = (ofGetHeight() / FULL_HD_H);
}

void Presentation::disable()
{
  /*//TODO: disable all buttons, etc.
    ofUnregisterMouseEvents(this);*/
}

void Presentation::enable()
{
  /*//TODO: enable all buttons, etc.
    ofRegisterMouseEvents(this);*/
}

void Presentation::slideToPage(int _step) {
	if (!isSliding) {
        imageIndex += _step;
        if(imageIndex < 0) {
            // TODO: here disable prev button
            imageIndex = 0;
        }
		else if (imageIndex > images.size()-1) {
            imageIndex = images.size()-1;
        }
        else {
            //imageNextPos = imagePos + ofPoint(imgData[imageIndex].width,0);
            imageNextPos = imagePos - ((_step/abs(_step)) * ofPoint(galleryArea.getWidth(),0)) - (imagePos - imageNextPos); //last brackets enable earlier sliding
            isSliding = true;

			timerSlide.stop();
			timerSlide.start(TIMER_SLIDE_DURATION);
        }
    }
}

//when mouse is pressed reset the timer
void Presentation::mousePressed(ofMouseEventArgs& args)
{
	if(args.button == 0) slideToPage(1);
	else if(args.button == 2) slideToPage(-1);
	else { reset(); }
}

void Presentation::reset() {
	timerTotal.stop();
	timerTotal.start(TIMER_TOTAL_DURATION);

	timerSlide.stop();
	timerSlide.start(TIMER_SLIDE_DURATION);

	imageIndex = 0;
	imageNextPos = ofPoint(0,0);
}

bool Presentation::hasFinished() {
	return imageIndex == images.size()-1;
}