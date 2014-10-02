#pragma once

#include "ofMain.h"
#include "ofxFingerModel.h"
#include "ofxThumbModel.h"
#include "ofxFingerParameters.h"

#include "ofxUtilityTimer.h"
#include "FixedParameters.h"

//namespace ofxHandTracker {

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
		void			drawFboProjection(ofPoint _position, ofPoint _palmCenter = ofPoint(IMG_DIM/2, IMG_DIM/2, 0), int _kernelSize = FIXED_KERNEL);
		ofImage			getProjection(ofPoint _palmCenter = ofPoint(IMG_DIM/2, IMG_DIM/2, 0), int _kernelSize = FIXED_KERNEL);
		void			drawFingerProjection(ofxFingerModel f); //helper only

		void			restoreFrom(ofxFingerParameters _localParams, bool _includeAngleX = false);

		void			setScale(float _factor);
		void			setScale(ofPoint _scale);
		ofPoint&		getScaleRef();
		ofPoint			getScale();

		void			setOrigin(ofPoint _origin);
		ofPoint&		getOriginRef();
		ofPoint			getOrigin();

		void			setRotation(ofQuaternion _rotation);
		ofQuaternion&	getRotationRef();
		ofQuaternion	getRotation();

		ofxFingerModel*	getFingerRef(int _index) { 
			if (0 <= _index && _index < NUM_FINGERS) 
				return f[_index];
		}

		ofxThumbModel* getThumbRef() {
			return (ofxThumbModel*)f[0];
		}

		ofxFingerParameters	saveFingerParameters();
		//ofxPalmParameters savePalmParameters();

		// _mask -> masks fingers by bits (1- enabled, 0 - disabled)
		void			interpolate(ofxFingerParameters _to, short _mask = 31); // interpolates hand params, _mask: 1 - 31 (enables fingers bitwise)
		void			open(float _factor = 1.0, short _mask = 31); // opens hand, _factor: 0 - 1, _mask: 1 - 31 (enables fingers bitwise)
		void			close(float _factor = 1.0, short _mask = 31); // closes hand, _factor: 0 - 1, _mask: 1 - 31 (enables fingers bitwise)


	private:
		ofxFingerModel*			f[NUM_FINGERS]; 

		//ofPoint		  rotation;
		// TODO: setters/getters (also fix references in other files)
		ofPoint			origin;
		ofPoint			scaling; // TODO: maybe use 2 scales (display, processing)

		//current state of the rotation  
		ofQuaternion	curRot;  


		//a place to store the mouse position so we can measure incremental change  
		ofVec2f			lastMouse;
		//slows down the rotation 1 = 1 degree per pixel
		float			dampen;

		// interpolation methods & variables
		//void			interpolate(ofxFingerParameters _from, ofxFingerParameters _to);
		ofxFingerParameters desiredParams;

		//ofxUtilityTimer	interpolationTimer; // not used
		//bool			isInterpolating;// not used

		// mesh & fbo & shaders (experimental)
		//off screen drawing fbo - for better model projection generation
		ofFbo			meshFbo;
		ofFbo			dilateFbo;

		ofMesh			modelMesh; // used for drawing finger segment as lines
		ofMesh			palmMesh;  // used for drawing palm region as triangle strip
		//ofVbo			modelVbo; // not used right now

		ofShader		dilateShader;

		// img & pixels for creating proj image
		ofPixels		projPix;
		ofImage			projImg;

		// helper for mesh coloring
		ofFloatColor	getPointColor(ofPoint p);
};

//}