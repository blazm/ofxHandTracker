#pragma once

//#include "ofMain.h"
#include "ofxHandModel.h"
#include "ofxOpenNI.h"
#include "ofxOpenCv.h"
#include "ofxImageMatcher.h"

#define	TINY_IMG_DIM		150
#define FTIP_HIST_SIZE		3	// history array size

#define	HAND_GRABBED			"hand_grabbed"
#define HAND_RELEASED			"hand_released"
#define HAND_PICKED				"hand_picked"

#define MIN_HAND_DEPTH		 500.0
#define MAX_HAND_DEPTH	    2000.0

// tracker for each hand
class ofxHandTracker
{
	public:
		//ofxHandTracker();
		ofxHandTracker(ofxUserGenerator *_userGen, ofxHandGenerator *_handGen, ofxDepthGenerator *_depthGen, int _hIndex);
		//~ofxHandTracker(void);

		void update();
		void draw();

	// getters 
		ofxHandModel*			getHandModel();
		vector<ofPoint> getActiveFingerTips();
		int				getNumFingerTips();
		
		//ofEvent<string> handEvent;	// will notify about grabbing, releasing, etc.

	private:
		//OpenNI Generator references 
		ofxUserGenerator	*userGen;
		ofxHandGenerator	*handGen;
		ofxDepthGenerator	*depthGen; // not needed, remove if useless in future

		// depth image fbo
		ofFbo				depthFbo;

		//hand model
		ofxHandModel				h;
		int					hIndex;

		//Images
		ofImage				blankImg;  //image for clearing (overriding) real image pixels
		//	-> hand image from point cloud
		ofImage				realImg;
		ofxCvGrayscaleImage realImgCV;
		ofxCvGrayscaleImage realImgCV_previous; // not used right now, stores info about hand pose changes in prev frame
		//ofImage				colorImg; // maybe used to find contours on camera img
		// -> hand image from model rasterization
		ofImage				modelImg;
		ofxCvGrayscaleImage modelImgCV;
		// -> hand contour image
		ofxCvContourFinder	realImgCvContour;
		// -> difference image (between real and model hand)
		ofxCvGrayscaleImage diffImg;

		ofxCvGrayscaleImage				tinyModelImg;
		ofxCvGrayscaleImage				tinyHandImg;
		ofxCvGrayscaleImage				tinyDiffImg;

		//Points from Point cloud
		vector<ofPoint>		handPoints;
		vector<ofPoint>		handRootPoints;
		vector<ofPoint>		handEdgePoints; 
		vector<ofPoint>		handPalmCandidates;
		vector<ofPoint>		imgPalmCandidates;

		//Points from CV contour image
		vector<ofPoint>		blobPoints;

		//BBox 
		int maxX, minX, maxY, minY, maxZ, minZ;

		//Orientation & Centroids & PalmCenter
		ofPoint handCentroid;
		ofPoint handRootCentroid;
		ofPoint palmCenter; // calculated on image, so it has not same coord system as previous vars
		float	palmRadius;

		// current active hand position
		ofPoint	activeHandPos;

		// angle for rotation left, right
		float	rollAngle;

		// fingerTips counter
		int		fingerTipsCounter;

		vector<ofPoint>	activeFingerTips; // to get active fingertips outside of tracker for any use (for example drawing)

		int		fTipHistory[FTIP_HIST_SIZE];
		int		fTipLastInd;

		bool	recheckFingers;

		//TODO: based on fTipHistory we can trigger events for grabbing, painting etc (for demo application & show usability)
		//ofEvent<string>		grabEvent;
		//ofEvent<string>		paintEvent; // not sure how will be used but ok
	
		//TODO: we can check radius changes over time -> we will need maxRad & minRad vars, 
		//		then we can check if radius very small, hand is probably facing kinect sideways

		// contour analyzer method
		void				analyzeContours();

		//helper methods // should be private
		float				getHandDepth(float _rawZ, bool _normalized = true, float _min = MIN_HAND_DEPTH, float _max = MAX_HAND_DEPTH);
		float				angleOfVectors(ofPoint _downVector, ofPoint _rotVector, bool _absolute = true);
		float				distFactor(float zDist); 
		void				generateModelProjection(bool _useLegacy = false);
		float				getImageMatching(ofxCvGrayscaleImage &realImage, 
										     ofxCvGrayscaleImage &modelImage,  
											 ofxCvGrayscaleImage &differenceImage);
		float				getImageMatching(ofImage &realImage,   
											 ofImage &diffImage);
		float				getImageMatching(ofxCvGrayscaleImage &differenceImage);
		void				fetchHandPointCloud(ofPoint _handTrackedPos);
		ofPoint				getPalmCenter();
		ofPoint				getCentroid(vector<ofPoint> &points);
		void				drawLine(ofImage *img, int x0, int y0, int z0, int x1, int y1, int z1);

		// optimum searching methods (currently very basic)
		void				findParamsOptimum(int _params[], int _size);
		void				findParamsOptimum(ofxFingerParameters _params[], int _size);
		void				findParamsOptimum(int _paramsZ[], int _sizeZ, ofxFingerParameters _paramsX[], int _sizeX);

		// here is place for advanced shader & fbo objects
		// which will help us realize paralel image comparing on GPU
		ofxImageMatcher		shaderMatcher;
};

