#pragma once

#include "ofMain.h"
#include "ofxFingerModel.h"
#include "ofxThumbModel.h"
#include "ofxFingerParameters.h"

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
		void            getFingerWorldCoord(int index, vector<ofPoint> &_joints);
		void			getFillWorldCoord(vector<ofPoint> &_vertices);
		ofPoint			getWorldCoord(ofPoint localPoint, ofPoint localOrigin);

		void			drawProjection(); 
		void			drawFboProjection(ofPoint _position, ofPoint _palmCenter = ofPoint(IMG_DIM/2, IMG_DIM/2, 0), int _kernelSize = FIXED_KERNEL, bool _draw = true);
		void			getProjection(ofImage &_target, ofPoint _palmCenter = ofPoint(IMG_DIM/2, IMG_DIM/2, 0), int _kernelSize = FIXED_KERNEL);
		void			drawFingerProjection(ofxFingerModel f); //helper only

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

		ofxFingerModel*	getFingerRef(int _index) { if (0 <= _index && _index < NUM_FINGERS) return f[_index]; }
		ofxThumbModel* getThumbRef() { return (ofxThumbModel*)f[0];	}

		void restoreFrom(ofxFingerParameters _localParams, bool _includeAngleX = false);
		ofxFingerParameters	saveFingerParameters();
		//ofxPalmParameters savePalmParameters();

		// _mask -> masks fingers by bits (1- enabled, 0 - disabled)
		void			interpolate(ofxFingerParameters _to, short _mask = 31); // interpolates hand params, _mask: 1 - 31 (enables fingers bitwise)
		
		void			open(float _factor = 1.0, short _mask = 31); // opens hand, _factor: 0 - 1, _mask: 1 - 31 (enables fingers bitwise)
		void			close(float _factor = 1.0, short _mask = 31); // closes hand, _factor: 0 - 1, _mask: 1 - 31 (enables fingers bitwise)

		void			spread(float _factor = 1.0, short _mask = 31); // spreads hand fingers, _factor: 0 - 1, _mask: 1 - 31 (enables fingers bitwise)
		void			narrow(float _factor = 1.0, short _mask = 31); // narrows hand fingers, _factor: 0 - 1, _mask: 1 - 31 (enables fingers bitwise)

	private:
		void			interpolateParam(float &_desired, float &_prev, float _weight = 0.1f); // internal helper

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
		
		// mesh & fbo & shaders (experimental)
		//off screen drawing fbo - for better model projection generation
		ofFbo			meshFbo;
		ofFbo			dilateFbo;

		ofMesh			modelMesh; // used for drawing finger segment as lines
		ofMesh			palmMesh;  // used for drawing palm region as triangle strip
		//ofVbo			modelVbo; // not used right now

		ofShader		dilateShader; // for generating dilated model image (e.q. model projection)

		// img & pixels for creating proj image
		ofPixels		projPix;
		ofImage			projImg;

		// helper for mesh coloring
		ofFloatColor	getPointColor(ofPoint p);
};

//}