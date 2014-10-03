#pragma once

//#include "ofMain.h"
#include "ofxHandModel.h"
#include "ofxOpenNI.h"
#include "ofxOpenCv.h"
#include "ofxImageMatcher.h"

#include "FixedParameters.h"

/* // namespace usage example
namespace ofxHT {
	class Tracker;
}

class ofxHT::Tracker {
};*/

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
		ofxHandModel&	getHandModelRef();
		vector<ofPoint> getActiveFingerTips();
		int				getNumFingerTips();
		
		//ofEvent<string> handEvent;	// will notify about grabbing, releasing, etc.

		void kMeansClustering(vector<ofPoint> &_cloud, int _iterations, int _numClusters); // super simple k means clustering -> problem is seed selection
		float getCircularity(const vector<ofPoint> &_edgePoints, const vector<ofPoint> &_areaPoints); // returns circularity measurement from edge points and area points
		
		// void rotatePCL(vector<ofPoint> &_cloud, ofPoint _origin, float _angle); // TODO: useful to obtain OBB from pcl, and aspect ratio from it
		void generateRegionSkeleton(ofxCvGrayscaleImage &_img, ofxCvGrayscaleImage &_result); // TODO: test if it works? then do your own implementation?
		void generateTinyImage(ofxCvGrayscaleImage &_img, ofxCvGrayscaleImage &_result, int _size);
		void drawSideProjections(ofxCvGrayscaleImage &_tiny, ofPoint _position, int _size=10);
		void findLines(ofxCvGrayscaleImage &_img, vector<ofVec4f> &_lines);
		void filterLines(vector<ofVec4f> &_lines, ofPoint _position); // primitive O(n^2)
	private:
		// some filtering helpers
		void filterMedian(ofxCvGrayscaleImage *i, int kernelSize);
		void filterMedian(ofImage *i, int kernelSize);
		void filterGauss(ofxCvGrayscaleImage *i, int kernelSize);

		static int			ccw(ofPoint p1, ofPoint p2, ofPoint p3); // returns counter clock wise orientation of three points
		
		struct ofPointComparator {
			ofPoint pivot; // TODO: need to set it up, before sorting

			bool operator() (ofPoint p1, ofPoint p2) {
				/*int res = ofxHandTracker::ccw(p1, pivot, p2);
				if (res == 0) // if same angle (collinear points), arrange by x coord (left first)
					res = p1.x - p2.x;
				return res;*/
				return (p1.x < p2.x);
			}
		};
		
		void				sortEdgePoints(vector<ofPoint> &_edgePoints);
		void				simplifyDP_openCV ( const vector<ofPoint>& contourIn, vector<ofPoint>& contourOut, float tolerance );
		void				simplifyByRadDist( const vector<ofPoint>& _contourIn, vector<ofPoint>& _contourOut, float _threshDist);
		ofPoint&			getPivotPoint(vector<ofPoint> &_edgePoints);
		
		//OpenNI Generator references 
		ofxUserGenerator	*userGen;
		ofxHandGenerator	*handGen;
		ofxDepthGenerator	*depthGen; // not needed, remove if useless in future

		// depth image fbo
		//ofFbo				depthFbo;

		//hand model
		ofxHandModel		h;
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

		ofxCvGrayscaleImage handSkeleton;
		ofxCvGrayscaleImage modelSkeleton;
		//ofImage				handFrontImg, handSideImg, handTopImg;

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
		//ofPoint min, max;

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

		//TODO: based on fTipHistory we can trigger events for grabbing, painting etc (for demo application & show usability)
		//ofEvent<string>		grabEvent;
		//ofEvent<string>		paintEvent; // not sure how will be used but ok
	
		//TODO: we can check radius changes over time -> we will need maxRad & minRad vars, 
		//		then we can check if radius very small, hand is probably facing kinect sideways

		// contour analysis methods
		void				analyzeContours(vector<ofPoint>	&_activeFingerTips); // populates passed vector with detected fingertips
		void				drawContours(ofPoint _position = ofPoint::zero());


		//helper methods // some of them should be private
		bool				getTrackedPosition(ofPoint &_trackedPosition);
		void				getCloudBBox(ofPoint &_min, ofPoint &_max, vector<ofPoint> &_cloud); // gets bounding box (min,max points) from cloud

		float				getHandDepth(float _rawZ, bool _normalized = true, float _min = MIN_HAND_DEPTH, float _max = MAX_HAND_DEPTH);
		float				angleOfVectors(ofPoint _downVector, ofPoint _rotVector, bool _absolute = true);
		float				distFactor(float zDist); 
		void				generateModelProjection();
		float				getImageMatching(ofxCvGrayscaleImage &realImage, 
										     ofxCvGrayscaleImage &modelImage,  
											 ofxCvGrayscaleImage &differenceImage);
		float				getImageMatchingV2(ofxCvGrayscaleImage &realImage, // using cv methods, probably faster
											   ofxCvGrayscaleImage &modelImage,  
											   ofxCvGrayscaleImage &differenceImage);
		float				getImageMatching(ofImage &realImage,   
											 ofImage &diffImage);
		float				getImageMatching(ofxCvGrayscaleImage &differenceImage);
		void				fetchHandPointCloud(ofPoint _handTrackedPos);
		void				getPalmCenterAndRadius(ofPoint &_palmCenter, float &_palmRadius);
		ofPoint				getCentroid(vector<ofPoint> &points);
		void				drawPointCloud(ofPoint _position, vector<ofPoint> &_cloud, ofColor _color);
		void				drawRegionSkeletons(ofxCvGrayscaleImage &_img,  ofPoint _position);
		//void				drawLine(ofImage *img, int x0, int y0, int z0, int x1, int y1, int z1);

		bool				isFistFormed(int _fingerTipsCounter); // not used helper

		// optimum searching methods (currently very basic)
		void				setParamsFromFingerTips(int _fingerTipsCounter);
		void				findParamsOptimum(int _params[], int _size);
		void				findParamsOptimum(ofxFingerParameters _params[], int _size);
		void				findParamsOptimum(int _paramsZ[], int _sizeZ, ofxFingerParameters _paramsX[], int _sizeX);

		
		void				findParamsOptimum(ofxHandModel &_h, 
											  ofxCvGrayscaleImage &_hand, 
											  ofxCvGrayscaleImage &_model, 
											  ofxCvGrayscaleImage &_diff, 
											  short _mask); 
		// here is place for advanced shader & fbo objects
		// which will help us realize paralel image comparing on GPU
		ofxImageMatcher		shaderMatcher;
};

