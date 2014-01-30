#pragma once

#include "ofMain.h"
#include "ofxFingerModel.h"
#include "ofxThumbModel.h"
#include "ofxHandParameters.h"

#include "ofxUtilityTimer.h"

#define IMG_DIM			150.0
#define	NUM_FINGERS	5

class ofxHandModel
{
	public:
		ofxHandModel(void);
		~ofxHandModel(void);

		void update();
		void draw();
		void drawMesh(); // testing of proper vertices & colors drawing
		void keyPressed(int key);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);

		ofPoint			getIndexFingerWorldCoord();
		vector<ofPoint> getFingerWorldCoord(int index);
		vector<ofPoint> getFillWorldCoord();
		ofPoint			getWorldCoord(ofPoint localPoint, ofPoint translateOrigin);

		void			drawProjection(); 
		ofImage			getProjection(ofPoint _palmCenter = ofPoint(IMG_DIM/2, IMG_DIM/2, 0), int _kernelSize = 4);
		void			drawFingerProjection(ofxFingerModel f); //helper only

		void			restoreFrom(ofxFingerParameters _localParams, bool _includeAngleX = false);

		ofxFingerParameters	saveFingerParameters();

		//ofxPalmParameters savePalmParameters();

		ofPoint			origin;
		ofPoint			scaling;

		/*
		// fingers
		ofxFingerModel			f1;
		ofxFingerModel			f2;
		ofxFingerModel			f3;
		ofxFingerModel			f4;

		// TODO: add thumb
		Thumb			t; // will be f0
		*/
		ofxFingerModel*			f[NUM_FINGERS]; //

		//ofPoint		  rotation;
		// Quaternion rotation from example
		//current state of the rotation  
		ofQuaternion	curRot;  
		//a place to store the mouse position so we can measure incremental change  
		ofVec2f			lastMouse;
		//slows down the rotation 1 = 1 degree per pixel
		float			dampen;

		// interpolation methods & variables
		void			interpolate(ofxFingerParameters _from, ofxFingerParameters _to);
		void			interpolate(ofxFingerParameters _to);
		ofxFingerParameters desiredParams;

		ofxUtilityTimer	interpolationTimer;
		bool			isInterpolating;



		// mesh & fbo & shaders (experimental)
		//off screen drawing fbo - for better model projection generation
		ofFbo			meshFbo;
		ofFbo			dilateFbo;

		ofMesh			modelMesh; // used for drawing finger segment as lines
		ofMesh			palmMesh;  // used for drawing palm region as triangle strip
		ofVbo			modelVbo; // not used right now

		ofShader		dilateShader;

		// img & pixels for creating proj image
		ofPixels		projPix;
		ofImage			projImg;

		// helper for mesh coloring
		ofFloatColor	getPointColor(ofPoint p);
};

