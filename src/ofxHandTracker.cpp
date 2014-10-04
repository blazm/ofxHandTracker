#include "ofxHandTracker.h"
#include "ofConstants.h"

/*ofxHandTracker::ofxHandTracker()
{
	userGen = NULL;
	handGen = NULL;
}*/

ofxHandTracker::ofxHandTracker(ofxUserGenerator *_userGen, ofxHandGenerator *_handGen, ofxDepthGenerator *_depthGen, int _hIndex)
{
	userGen = _userGen;
	handGen = _handGen;
	depthGen = _depthGen;

	hIndex = _hIndex;
	h.setScale(.3);
	//h.scaling = ofPoint(.3,.3,.35);

	// setup ofImages for generating / getting pixels
	realImg.allocate(IMG_DIM, IMG_DIM, OF_IMAGE_GRAYSCALE); 
	modelImg.allocate(IMG_DIM, IMG_DIM, OF_IMAGE_GRAYSCALE);  
	blankImg.allocate(IMG_DIM, IMG_DIM, OF_IMAGE_GRAYSCALE); 

	for (int i=0; i<IMG_DIM; i++) {    
		for (int j=0; j<IMG_DIM; j++) {    
			realImg.setColor(i, j, ofColor::black);
			blankImg.setColor(i, j, ofColor::black);
			modelImg.setColor(i, j, ofColor::black); 
		}    
	}

	// set up CV images
	realImgCV.allocate(IMG_DIM, IMG_DIM);
	realImgCV_previous.allocate(IMG_DIM, IMG_DIM);
	modelImgCV.allocate(IMG_DIM, IMG_DIM);
	diffImg.allocate(IMG_DIM,IMG_DIM);

	realImgCV.set(0);
	realImgCV_previous.set(0);
	modelImgCV.set(0);
	diffImg.set(0);

	palmCenter = ofPoint(IMG_DIM/2, IMG_DIM/2, 0);
	palmRadius = 0;
	rollAngle = 0;

	fingerTipsCounter = 0;

	// logging number of detected fingertips for few steps back
	fTipLastInd = 0;
	for (int i=0; i<FTIP_HIST_SIZE; i++) {
		fTipHistory[i] = 0;
	}
	/*
	ofFbo::Settings s = ofFbo::Settings();  
	s.width = IMG_DIM;  
	s.height = IMG_DIM;  
	s.internalformat = GL_LUMINANCE32F_ARB;
	depthFbo.allocate(s);*/
	//depthFbo.allocate(IMG_DIM, IMG_DIM);

	//tinyHandImg.allocate(TINY_IMG_DIM, TINY_IMG_DIM, OF_IMAGE_GRAYSCALE);
	//tinyModelImg.allocate(TINY_IMG_DIM, TINY_IMG_DIM, OF_IMAGE_GRAYSCALE);
	tinyHandImg.allocate(TINY_IMG_DIM, TINY_IMG_DIM);
	tinyModelImg.allocate(TINY_IMG_DIM, TINY_IMG_DIM);
	tinyDiffImg.allocate(TINY_IMG_DIM, TINY_IMG_DIM);


	shaderMatcher.setup(IMG_DIM, IMG_DIM);
}

void ofxHandTracker::fetchHandPointCloud(ofPoint _handTrackedPos) {
	int width = userGen->getWidth();
	int height = userGen->getHeight();

	int xx = _handTrackedPos.x;
	int yy = _handTrackedPos.y;
	int zz = _handTrackedPos.z;
	int raw = _handTrackedPos.z;
	ofPoint handTrackedPos = _handTrackedPos;

	ofPoint handImageTrackedPos = handTrackedPos;
	handImageTrackedPos.z = 0;

	float percentage = getHandDepth(raw, true);

	//depthGen->setDepthThreshold(0, 600, 800);
	//depthGen->update();

	// here's the part where we segment hand from depthGenerator (not used but should be available to other devs as feature)
	// bad thing here is that we dont get much depth information, so its just silhouette of the hand
	/*unsigned char *pix = depthGen->getDepthPixels(raw - 100, raw + 100, 500, 640, 640);
	ofImage depth;
	depth.setFromPixels(pix, 640, 480, ofImageType::OF_IMAGE_GRAYSCALE);

	depthFbo.begin();
		ofClear(ofColor::black);
		ofPushMatrix();
		ofScale(0.8*(1 + percentage), 0.8*(1 + percentage));
		ofTranslate(-handImageTrackedPos + ofPoint((IMG_DIM/2)*(1 - percentage/2), (IMG_DIM/2)*(1 - percentage/2), 0));
		//depthGen->draw();
		depth.draw(0,0);
		ofPopMatrix();
	depthFbo.end();
	*/
	// bounding square limits - TODO: parametrize
	int bbMinX = 160; 
	int bbMaxX = 160;
	int bbMinY = 160;
	int bbMaxY = 160;

	// adjust bb by distance factor
	if (zz >= MIN_HAND_DEPTH) {
		bbMinX -= distFactor(zz);
		bbMaxX -= distFactor(zz);
		bbMinY -= distFactor(zz);
		bbMaxY -= distFactor(zz);
	}
	
	// check if bounding square outside of kinect view
	if(xx < bbMinX) bbMinX = xx;
	if(yy < bbMinY) bbMinY = yy;

	if(yy > height-bbMaxY) bbMaxY = height - yy;
	if(xx > height-bbMaxX) bbMaxX = width - xx;
	
	ofDrawBitmapString(ofToString(ofPoint(xx, yy, zz)), 100, 100, 0);

	handPoints.clear();
	handRootPoints.clear();
	handEdgePoints.clear();
	handPalmCandidates.clear();

	int step = 2;
	minX = ofGetWidth(), maxX = 0, minY = ofGetHeight(), maxY = 0;
	maxZ = 0, minZ = 10000; //?

	int userID = 0; // all users

	for(int y = yy-bbMinY; y < yy+bbMaxY; y += step) {
		for(int x = xx-bbMinX; x < xx+bbMaxX-step; x += step) {

			ofPoint &nextPoint = userGen->getWorldCoordinateAt(x+step, y, userID);
			ofPoint &pos = userGen->getWorldCoordinateAt(x, y, userID);

			if(abs(pos.z - nextPoint.z) > 50) {
				if(nextPoint.z != 0 && nextPoint.distance(handTrackedPos) < 150 && nextPoint.distance(handTrackedPos) > 20)  //
					handEdgePoints.push_back(nextPoint);
				else if(pos.z != 0 && pos.distance(handTrackedPos) < 150 && pos.distance(handTrackedPos) > 20) // 
					handEdgePoints.push_back(pos);
			}
			
			if (pos.z == 0 || pos.distance(handTrackedPos) > (160 - distFactor(zz))) continue;	// gets rid of background -> still a bit weird if userID > 0...
			//|| abs(pos.z - zz) > 50

			//BBox
			if(pos.x < minX) minX = pos.x;
			else if(pos.x > maxX) maxX = pos.x;

			if(pos.y < minY) minY = pos.y;
			else if(pos.y > maxY) maxY = pos.y;

			if(pos.z < minZ) minZ = pos.z;
			else if(pos.y > maxZ) maxZ = pos.z;

			if (pos.distance(handTrackedPos) <= (160 - distFactor(zz))
			 && pos.distance(handTrackedPos) > (152 - distFactor(zz))) // filter most outer points
				handRootPoints.push_back(pos);
			/*else if(pos.distance(handTrackedPos) < 30 && pos.distance(handTrackedPos) > 0)
				handPalmCandidates.push_back(pos);*/
			//ofColor color = user_generator->getWorldColorAt(x,y, userID);

			//TODO: check for 2 hands tracking - separate hand points for each hand
			else
				handPoints.push_back(pos);
		}
	}
	
	handCentroid = getCentroid(handPoints);

//	ofPoint prevHandRootCentroid = handRootCentroid;
	handRootCentroid = getCentroid(handRootPoints);

	ofPoint maxDistCentroidPoint = ofPoint(handCentroid);
	float maxDistCentroid = 0;

	ofPoint maxDistTrackedPoint = ofPoint(handTrackedPos);
	float maxDistTracked = 0;

	ofPoint maxDistRootPoint = ofPoint(handRootCentroid);
	float maxDistRoot = 0;

	// calculate approx. oritentation by calcing max distance from centroid
	for(std::vector<ofPoint>::iterator it = handPoints.begin(); it != handPoints.end(); ++it) {
		/* std::cout << *it; ... */
		ofPoint &p = *it;
		if(p.distance(handCentroid) > maxDistCentroid) {
			maxDistCentroid = p.distance(handCentroid);
			maxDistCentroidPoint = ofPoint(p);
		}
		if(p.distance(handTrackedPos) > maxDistTracked) {
			maxDistTracked = p.distance(handCentroid);
			maxDistTrackedPoint = ofPoint(p);
		}
		if(p.distance(handRootCentroid) > maxDistRoot) {
			maxDistRoot = p.distance(handRootCentroid);
			maxDistRootPoint = ofPoint(p);
		}
	}

	// prevent jumping rootCentroid to 0,0,0, need better solution
	if(handRootCentroid.distance(ofPoint(0,0,0)) < 10)
		handRootCentroid = handCentroid;

	/*bool usingCentroid = false;
	
	ofPoint estimatedPalmCenter = palmCenter;
	if(estimatedPalmCenter.distance(ofPoint::zero()) < 10) {
		estimatedPalmCenter = handCentroid;
		usingCentroid = true;
	}*/

	int i = 0;
	for(std::vector<ofPoint>::iterator it = handPoints.begin(); it != handPoints.end(); ++it) {
		ofPoint &pos = *it;
		i++;
		//if(usingCentroid) {
			if (i%2 == 0 || pos.distance(handCentroid) > (maxDistCentroid * 0.60)) continue;
		/*}
		else
			if (pos.distance(estimatedPalmCenter) > 60) continue;
		*/
		handPalmCandidates.push_back(pos);
	}
}

void ofxHandTracker::kMeansClustering(vector<ofPoint> &_cloud, int _iterations, int _numClusters) {
	if (_cloud.size() < 2) { // handle situation when _cloud empty
		return;
	}

	//int numClusters = 5;
	ofPoint* means = new ofPoint[_numClusters];
	vector<ofPoint>* clusters = new vector<ofPoint>[_numClusters];

	// populate first means (e.g. seeds)
	for (int i=0; i<_numClusters; i++) {
		//int randIndex = rand() % (_cloud.size()-1); // if cloud size == 1 => we get exc: div by zero
		//means[i].set(_cloud.at(randIndex));

		means[i].set(activeHandPos + ofPoint(0, 0, (i - _numClusters/2)*20));
	}

	while (_iterations) {
	
		// for each point in _cloud
		for(std::vector<ofPoint>::iterator it = _cloud.begin() ; it != _cloud.end(); ++it) {
			//		find seed with min distance to point (for each seed)
			ofPoint &p = *it;
			int minI = 0;
			float minDist = means[minI].distance(p);
			for(int i=1; i<_numClusters; i++) {
				//float dist = means[i].distance(p); //
				//float dist = abs(means[i].x - p.x) + abs(means[i].y - p.y) + abs(means[i].z - p.z); // manhattan dist
				float dist = abs(means[i].z - p.z); // by depth
				if (dist < minDist) {
					minDist = dist;
					minI = i;
				}
			}	

			// assign point to closest seed
			clusters[minI].push_back(p);
		}
	
		// calculate new centroids for each seed
		// set new seeds to centroid points
		for (int i=0; i<_numClusters; i++) {
			ofPoint centroid = getCentroid(clusters[i]);
			means[i].set(centroid);
		}

		_iterations--;
	}
	// finally draw all points colored
	ofColor colors[] = {ofColor::red, ofColor::green , ofColor::blue, ofColor::magenta, ofColor::cyan, ofColor::yellow};
	int numColors = 6;
	for (int i=0; i<_numClusters; i++) {
		drawPointCloud(ofPoint(), clusters[i], colors[i%numColors]);
	}
	
	delete[] means;
	delete[] clusters;
	// TODO: return results if needed
}

void ofxHandTracker::drawPointCloud(ofPoint _position, vector<ofPoint> &_cloud, ofColor _color) {
	
	ofPushMatrix();
	ofPushStyle();
	ofTranslate(_position);
	
	/*for(std::vector<ofPoint>::iterator it = _cloud.begin() ; it != _cloud.end(); ++it)
		ofPoint &p = *it; */

	ofSetColor(_color);
	glBegin(GL_POINTS);

	// drawing points
	for(std::vector<ofPoint>::iterator it = _cloud.begin(); it != _cloud.end(); ++it) {
		ofPoint &pos = *it;
		//ofColor color = userGen->getWorldColorAt(pos.x,pos.y, userID);
		//glColor4ub((unsigned char)color.r, (unsigned char)color.g, (unsigned char)color.b, (unsigned char)color.a);
		glVertex3f(pos.x, pos.y, pos.z);
	}

	glEnd();

	ofPopStyle();
	ofPopMatrix();
}

float ofxHandTracker::distFactor(float zDist) {
	float zz = zDist;
	return (zz) * 1/30;
}

void ofxHandTracker::getPalmCenterAndRadius(ofPoint &_palmCenter, float &_palmRadius) {
	/* PALM CENTER CALCULATION - from: http://blog.candescent.ch/2011/04/center-of-palm-hand-tracking.html
	1. foreach x-th candidate 
	2. calculate the smallest distance to any point in the contour
	3. then take the candidate with the largest minimum distance 
	*/
	float minDistance = 1000000;
	float minMaxDistance = 0;

	ofPoint minCandidate;
	ofPoint minMaxCandidate;

	//for(std::vector<ofPoint>::iterator canIt = handPalmCandidates.begin(); canIt != handPalmCandidates.end(); ++canIt) {
	for(std::vector<ofPoint>::iterator canIt = imgPalmCandidates.begin(); canIt != imgPalmCandidates.end(); ++canIt) {
	
		ofPoint &canPos = *canIt;
		minDistance = 10000000;

		//for(std::vector<ofPoint>::iterator edgeIt = handEdgePoints.begin(); edgeIt != handEdgePoints.end(); ++edgeIt) {
		for(std::vector<ofPoint>::iterator edgeIt = blobPoints.begin(); edgeIt != blobPoints.end(); ++edgeIt) {
		
			ofPoint &edgePos = *edgeIt;
			
			float currentDistance = edgePos.distance(canPos);
			if (currentDistance < minDistance && currentDistance > 5){ //
				minDistance = currentDistance;
				minCandidate = canPos;
			}
		}

		if(minDistance > minMaxDistance){
			minMaxDistance = minDistance;
			minMaxCandidate = minCandidate;
		}
	}

	//ofNoFill();
	//ofCircle(minMaxCandidate, minMaxDistance);
	//ofFill();

	_palmRadius = minMaxDistance;
	_palmCenter.set(minMaxCandidate);
}

void ofxHandTracker::getCloudBBox(ofPoint &_min, ofPoint &_max, vector<ofPoint> &_cloud) {
	_min.set(640, 480, MAX_HAND_DEPTH); // TODO: kinect viewport consts?
	_max.set(0,0,MIN_HAND_DEPTH);

	for(vector<ofPoint>::iterator it = _cloud.begin() ; it != _cloud.end(); ++it) {
		ofPoint &p = *it;
		if (p.x < _min.x)  _min.x = p.x;
		if (p.y < _min.y)  _min.y = p.y;
		if (p.z < _min.z)  _min.z = p.z;
		if (p.x > _max.x)  _max.x = p.x;
		if (p.y > _max.y)  _max.y = p.y;
		if (p.z > _max.z)  _max.z = p.z;
	}
}

// updates tracked hand position if found, else does nothing (uses hand generator and also user skeleton generator)
bool ofxHandTracker::getTrackedPosition(ofPoint &_trackedPosition) {
	bool handDetected = false;

	if(handGen->getNumTrackedHands() > 0) { // else use handGen
		int xx = handGen->getHand(hIndex)->projectPos.x;
		int yy = handGen->getHand(hIndex)->projectPos.y;
		int zz = handGen->getHand(hIndex)->projectPos.z;
		int raw = handGen->getHand(hIndex)->rawPos.Z;

		if(ofPoint(xx,yy,zz).distance(ofPoint::zero()) > 10) {
//			h.setOrigin(ofPoint(activeHandPos.x, activeHandPos.y));
			_trackedPosition.set(xx, yy, zz);

			handDetected = true;
		}
	}
	else if (userGen->getNumberOfTrackedUsers() > 0) { // if skeleton available use skeleton data
  												// here we can determine left or right hand?
		for (int i=0; i < userGen->getNumberOfTrackedUsers(); i++) {
			if(i == 0) {
				ofxTrackedUser* currentUser = userGen->getTrackedUser(i+1);
					
				ofxLimb closerArm;
				if (currentUser->left_lower_arm.position[1].Z < currentUser->right_lower_arm.position[1].Z) {
					closerArm = currentUser->left_lower_arm;
					//if (h.scaling.z < 0) // scaling hand to invert its orientation
					//	h.scaling.z = abs(h.scaling.z);
				}
				else {
					closerArm = currentUser->right_lower_arm;
					//if (h.scaling.z > 0)
					//	h.scaling.z = -abs(h.scaling.z);
				}

				int xx = closerArm.position[1].X;
				int yy = closerArm.position[1].Y;
				int zz = closerArm.position[1].Z;
				int raw = handGen->getHand(hIndex)->rawPos.Z;

				if(ofPoint(xx,yy,zz).distance(ofPoint::zero()) > 10) {
					//h.setOrigin(ofPoint(activeHandPos.x, activeHandPos.y));
					_trackedPosition.set(xx, yy, zz);

					handDetected = true;
				}
			}
		}

	}

	return handDetected;
}

	/*
	depthFbo.begin();
	int tx = handGen->getHand(hIndex)->projectPos.x;
	int ty = handGen->getHand(hIndex)->projectPos.y;
	ofRect(0,0,IMG_DIM,IMG_DIM);
	depthGen->draw(-tx + IMG_DIM/2, -ty + IMG_DIM/2);
	depthFbo.end();
	*/

void ofxHandTracker::update() {

	ofPoint prevActiveHandPos = activeHandPos; // remember previous location of the hand

	// new way of image clearing - any faster?
	realImg.setFromPixels(blankImg.getPixelsRef());

	bool handDetected = getTrackedPosition(activeHandPos); // update active hand position (passed by ref)
	h.setOrigin(ofPoint(activeHandPos.x, activeHandPos.y));	// update model's origin with new position, with no z dim

	if(handDetected) {
		fetchHandPointCloud(activeHandPos);	

		for(std::vector<ofPoint>::iterator it = handPoints.begin(); it != handPoints.end(); ++it) {
			ofPoint &pos = *it; // by reference (not copying)
			/*realImg.setColor((pos.x - palmCenter.x)*(IMG_DIM/300.0) + IMG_DIM/2, // too much shaking
							 (pos.y - palmCenter.y)*(IMG_DIM/300.0) + IMG_DIM/2, 
							 ofColor((-pos.z +  palmCenter.z)+128, 255));*/
			
			float tX = (pos.x - handCentroid.x)*(IMG_DIM/300.0) + IMG_DIM/2;
			float tY = (pos.y - handCentroid.y)*(IMG_DIM/300.0) + IMG_DIM/2;
			if (tX >= 0 && tX < IMG_DIM && tY >= 0 && tY < IMG_DIM) {
				// if crosses border causes 0xC0000005: Access violation reading location
				int col = 255 - (((pos.z - (handCentroid.z))/0.5)+128);
				if (col >= 255) col = 255;
				if (col <= 0)	col = 0;

				realImg.setColor((pos.x - handCentroid.x)*(IMG_DIM/300.0) + IMG_DIM/2, 
								 (pos.y - handCentroid.y)*(IMG_DIM/300.0) + IMG_DIM/2, 
								 //ofColor(((-pos.z + handCentroid.z)+128), 255));
								 ofColor(col, 255));
			}
		}

		//ofPixels depthPixels;
		//depthFbo.readToPixels(depthPixels);
		//colorImg.setFromPixels(depthPixels);

		imgPalmCandidates.clear();
		for(std::vector<ofPoint>::iterator it = handPalmCandidates.begin(); it != handPalmCandidates.end(); ++it) {
			ofPoint &pos = *it;

			imgPalmCandidates.push_back(ofPoint((pos.x - handCentroid.x)*(IMG_DIM/300.0) + IMG_DIM/2,
												(pos.y - handCentroid.y)*(IMG_DIM/300.0) + IMG_DIM/2, 0));

		}

		ofPoint prevPalmCenter = palmCenter;
		getPalmCenterAndRadius(palmCenter, palmRadius);

		if(palmCenter.distance(ofPoint::zero()) < 10) // some sort of safety, to prevent ofPoint::zero() -> TODO: generalize or replace with better solution
			palmCenter = prevPalmCenter;
		else {
			palmCenter = prevPalmCenter + ((palmCenter - prevPalmCenter)*0.6f); // we can smooth & interpolate to new center
		}
		realImg.setColor(palmCenter.x, palmCenter.y, 255); // white pixel in the middle of detected palm in real img

		// calculate average orientation
		/*ofPoint orientationVector;
		for(std::vector<ofPoint>::iterator it = handEdgePoints.begin(); it != handEdgePoints.end(); ++it) {
			ofPoint pos = *it;
			orientationVector += (pos - handRootCentroid);
		}
		ofPoint pos = handRootCentroid + orientationVector;
		
		glBegin(GL_LINES);
		glColor4ub(255, 128, 0, 0);
		glVertex3f(pos.x, pos.y, pos.z);
		glVertex3f(handRootCentroid.x, handRootCentroid.y, handRootCentroid.z);
		glEnd();
		*/
			
		ofPoint dirDown = ofPoint(handCentroid.x, handCentroid.y+50, 0);
		ofPoint dirRot = ofPoint(handRootCentroid.x, handRootCentroid.y, 0);
		
		//ofPoint downVector = dirDown - handCentroid;
		//ofPoint rotVector = dirRot - handCentroid;
		
		ofPoint downVector = ofPoint(0, -50, 0);
		ofPoint rotVectorZ = ofPoint(handRootCentroid.x - handCentroid.x,
									 handRootCentroid.y - handCentroid.y, 
									 0);
		ofPoint rotVectorX = ofPoint(0,
									 handRootCentroid.y - handCentroid.y, 
									 handRootCentroid.z - handCentroid.z);

		
		float prevRollAngle = rollAngle;
		rollAngle = angleOfVectors(downVector, rotVectorZ);

		if (abs(rollAngle - prevRollAngle) < 180)
			rollAngle = prevRollAngle + ((rollAngle - prevRollAngle)*0.5f); // smoothing
		
		/*
		double x = (handCentroid.x - handRootCentroid.x);
		double y = (handCentroid.y - handRootCentroid.y);
		
		double angleInRadiansXY = std::atan2(y, x);
		double angleInDegreesXY = (angleInRadiansXY / PI) * 180.0 + 90;
		*/

		// pre-rotate, so hand is facing us
		h.setRotation(ofQuaternion(90, ofVec3f(0,1,0)));

		// check if hand facing sensor directly and rotate accordingly
		// TODO: make smooth transition
		if(rotVectorZ == ofPoint::zero()) {
			//cout << rotVectorZ <<endl;
			rollAngle = 0;
			h.getRotationRef() *= ofQuaternion((-90), ofVec3f(1,0,0));
		}
		else {
			float angle = angleOfVectors(downVector, rotVectorX, false);
			//cout << "xangle: " << angle << endl;
			h.getRotationRef() *= ofQuaternion((180+angle), ofVec3f(1,0,0));
		}

		//h.curRot *= ofQuaternion((angleInDegreesXY), ofVec3f(0,0,1));
		h.getRotationRef() *= ofQuaternion((rollAngle), ofVec3f(0,0,1));
		
		/*
		//x = (maxZ - minZ);
		x = (handCentroid.z - handRootCentroid.z);
		
		double angleInRadiansZY = std::atan2(y, x);
		double angleInDegreesZY = (angleInRadiansZY * 180/PI) + 90;
		
		//h.curRot *= ofQuaternion((angleInDegreesZY), ofVec3f(1,0,0));
		*/
		
		// maybe scaling like this?
		//realImg.setAnchorPercent(0.5, 0.5);
		//realImg.resize();
		//realImg.crop();

		h.update();
		
		//generateModelProjection();
		realImg.update();
		analyzeContours(activeFingerTips);
		setParamsFromFingerTips(fingerTipsCounter);
	//}


	//for(int itY=0; itY<3; itY++) 
	///for(int itX=0; itX<3; itX++) {
		//int startX = palmCenter.x + (itX-1)*15; //(int)(IMG_DIM/2); //
		//int startY = palmCenter.y + (itY-1)*15; //(int)(IMG_DIM/2); //
		// here we try with searching for peaks on image
	/*
	int numberOfPoints = 8;
	float angle = 360.0f/numberOfPoints;
    
    for (int itX=0; itX<numberOfPoints; itX++) {

		int startX = palmCenter.x + 20 * cosf((angle*itX)*PI/180);
		int startY = palmCenter.y + 20 * sinf((angle*itX)*PI/180);
		//int max = modelImg.getColor(i,j).getBrightness();
		int maxVal = 0;
		int maxX = 0, maxY = 0;
		int counter = 0;

		int neighbours[3][3];

		while(counter < 20) {
		
			counter++;

			//cout << "3x3 part from img: \n >--------<" << endl;
			for(int i=0; i<3; i++) {
				for(int j=0; j<3; j++) {
					neighbours[i][j] = realImg.getColor(startX + (i-1), startY + (j-1)).getBrightness();
					//cout << " " << neighbours[i][j] << " ";

					if(i==1 && j==1)
						neighbours[i][j]= -1;
				}
				cout << endl;
			}

			bool newMaxFound = false; 

			for(int i=0; i<3; i++) {
				for(int j=0; j<3; j++) {
					if(neighbours[i][j] >= maxVal) {
						maxVal = neighbours[i][j];
						maxX = (i-1);
						maxY = (j-1);

						newMaxFound = true;
					}
				}
			}

			if(!newMaxFound)
				break;

			//neighbours[i][j] = modelImg.getColor(x + (i-1),y + (j-1)).getBrightness();

			startX += maxX;
			startY += maxY;

			maxX = 0, maxY = 0;

			if(startX < 0 || startX >= IMG_DIM)
				break;
			if(startY < 0 || startY >= IMG_DIM)
				break;

			realImg.setColor(startX, startY, ofColor::black); 
		}
	}
	realImg.update();
	*/
	
		/*handMask.setFromPixels(depthGen.getDepthPixels(zz-100, zz+100),
							   depthGen.getWidth(), 
							   depthGen.getHeight(), 
							   OF_IMAGE_GRAYSCALE);*/

		//handMask.setFromPixels(userGen.getUserPixels(), userGen.getWidth(), userGen.getHeight(), OF_IMAGE_GRAYSCALE);
			
		/*handMask.setFromPixels(depthGen.getDepthPixels(zz-120, zz+120),
						depthGen.getWidth(), 
						depthGen.getHeight(), 
						OF_IMAGE_GRAYSCALE);*/

		//if(thresh != 0)
		
		// temporary calling convex hull methods
		/*if(handEdgePoints.size() > 0) {
		//if(filteredMaxHandPoints.size() > 5) {
			//ofPoint pivot = ofPoint(maxDistRootPoint);
			ofPoint pivot = ofPoint(xx-bbMinX, yy+bbMaxY, 0);

			//pivot = ofPoint(minX, maxY, handCentroid.z);
			vector<ofPoint> convexHull = getConvexHull(handEdgePoints, pivot);

			//cout << "EDGE SIZE: " << handEdgePoints.size() << "\nHULL SIZE: " << convexHull.size() << endl;

			drawOfPointVector(convexHull);
			drawOfPointVectorFromCenter(convexHull, handCentroid);
		}
		*/

		// calc another angle
		/*double y2 = (handCentroid.y - handRootCentroid.y);
		double z = (handCentroid.z - handRootCentroid.z);
		double angleInRadiansYZ = std::atan2(y2, z);
	    double angleInDegreesYZ = (angleInRadiansYZ / PI) * 180.0 + 90;
		h.curRot *= ofQuaternion((angleInDegreesYZ), ofVec3f(1,0,0));
		*/

		//float handAngle = atan2(maxDistCentroidPoint.x-maxDistOppositePoint.x, maxDistCentroidPoint.y-maxDistOppositePoint.y);
		//float centroidDistance2D = sqrt(pow(handCentroid.x - handRootCentroid.x,2) + pow(handCentroid.y - handRootCentroid.y,2));
	}
}

void ofxHandTracker::analyzeContours(vector<ofPoint> &_activeFingerTips) {
	realImgCV_previous.setFromPixels(realImgCV.getPixelsRef());

	realImgCV.setFromPixels(realImg.getPixelsRef());
	realImgCV.dilate(); // if image dimensions are larger, image must be dilated before contour processing
	realImgCV.erode();

	realImgCV_previous.absDiff(realImgCV);

	tinyModelImg.scaleIntoMe(modelImgCV);
	tinyHandImg.scaleIntoMe(realImgCV);

	realImgCvContour.findContours(realImgCV,0,IMG_DIM*IMG_DIM,1, false, false); // this finds blobs

	int prevFingerTipsCounter = fingerTipsCounter;
	fingerTipsCounter = -1;

	if(realImgCvContour.blobs.size() > 0) {
	//if(realImgCvContour.nBlobs > 0) {

		ofxCvBlob handBlob = realImgCvContour.blobs[0];
		//blobPoints.clear();
		blobPoints = handBlob.pts;

		int size = blobPoints.size();
		
		vector<float> angles;
		vector<ofPoint>	fingerTips;

		int step = 12;
		float minAngle = 360;
		int minIndex = -1;

		float threshAngle = 60;

		fingerTipsCounter = 0;

		// TODO: refactor this algorithm to new function
		for(int i=step; i < (size+step); i++) { //*=step*/
			ofPoint prevPos = blobPoints[(i-step)];
			ofPoint curPos = blobPoints[(i)%size];
			ofPoint nextPos = blobPoints[(i+(step))%size];

			ofPoint prevVector = curPos - prevPos;
			ofPoint nextVector = curPos - nextPos;

			float kCosine = prevVector.dot(nextVector)/(prevVector.length()*nextVector.length());
			float angle = acosf(kCosine)*180/PI; //*180/PI
			//cout << "index : " << i << " angle: " << angle << endl;

			float normalZ = prevVector.crossed(nextVector).z; // for filtering peaks -> fingertips

			// here we search for minimum angles which define fingertips and gaps between them
			// also check if normalZ is greater or equal zero for fingertips
			if (angle < threshAngle && normalZ >= 0)
			{
				if (minAngle >= angle) {
					minIndex = i;
					minAngle = angle;
				}
			}
			else if (minIndex != -1){

					fingerTipsCounter++;
					fingerTips.push_back(ofPoint(blobPoints[(minIndex)%size].x, blobPoints[(minIndex)%size].y, 0));
					
					//fingerTips.at(fingerTipsCounter++) = ofPoint(blobPoints[(minIndex)%size].x, blobPoints[(minIndex)%size].y, 0);

					minAngle = 360;
					minIndex = -1;
			}

			angles.push_back(angle);
		}

		ofPoint rotVectorZ = ofPoint(handRootCentroid.x - handCentroid.x,
									 handRootCentroid.y - handCentroid.y, 
									 0);

		int prevCounter = fTipHistory[((fTipLastInd-1)%FTIP_HIST_SIZE)];

		fTipHistory[(fTipLastInd++)%FTIP_HIST_SIZE] = fingerTipsCounter;

		float avgFingerTips = 0;

		for (int i=0; i<FTIP_HIST_SIZE; i++) {
			avgFingerTips += fTipHistory[i];
		}
		avgFingerTips /= FTIP_HIST_SIZE;
		
		//bool fistFormed = isFistFormed(fingerTipsCounter);

		//fingerTipsCounter = floor(avgFingerTips+0.5); //round to nearest int
		//fingerTipsCounter = (int)(avgFingerTips+0.5);
		int avgFingers = (int)(avgFingerTips+0.5);
		fingerTipsCounter = avgFingers;
		/*
		stringstream ss;
		ss << "F TIP HIST: ";
		for (int i=0; i<FTIP_HIST_SIZE; i++) {
			ss << fTipHistory[i] << " ";
		}
		ss << endl;
		cout << ss.str() << "AVG FTIPS: " << avgFingerTips <<  endl;
		*/
		
		_activeFingerTips.clear();

		for(int i=0; i<fingerTips.size(); i++) {
			ofPoint fTip = fingerTips[i];
			ofPoint tempTip = fingerTips[(i+1)%fingerTips.size()];

			float angle = angleOfVectors(rotVectorZ, fTip - palmCenter);

			int xx = handGen->getHand(hIndex)->projectPos.x;
			int yy = handGen->getHand(hIndex)->projectPos.y;
			ofPoint handTrackedPos = ofPoint(xx,yy,0);

			_activeFingerTips.push_back((fTip - palmCenter) + handTrackedPos);
		}
	}	
}

bool ofxHandTracker::isFistFormed(int _fingerTipsCounter) {
	bool fistFormed = false; 
	if(_fingerTipsCounter == 0) {
		fistFormed = true;
		for (int i=0; i<FTIP_HIST_SIZE; i++) {
			if(fTipHistory[i] != 0) // fTipHistory is currently member of ofxHandTracker
				fistFormed = false;
		}

		if(!fistFormed) { // not sure anymore what this part does, but it alters the current index of ftip history array
			int index = (fTipLastInd-1)%FTIP_HIST_SIZE;
			while(fTipHistory[index] == 0) {
				index--;
				index = (index)%FTIP_HIST_SIZE;
			}
			_fingerTipsCounter = fTipHistory[((index-1)%FTIP_HIST_SIZE)];
		}
	}

	return fistFormed;
}

void ofxHandTracker::setParamsFromFingerTips(int _fingerTipsCounter) {
	// TODO: consider creating set of parameters which define only x angles (lets say 4 different parameter objects)
	// then for each case in checking for best parameters add one more for loop and each time add x angle param to current param
	// that way we can check for multiple finger poses (by x angle)
	// also we should check for which fingers we have to apply angles (not to all, just to these which are straight)
	// PARTIALLY DONE: distinguishing between fist and aligned fingers position is working
	const int X_PARAMS_SIZE = 6;
	ofxFingerParameters xParams[X_PARAMS_SIZE];

	xParams[0] = ofxFingerParameters(12, 0, -13, -20, 0); //default hand setup
	xParams[1] = ofxFingerParameters(9, 0, -8, -14, 0); 
	xParams[2] = ofxFingerParameters(7, 0, -6, -9, 0);
	xParams[3] = ofxFingerParameters(4, 0, -4, -4, 0);
	xParams[4] = ofxFingerParameters(1, 0, -2, 1, 0);
	xParams[5] = ofxFingerParameters(-3, 0, -1, 5, 0);    // fingers aligned tight by x angle

	//cout << "prev_curr img diff: " << getImageMatching(realImgCV_previous) << endl;
	//if (getImageMatching(realImgCV_previous) > 5) // 10?
	//	findParamsOptimum(p, 2);

	//TODO: here update hand model logic which is handled by different rules (palm radius, fingerTipsCounter, ...)
	if(_fingerTipsCounter == 0) {
	//if(avgFingers == 0) {
	//if(fistFormed) {
		//here check if hand completely closed or just closed fingers
		/*ofxFingerParameters zeroTips[2];
		zeroTips[0] = ofxFingerParameters(31) + ofxFingerParameters(1, 0, -1, -2, -30); // just fingers
		zeroTips[1] = ofxFingerParameters(0);				// fist formed
		findParamsOptimum(zeroTips, 2);
		*/
		
		findParamsOptimum(h, handSkeleton, modelSkeleton, tinyDiffImg, 31);
	}
	else if (_fingerTipsCounter >= 5) {
	//else if (avgFingers >= 5) {
		/*ofxFingerParameters p = ofxFingerParameters(31);
	//	p = p + xParams[0];

		//const int X_PARAMS_5_SIZE = 6;
		//ofxFingerParameters xParams5[X_PARAMS_5_SIZE];

		xParams[0] = ofxFingerParameters(11, 0, -10, -17, 0); //default hand setup
		xParams[1] = ofxFingerParameters(9, 0, -8, -14, -5); 
		xParams[2] = ofxFingerParameters(7, 0, -6, -9, -10);
		xParams[3] = ofxFingerParameters(4, 0, -4, -4, -15);
		xParams[4] = ofxFingerParameters(1, 0, -2, 1, -20);
		xParams[5] = ofxFingerParameters(0, 0, 0, 2, -25);    // fingers aligned tight by x angle
		
		int params[] = {31};
		int size = 1;

		//findParamsOptimum(params, size, xParams, X_PARAMS_SIZE); // not working properly
		findParamsOptimum(params, size);
		h.restoreFrom(p, false);*/

		findParamsOptimum(h, handSkeleton, modelSkeleton, tinyDiffImg, 31);
		//h.interpolate(p);
	}
	else if(_fingerTipsCounter == 1) {
	//else if (avgFingers == 1) {
		//int params1[] = {1, 2, 4, 8, 16};
		//int size = 5;
		// with no ring finger cause it's difficult to form that shape - 8,
		//int params1[] = {1, 2, /*4,*/ 16};
		//int size = 3;

		int params1[] = {2};
		int size = 1;
		//findParamsOptimum(params1, size, xParams, X_PARAMS_SIZE);
		findParamsOptimum(params1, size);
	}
	else if(_fingerTipsCounter == 2) {
	//else if (avgFingers == 2) {
		// 12 is index & middle which is hard to form together
		int params2[] = {3, 6, 17, 18}; 
		int size = 4;
		//findParamsOptimum(params2, size, xParams, X_PARAMS_SIZE);
		findParamsOptimum(params2, size);
	}
	else if(_fingerTipsCounter == 3) {
	//else if (avgFingers == 3) {
		int params3[] = {7, 11, 14, 19, 22, 28};
		int size = 6;
		findParamsOptimum(params3, size);
	}
	else if(_fingerTipsCounter == 4) {
	//else if (avgFingers == 4) {
		int params4[] = {15, 23, 27, 29, 30};
		int size = 5;
		findParamsOptimum(params4, size);
	}
}

float ofxHandTracker::angleOfVectors(ofPoint _downVector, ofPoint _rotVector, bool _absolute) {
	_downVector.normalize();
	_rotVector.normalize();

	float cosine = _downVector.dot(_rotVector); ///(downVector.length()*rotVector.length());
	float angle = acosf(cosine)*180/PI; //*180/PI
	
	if (_absolute) {
		float normalZ = _downVector.crossed(_rotVector).z; // for filtering peaks -> fingertips
		if(normalZ < 0)
			angle *= -1;
		angle += 180;
	}

	return angle;
}

void ofxHandTracker::draw() {

	int userID = 0;

	int width = userGen->getWidth();
	int height = userGen->getHeight();
		
	//int xx = handGen->getHand(handIndex)->projectPos.x;
	//int yy = handGen->getHand(handIndex)->projectPos.y;
	//int zz = handGen->getHand(handIndex)->projectPos.z;
	int thresh = handGen->getHand(hIndex)->rawPos.Z;

	ofPushMatrix();
	ofTranslate(-width/2, -height/2, -thresh*3);
	ofScale(2,2,2);
	
	/*glBegin(GL_POINTS);

	// drawing points
	for(std::vector<ofPoint>::iterator it = handPoints.begin(); it != handPoints.end(); ++it) {
		ofPoint pos = *it;
		ofColor color = userGen->getWorldColorAt(pos.x,pos.y, userID);
		glColor4ub((unsigned char)color.r, (unsigned char)color.g, (unsigned char)color.b, (unsigned char)color.a);
		glVertex3f(pos.x, pos.y, pos.z);
	}
	glEnd();*/

	//drawPointCloud(ofPoint(), handPoints, ofColor::white);
	//drawPointCloud(ofPoint(), handRootPoints, ofColor(127, 255, 0, 127));
	//drawPointCloud(ofPoint(), handEdgePoints, ofColor::magenta);
	//drawPointCloud(ofPoint(), handPalmCandidates, ofColor(255, 100, 100, 0));

	//kMeansClustering(handPoints, 10, 10); // also draws clusters for now

	// drawing orientation
	ofDrawArrow(handCentroid, handRootCentroid, 5.0);

	ofPopMatrix();

	glColor3f(1.0f, 1.0f, 1.0f);

	ofDrawBitmapString("HAND MODEL:", 20, 460);
	ofPushStyle();
	modelImg.draw(0, 480, 200, 200);
	ofPopStyle();

	ofDrawBitmapString("DILATED\nHAND MODEL:", 20, 650);
	modelImgCV.draw(0, 680, 200, 200);

	//h.draw();
	//h.getProjection().draw(200, 680, 200, 200);

	ofDrawBitmapString("HAND \nPOINT CLOUD:", 220, 450);

	ofPushStyle();
	//ofSetColor(0, 255, 0);
	realImg.draw(200, 480, 200, 200);
	ofPopStyle();

	filterMedian(&realImgCV, 3);
	ofDrawBitmapString("DILATED HAND \nPOINT CLOUD:", 420, 450);
	realImgCV.draw(400, 480, 200, 200);
	
	ofDrawBitmapString("PREV FRAME \n ABS DIFF:", 420, 680);
	realImgCV_previous.draw(400, 680, 200, 200);

	realImgCvContour.draw(0, 880, 200, 200);
	ofDrawBitmapString("CV Contour:", 60, 860);
	
	drawContours(ofPoint(200, 800, 0));
	
	/*ofxCvGrayscaleImage tiny; // NOTICE: reallocating occurs
	generateTinyImage(realImgCV, tiny, 24);
	tiny.draw(800, 680, 200, 200);
	ofDrawBitmapString("CV Tiny Image:", 800, 660);
	drawSideProjections(tiny, ofPoint(800, 680), 8);
	*/
	// draw real hand skeleton 
	generateRegionSkeleton(realImgCV, handSkeleton);
	handSkeleton.draw(600, 480, 200, 200);
	ofDrawBitmapString("CV Region Skeleton:", 600, 460);

	ofDrawBitmapString("CV Skeleton Blobs:", 800, 460);
	drawRegionSkeletons(handSkeleton, ofPoint(800, 480));

	// draw model skeleton 
	generateRegionSkeleton(modelImgCV, modelSkeleton);
	modelSkeleton.draw(1000, 480, 200, 200);
	ofDrawBitmapString("CV Model Skeleton:", 1000, 460);

	vector<ofVec4f> modelLines;
	findLines(modelSkeleton,  modelLines);
	filterLines(modelLines, ofPoint(1000, 860));
	//ofLog() << "NUM OF LINES: " << modelLines.size();
	ofPushMatrix();
	ofTranslate(1000, 680);
	for (int i=0; i<modelLines.size(); i++) {
		ofVec4f line = modelLines.at(i);
		ofLine(ofPoint(line.x, line.y), ofPoint(line.z, line.w));
	} 
	ofPopMatrix();
	ofDrawBitmapString("CV Model Lines:", 1000, 840);


	vector<ofVec4f> handLines;
	findLines(handSkeleton,  handLines);
	filterLines(handLines, ofPoint(1200, 860));
	//ofLog() << "NUM OF LINES: " << handLines.size();
	ofPushMatrix();
	ofTranslate(1200, 680);
	for (int i=0; i<handLines.size(); i++) {
		ofVec4f line = handLines.at(i);
		ofLine(ofPoint(line.x, line.y), ofPoint(line.z, line.w));
	} 
	ofPopMatrix();
	ofDrawBitmapString("CV Hand Lines:", 1200, 840);

	// TODO: find best matching from both skeletons (use image or lines), and adjust model parameters

	//float matching = getImageMatching(realImgCV, modelImgCV, diffImg);
	//float matching = getImageMatching(tinyHandImg, tinyModelImg, tinyDiffImg);
	float matching = getImageMatchingV2(handSkeleton, modelSkeleton, tinyDiffImg);
	glColor3f(1.0f, 1.0f, 1.0f);
	
	ofDrawBitmapString("HAND/MODEL MATCH: " + ofToString(matching), 1200, 460, 0);
	diffImg.draw(1200, 480, 200, 200);
	tinyDiffImg.draw(1200, 480, 200, 200);
	
	/*ofPushStyle();
		ofSetColor(255, 65, 170);
		ofDrawBitmapString("EDGE SET SIZE: " + ofToString(handEdgePoints.size()) + 
					   "\n  PALM SET SIZE: " + ofToString(handPalmCandidates.size()) + 
					   "\n  HAND SET SIZE: " + ofToString(handPoints.size()), 20, 180);
	ofPopStyle();
	*/

	ofPushStyle();
		ofSetColor(255, 65, 170);
		ofDrawBitmapString("RAW Z: " + ofToString(getHandDepth(activeHandPos.z, false)) + 
						   "\n  Z: " + ofToString(getHandDepth(activeHandPos.z)), 20, 150);
	ofPopStyle();
	
	ofPushStyle();
		ofSetColor(255, 65, 170);
		ofDrawBitmapString("FINGERTIPS: " + ofToString(fingerTipsCounter), 20, 120);
	ofPopStyle();
	
	ofDrawBitmapString("PCL Circularity: " + ofToString(getCircularity(handEdgePoints, handPoints)), 20, 90);

	if (handEdgePoints.size() > 0) {
		//ofLog() << "BEFORE: " << handEdgePoints.front();
		//sortEdgePoints(handEdgePoints); // TODO: debug why this does nothing?
		//ofLog() << "AFTER: " << handEdgePoints.front();

		vector<ofPoint> reducedEdgePoints;
		//simplifyByRadDist(handEdgePoints, reducedEdgePoints, 30);
		simplifyDP_openCV(handEdgePoints, reducedEdgePoints, 50);
		ofDrawBitmapString("Reduced PCL size: " + ofToString(reducedEdgePoints.size()), 20, 60);

		ofPushMatrix();
		ofTranslate(-width/2, -height/2, -thresh*3);
		ofScale(2,2,2);

		//drawPointCloud(ofPoint(), reducedEdgePoints, ofColor::magenta);

		glBegin(GL_POINTS);

		//for(std::vector<ofPoint>::iterator it = handEdgePoints.begin(); it != handEdgePoints.end(); ++it) {
		//	ofPoint pos = *it;
		for(int i=0; i < handEdgePoints.size(); i++) {
			ofPoint pos = handEdgePoints.at(i);
			ofColor color = ofColor(i, 255 - (i), 0);
			glColor4ub((unsigned char)color.r, (unsigned char)color.g, (unsigned char)color.b, (unsigned char)color.a);
			glVertex3f(pos.x, pos.y, pos.z);
		}
		glEnd();

		ofPopMatrix();
	}
}

void ofxHandTracker::filterMedian(ofxCvGrayscaleImage *i, int kernelSize) {
	if (kernelSize <= 2) kernelSize = 3;
	if (kernelSize % 2 == 0) kernelSize--;
	
	cv::Mat src = cv::Mat(i->height, i->width, CV_8UC1, i->getPixels(), 0);
	cv::Mat dst;
	//cv::GaussianBlur(src, dst2, cv::Size(15, 15), 1.0); // gaussian filtering if needed
	cv::medianBlur(src, dst, kernelSize); // filter with median to remove noise

	i->setFromPixels(dst.data, dst.size().width, dst.size().height);

	/*i->erode();
	i->dilate();
	i->erode();
	
	i->dilate();
	i->erode();
	i->dilate();*/
}

void ofxHandTracker::filterMedian(ofImage *i, int kernelSize) {
	if (kernelSize <= 2) kernelSize = 3;
	if (kernelSize % 2 == 0) kernelSize--;

	ofxCvGrayscaleImage imgCV;
	imgCV.allocate(i->height, i->width);
	filterMedian(&imgCV, kernelSize);
	imgCV.dilate();
	imgCV.erode();
	imgCV.dilate();

	imgCV.dilate();
	imgCV.erode();
	imgCV.dilate();

	i->setFromPixels(imgCV.getPixelsRef());
}

void ofxHandTracker::filterGauss(ofxCvGrayscaleImage *i, int kernelSize) {
	if (kernelSize <= 2) kernelSize = 3;
	if (kernelSize % 2 == 0) kernelSize--;
	
	cv::Mat src = cv::Mat(i->height, i->width, CV_8UC1, i->getPixels(), 0);
	cv::Mat dst;
	cv::GaussianBlur(src, dst, cv::Size(15, 15), 1.0); // gaussian filtering
	
	i->setFromPixels(dst.data, dst.size().width, dst.size().height);
	i->dilate();
	i->erode();
	i->dilate();
}


/**
 * Code for thinning a binary image using Zhang-Suen algorithm.
 * src: http://opencv-code.com/quick-tips/implementation-of-thinning-algorithm-in-opencv/
 */

/**
 * Perform one thinning iteration.
 * Normally you wouldn't call this function directly from your code.
 *
 * @param  im    Binary image with range = 0-1
 * @param  iter  0=even, 1=odd
 */
void thinningIteration(cv::Mat& im, int iter)
{
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);

    for (int i = 1; i < im.rows-1; i++)
    {
        for (int j = 1; j < im.cols-1; j++)
        {
            uchar p2 = im.at<uchar>(i-1, j);
            uchar p3 = im.at<uchar>(i-1, j+1);
            uchar p4 = im.at<uchar>(i, j+1);
            uchar p5 = im.at<uchar>(i+1, j+1);
            uchar p6 = im.at<uchar>(i+1, j);
            uchar p7 = im.at<uchar>(i+1, j-1);
            uchar p8 = im.at<uchar>(i, j-1);
            uchar p9 = im.at<uchar>(i-1, j-1);

            int A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) + 
                     (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) + 
                     (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                     (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
            int B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                marker.at<uchar>(i,j) = 1;
        }
    }

    im &= ~marker;
}

/**
 * Function for thinning the given binary image
 *
 * @param  im  Binary image with range = 0-255
 */
void thinning(cv::Mat& im)
{
    im /= 255;

    cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
    cv::Mat diff;

    do {
        thinningIteration(im, 0);
        thinningIteration(im, 1);
        cv::absdiff(im, prev, diff);
        im.copyTo(prev);
    } 
    while (cv::countNonZero(diff) > 0);

    im *= 255;
}

/*
	src: http://felix.abecassis.me/2011/09/opencv-morphological-skeleton/
*/
void ofxHandTracker::generateRegionSkeleton(ofxCvGrayscaleImage &_img, ofxCvGrayscaleImage &_result) {

	cv::Mat img = cv::Mat(_img.height, _img.width, CV_8UC1, _img.getPixels(), 0);
	cv::threshold(img, img, 16, 255, cv::THRESH_BINARY);

	cv::Mat skel(img.size(), CV_8UC1, cv::Scalar(0));
	cv::Mat temp(img.size(), CV_8UC1);
	cv::Mat eroded(img.size(), CV_8UC1);

	cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3)); // MORPH_OPEN

	cv::Mat prev = cv::Mat::zeros(img.size(), CV_8UC1);
    cv::Mat diff;

    /*do {
        cv::morphologyEx(img, temp, cv::MORPH_OPEN, element);
		cv::bitwise_not(temp, temp);
		cv::bitwise_and(img, temp, temp);
		cv::bitwise_or(skel, temp, skel);
		cv::erode(img, img, element);
 
        cv::absdiff(img, prev, diff);
        img.copyTo(prev);
    } 
    while (cv::countNonZero(diff) > 0);
	*/
	bool done;	 // updated code with optimizations	
	do
	{
	  cv::erode(img, eroded, element);
	  cv::dilate(eroded, temp, element); // temp = open(img)
	  cv::subtract(img, temp, temp);
	  cv::bitwise_or(skel, temp, skel);
	  eroded.copyTo(img);
 
	  done = (cv::countNonZero(img) == 0);
	} while (!done);
	
	_result.setFromPixels(skel.data, skel.size().width, skel.size().height);
	_result.dilate();
	_result.erode();
	_result.dilate();

	filterMedian(&_result, 3);
}

void ofxHandTracker::drawRegionSkeletons(ofxCvGrayscaleImage &_img, ofPoint _position) {
	ofxCvContourFinder	contourFinder;
	contourFinder.findContours(_img, _img.width*_img.height*0.001, _img.width*_img.height, 6, false, false); // this finds blobs
	if(contourFinder.blobs.size() > 0) {
		for (int i=0; i< contourFinder.blobs.size(); i++) {
			ofxCvBlob handBlob = contourFinder.blobs[i];
			vector<ofPoint> blobPoints = handBlob.pts;
			drawPointCloud(_position, blobPoints, ofColor::aliceBlue);

			ofPushMatrix();
			ofTranslate(_position);
			ofPoint min, max;
			getCloudBBox(min, max, blobPoints); 

			ofNoFill();
			ofRect(min, max.x - min.x, max.y - min.y);
			ofPopMatrix();
		}
	}
}

// finds lines on using CV probability Hough line transform
void ofxHandTracker::findLines(ofxCvGrayscaleImage &_img, vector<ofVec4f> &_lines) {
	cv::Mat img = cv::Mat(_img.height, _img.width, CV_8UC1, _img.getPixels(), 0);
//	cv::Canny(img, img, 50, 200, 3);

	/*vector<cv::Vec2f> lines;
	cv::HoughLines(img, lines, 1, CV_PI/180, 10);

    for( size_t i = 0; i < lines.size(); i++ )
    {
		_lines.push_back(ofVec4f(lines[i][0], lines[i][1], 0, 0 /*lines[i][2], lines[i][3]*));
    }

	 for( size_t i = 0; i < lines.size(); i++ )
	  {
		 float rho = lines[i][0], theta = lines[i][1];
		 ofPoint pt1, pt2;
		 double a = cos(theta), b = sin(theta);
		 double x0 = a*rho, y0 = b*rho;
		 pt1.x = cvRound(x0 + 1000*(-b));
		 pt1.y = cvRound(y0 + 1000*(a));
		 pt2.x = cvRound(x0 - 1000*(-b));
		 pt2.y = cvRound(y0 - 1000*(a));
		 //line( cdst, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
		 ofLine(pt1, pt2);
	  }*/

	  vector<cv::Vec4i> lines;
	  HoughLinesP(img, lines, 1, CV_PI/180, 10, 5);
	  for( size_t i = 0; i < lines.size(); i++ )
	  {
		//cv::Vec4i l = lines[i];
		//ofLine( ofPoint(l[0], l[1]), ofPoint(l[2], l[3]));
		_lines.push_back(ofVec4f(lines[i][0], lines[i][1], lines[i][2], lines[i][3]));
	  }
}

void ofxHandTracker::filterLines(vector<ofVec4f> &_lines, ofPoint _position) {

	// TODO: not useful yet, find better way to join / average line positions
	vector<vector<ofVec4f>> groups;

	for( size_t i = 0; i < _lines.size(); i++ )
	{
		ofVec4f l1 = _lines.at(i);

		bool push_new = false;
		if (groups.size() != 0) {
			bool pushed = false;
			for( size_t j = 0; j < groups.size(); j++ ) {
				if (groups.at(j).size() == 0) continue;

				ofVec4f l2 = groups.at(j).at(0);
				ofVec2f p1 = ofVec2f(abs(l1.x - l1.z)/2., abs(l1.y - l1.w)/2.);
				ofVec2f p2 = ofVec2f(abs(l2.x - l2.z)/2., abs(l2.y - l2.w)/2.);
				// TODO: we should also calculate angle similarity?

				if (p1.distance(p2) < 2.5) {
					groups.at(j).push_back(l1);
					pushed = true;
				}
			}
			if (!pushed) 
				push_new = true;
		}
		else {
			push_new = true;
		}
		
		if (push_new) {
			vector<ofVec4f> lns;
			lns.push_back(l1);
			groups.push_back(lns);
		}
	}

	//ofLog() << "NUM OF GROUPS: " << groups.size();
	// quick draw
	ofPushMatrix();
	ofTranslate(_position);
	for (int i = 0; i < groups.size(); i++)
	{
		if (groups.at(i).size() == 0) continue;

		ofVec2f avgP1, avgP2;
		for( int j = 0; j < groups.at(i).size(); j++ ) {
			ofVec4f l = groups.at(i).at(j);
			avgP1 += ofVec2f(l.x, l.y);
			avgP2 += ofVec2f(l.z, l.w);
		}
		avgP1 /= groups.at(i).size();
		avgP2 /= groups.at(i).size();

		ofLine(avgP1, avgP2);
	}
	ofPopMatrix();

	// remove lines:
	/*_lines.erase(std::remove_if(
    _lines.begin(), _lines.end(),
    [](const ofVec4f& v) { 
        return v.x < 0; // put your condition here
    }), _lines.end());*/
}

// horizontal and vertical region projections of tiny image
void ofxHandTracker::drawSideProjections(ofxCvGrayscaleImage &_tiny, ofPoint _position, int _size) {

	// heights
	for(int i=0; i<_tiny.width; i++) {
		int h =_tiny.countNonZeroInRegion(i, 0, 1, _tiny.height);
		ofRect(_position + ofPoint(i*_size, _tiny.height*_size), _size, h*_size); 
	}

	// widths
	for(int i=0; i<_tiny.height; i++) {
		int w =_tiny.countNonZeroInRegion(0, i, _tiny.width, 1);
		ofRect(_position + ofPoint( -w*_size, i*_size), w*_size, _size);
	}
}

IplImage* img_resize(IplImage* src_img, int new_width,int new_height)
{
    IplImage* des_img;
    des_img=cvCreateImage(cvSize(new_width,new_height),src_img->depth,src_img->nChannels);
    cvResize(src_img,des_img,CV_INTER_LINEAR);
    return des_img;
} 

void ofxHandTracker::generateTinyImage(ofxCvGrayscaleImage &_img, ofxCvGrayscaleImage &_result, int _size) {
	cv::Mat img = cv::Mat(_img.height, _img.width, CV_8UC1, _img.getPixels(), 0);
	IplImage *iplImg = new IplImage(img);
	IplImage *resized = img_resize(iplImg, _size, _size);
	_result.setFromPixels((const unsigned char*)resized->imageData, resized->width, resized->height);
	delete iplImg; // delete temp image
}

float ofxHandTracker::getCircularity(const vector<ofPoint> &_edgePoints, const vector<ofPoint> &_areaPoints) {
	// returns 4PI * A(R) / P(R)**2
	if (_edgePoints.size() == 0 || _areaPoints.size() == 0)
		return -1.0; // return invalid result

	float perimeter = (float)(_edgePoints.size());
	float area = (float)(_areaPoints.size());

	return FOUR_PI * (area / (perimeter * perimeter));
}

/*# Three points are a counter-clockwise turn if ccw > 0, clockwise if
# ccw < 0, and collinear if ccw = 0 because ccw is a determinant that
# gives the signed area of the triangle formed by p1, p2 and p3.
function ccw(p1, p2, p3):
    return (p2.x - p1.x)*(p3.y - p1.y) - (p2.y - p1.y)*(p3.x - p1.x)*/
int ofxHandTracker::ccw(ofPoint p1, ofPoint p2, ofPoint p3) {
	return (p2.x - p1.x)*(p3.y - p1.y) - (p2.y - p1.y)*(p3.x - p1.x);
}

ofPoint& ofxHandTracker::getPivotPoint(vector<ofPoint> &_edgePoints) {

	if (_edgePoints.size() == 0) return ofPoint();

	ofPoint pivot = _edgePoints.front();
	for (ofPoint p : _edgePoints) // get most lower left point
		if (pivot.y > p.y || pivot.y == p.y && pivot.x > p.x) {
			pivot = p;
		}
	return pivot;
}

void ofxHandTracker::sortEdgePoints(vector<ofPoint> &_edgePoints) {

	ofPointComparator comparator;
	comparator.pivot.set(getPivotPoint(_edgePoints));

	stable_sort(_edgePoints.begin(), _edgePoints.end(), comparator);
}

// must be sorted before calling this simplification
void ofxHandTracker::simplifyByRadDist( const vector<ofPoint>& _contourIn, vector<ofPoint>& _contourOut, float _threshDist) {
	
	if (_contourIn.size() == 0) return;

	ofPoint key = _contourIn.front(); // front is element at 0?
	_contourOut.push_back(key);

	for(int i = 1; i < _contourIn.size(); i++) {
		ofPoint pos = _contourIn.at(i);
		
		if (key.distance(pos) > _threshDist) {
			key.set(pos);
			_contourOut.push_back(pos);
		}
	}

}

void ofxHandTracker::simplifyDP_openCV ( const vector<ofPoint>& contourIn, vector<ofPoint>& contourOut, float tolerance ) {  
	//-- copy points.  

	int numOfPoints;  
	numOfPoints = contourIn.size();  

	CvPoint* cvpoints;  
	cvpoints = new CvPoint[ numOfPoints ];  

	for( int i=0; i<numOfPoints; i++)  
	{  
		int j = i % numOfPoints;  

		cvpoints[ i ].x = contourIn[ j ].x;  
		cvpoints[ i ].y = contourIn[ j ].y;  
	}  

	//-- create contour.  

	CvContour	contour;  
	CvSeqBlock	contour_block;  

	cvMakeSeqHeaderForArray  
	(  
		CV_SEQ_POLYLINE,  
		sizeof(CvContour),  
		sizeof(CvPoint),  
		cvpoints,  
		numOfPoints,  
		(CvSeq*)&contour,  
		&contour_block  
	);  

	//printf( "length = %f \n", cvArcLength( &contour ) );  

	//-- simplify contour.  

	CvMemStorage* storage;  
	storage = cvCreateMemStorage( 1000 );  

	CvSeq *result = 0;  
	result = cvApproxPoly  
	(  
		&contour,  
		sizeof( CvContour ),  
		storage,  
		CV_POLY_APPROX_DP,  
		cvContourPerimeter( &contour ) * tolerance,  
		0  
	);  

	//-- contour out points.  

	contourOut.clear();  
	for( int j=0; j<result->total; j++ )  
	{  
		CvPoint * pt = (CvPoint*)cvGetSeqElem( result, j );  

		contourOut.push_back( ofPoint() );  
		contourOut.back().x = (float)pt->x;  
		contourOut.back().y = (float)pt->y;  
	}  

	//-- clean up.  

	if( storage != NULL )  
		cvReleaseMemStorage( &storage );  

	delete[] cvpoints;  
}  

void ofxHandTracker::drawContours(ofPoint _position) {
	// just drawing part of contour analysis
	if(realImgCvContour.blobs.size() > 0) {
	//if(realImgCvContour.nBlobs > 0) {

		ofxCvBlob handBlob = realImgCvContour.blobs[0];

		int size = blobPoints.size();
		
		ofPushMatrix();
		ofTranslate(_position); // ofPoint(1500, 0, 0)
		ofScale(2,2,1);
		ofNoFill();
		ofSetColor(255,255,255);
		ofCircle(palmCenter, palmRadius);
		ofFill();
		ofPopMatrix();
		
		vector<float> angles;
		vector<ofPoint>	fingerTips;

		int step = 12;
		float minAngle = 360;
		int minIndex = -1;

		for(int i=step; i < (size+step); i++) { //*=step*/
			ofPoint prevPos = blobPoints[(i-step)];
			ofPoint curPos = blobPoints[(i)%size];
			ofPoint nextPos = blobPoints[(i+(step))%size];

			ofPoint prevVector = curPos - prevPos;
			ofPoint nextVector = curPos - nextPos;

			float kCosine = prevVector.dot(nextVector)/(prevVector.length()*nextVector.length());
			float angle = acosf(kCosine)*180/PI; //*180/PI
			//cout << "index : " << i << " angle: " << angle << endl;

			float normalZ = prevVector.crossed(nextVector).z; // for filtering peaks -> fingertips

			ofPushMatrix();
			ofTranslate(_position + ofPoint(100, 80)); // ofPoint(1250, 0, 0)
			/*ofScale(2,2,1);
			ofSetColor(255-(angle*255.0/360.0), 0, angle*255.0/360.0);
			ofRect(blobPoints[(i)%size].x, blobPoints[(i)%size].y, 2,2);
			*/
			//ofTranslate();		// ofPoint(-25, 0, 0)
			ofScale(0.5,0.5,1); // histogram like plotting of angles
			glBegin(GL_LINES);
			glColor4ub(255-(angle*255.0/360.0), 0, angle*255.0/360.0, 255); // hand contour angle plotting
			glVertex3f(i+300, 200, 0);
			glVertex3f(i+300, 200 - angle, 0);
			glEnd();
			ofPopMatrix();

			ofPushMatrix();
			ofTranslate(_position);
			ofScale(2,2,1);
			// here we search for minimum angles which define fingertips and gaps between them
			// also check if normalZ is greater or equal zero for fingertips
			if (angle < 60.0 && normalZ >= 0)
			{
				if (minAngle >= angle) {
					minIndex = i;
					minAngle = angle;
				}
			}
			else if (minIndex != -1){
					ofSetColor(255,0,0);
					ofRect(blobPoints[(minIndex)%size].x, blobPoints[(minIndex)%size].y, 3, 3);

					glBegin(GL_LINES);
			
						glColor4ub(128, 255, 128, 255);
						glVertex3f(blobPoints[(minIndex)%size].x, blobPoints[(minIndex)%size].y, 0);
						glVertex3f(palmCenter.x, palmCenter.y, 0);
			
					glEnd();

					minAngle = 360;
					minIndex = -1;
			}
			
			glBegin(GL_LINES);
			glColor4ub(255, 255, i, 255);
			glVertex3f(blobPoints[(i)%size].x, blobPoints[(i)%size].y, 0);
			glVertex3f(blobPoints[(i+1)%size].x, blobPoints[(i+1)%size].y, 0);
			glEnd();
			
			ofPopMatrix();
		}

		// draw rectangles representing number of active fingertips => like this (for 3 fingertips): [][][] 
		ofPushMatrix();
		ofTranslate(_position + ofPoint(300, 200)); // ofPoint(1325, 500, 0)
		ofScale(2,2,1);
		for(int i=0; i<fingerTipsCounter; i++) {
			ofSetColor(255 - i*255/fingerTipsCounter, i*255/fingerTipsCounter, 0);
			ofRect(i*10, 0, 10, 10);
		}
		ofPopMatrix();

		for(int i=0; i<fingerTips.size(); i++) {
			ofPoint &fTip = fingerTips[i];
			ofPoint &tempTip = fingerTips[(i+1)%fingerTips.size()];

			ofSetColor(0, 255, 0);
			ofRect(fTip.x, fTip.y, 10, 10);
			
			//ofSetColor(50*(i+1), 0, 0);
			//ofLine(fTip, tempTip);
		}
	}
}

float ofxHandTracker::getHandDepth(float _rawZ, bool _normalized, float _minZ, float _maxZ) {
	float raw = _rawZ;

	if (_normalized) {		
		float percentage = (float)(raw - _minZ)/(float)(_maxZ - _minZ); // od 0 do 1500/1500
		percentage = ofClamp(percentage, 0, 1);

		return percentage;
	}
	return raw;
}

void ofxHandTracker::findParamsOptimum(ofxHandModel &_h, 
									   ofxCvGrayscaleImage &_hand, 
									   ofxCvGrayscaleImage &_model, 
									   ofxCvGrayscaleImage &_diff, 
									   short _mask) {
	ofxFingerParameters bestParams = _h.saveFingerParameters();
	generateModelProjection();
	float			bestMatching = getImageMatchingV2(_hand, _model, _diff);
	//float			bestMatching = IMG_DIM*IMG_DIM;
	for(float i=0.1; i<1.0; i+=0.1) {

		//for(float j=0.1; j<1.0; j+=0.1) {
			_h.spread(i, _mask);
			_h.open(1.0, _mask);
			ofxFingerParameters curParams = _h.saveFingerParameters();
			//h.restoreFrom(curParams);

			//tinyModelImg.set(0);
			generateModelProjection();
			//tinyModelImg.scaleIntoMe(modelImgCV);
			//tinyHandImg.scaleIntoMe(realImgCV);

			//float matching = getImageMatching(tinyHandImg, tinyModelImg, tinyDiffImg);
			//float matching = shaderMatcher.matchImages(modelImg, realImg);
			//float matching = getImageMatching(modelImg, shaderMatcher.imagesAbsDiff(modelImg, realImg));
			float matching = getImageMatchingV2(_hand, _model, _diff);


			if(matching < bestMatching) 
			//if(matching < bestMatching && matching > 0.01)
			{
				bestMatching = matching;
				bestParams = curParams;
			}
		//}
	}
	_h.restoreFrom(bestParams);
}

void ofxHandTracker::findParamsOptimum(int _params[], int _size) {
	ofxFingerParameters bestParams = ofxFingerParameters(0);
	float			bestMatching = 0;
	//float			bestMatching = IMG_DIM*IMG_DIM;
	for(int i=0; i<_size; i++) {
		ofxFingerParameters curParams = ofxFingerParameters(_params[i]);
		h.restoreFrom(curParams);

		tinyModelImg.set(0);
		generateModelProjection();
		tinyModelImg.scaleIntoMe(modelImgCV);
		tinyHandImg.scaleIntoMe(realImgCV);

		//float matching = getImageMatching(tinyHandImg, tinyModelImg, tinyDiffImg);
		//float matching = shaderMatcher.matchImages(modelImg, realImg);
		//float matching = getImageMatching(modelImg, shaderMatcher.imagesAbsDiff(modelImg, realImg));
		float matching = getImageMatchingV2(handSkeleton, modelSkeleton, tinyDiffImg);


		if(matching > bestMatching) 
		//if(matching < bestMatching && matching > 0.01)
		{
			bestMatching = matching;
			bestParams = curParams;
		}
	}

	ofSetColor(255,255,255);
	ofDrawBitmapString("best_match: " + ofToString(bestMatching), 30, 30);

	h.restoreFrom(bestParams);
}

void ofxHandTracker::findParamsOptimum(ofxFingerParameters _params[], int _size) {
	ofxFingerParameters bestParams = ofxFingerParameters(0);
	float			bestMatching = 0;
	//float			bestMatching = IMG_DIM*IMG_DIM;
	for(int i=0; i<_size; i++) {
		ofxFingerParameters curParams = _params[i];
		h.restoreFrom(curParams);

		tinyModelImg.set(0);
		generateModelProjection();
		tinyModelImg.scaleIntoMe(modelImgCV);
		tinyHandImg.scaleIntoMe(realImgCV);

		//float matching = getImageMatching(tinyHandImg, tinyModelImg, tinyDiffImg);
		//float matching = shaderMatcher.matchImages(modelImg, realImg);
		float matching = getImageMatching(modelImg, shaderMatcher.imagesAbsDiff(modelImg, realImg));

		if(matching > bestMatching) 
		//if(matching < bestMatching && matching > 0.01)
		{
			bestMatching = matching;
			bestParams = curParams;
		}
	}

	ofSetColor(255,255,255);
	ofDrawBitmapString("best_match: " + ofToString(bestMatching), 30, 30);

	h.restoreFrom(bestParams);
}

void ofxHandTracker::findParamsOptimum(int _paramsZ[], int _sizeZ, ofxFingerParameters _paramsX[], int _sizeX) {
	ofxFingerParameters bestParams = ofxFingerParameters(0);
	float			bestMatching = 0;
	//float			bestMatching = IMG_DIM*IMG_DIM;
	for(int i=0; i<_sizeZ; i++) {
		ofxFingerParameters curParams = ofxFingerParameters(_paramsZ[i]);
		for(int j=0; j<_sizeX; j++) {
			ofxFingerParameters merged = curParams + _paramsX[j];

			h.restoreFrom(merged, true);

			generateModelProjection();
			//tinyModelImg.scaleIntoMe(modelImgCV);
			//tinyHandImg.scaleIntoMe(realImgCV);

			//float matching = getImageMatching(tinyHandImg, tinyModelImg, tinyDiffImg);
			//float matching = shaderMatcher.matchImages(modelImg, realImg);
			//float matching = getImageMatching(modelImg, shaderMatcher.imagesAbsDiff(modelImg, realImg));
			float matching = getImageMatchingV2(handSkeleton, modelSkeleton, tinyDiffImg);

			tinyModelImg.set(0);

			if(matching > bestMatching) 
			//if(matching < bestMatching && matching > 0.01)
			{
				bestMatching = matching;
				bestParams = merged;
			}
		}
	}

	ofSetColor(255,255,255);
	ofDrawBitmapString("best_match: " + ofToString(bestMatching), 30, 30);

	h.restoreFrom(bestParams, true);
}

/*ofxHandTracker::~ofxHandTracker(void)
{
	//free(userGen);
	//free(handGen);
	//userGen = NULL;
	//handGen = NULL;
}*/


//------------------------- helper methods -------------------------------------
// here maybe rather than void we should return generated image directly?
void ofxHandTracker::generateModelProjection() {
	//ofPoint backupScaling = h.getScale();
	// rescale model, so it fits better to real hand depth image
	// 0.6 downscales the actual scaling range (from 0 - 1 to 0 - 0.6),
	// - 0.15 adds some startup scaling, so model is not underscaled 
	//h.scaling -= ((getHandDepth(activeHandPos.z)*0.6) - 0.15); 

	//h.setScale(backupScaling - ((getHandDepth(activeHandPos.z)*0.6) - 0.15));

	palmCenter.z = 0;
	h.getProjection(modelImg, palmCenter); //palmCenter, 6*(1-getHandDepth(activeHandPos.z))
	//modelImg = h.getProjection();
	modelImg.setImageType(OF_IMAGE_GRAYSCALE);
	modelImgCV.setFromPixels(modelImg);
		
	//h.setScale(backupScaling);
	
	//modelImgCV.dilate();
	//modelImgCV.erode();
}

float ofxHandTracker::getImageMatchingV2(ofxCvGrayscaleImage &realImage, 
										ofxCvGrayscaleImage &modelImage,  
										ofxCvGrayscaleImage &differenceImage) {

	cv::Mat real = cv::Mat(realImage.height, realImage.width, CV_8UC1, realImage.getPixels(), 0);
	cv::Mat model = cv::Mat(modelImage.height, modelImage.width, CV_8UC1, modelImage.getPixels(), 0);

	int realCount = cv::countNonZero(real);
	int modelCount = cv::countNonZero(model);

	cv::Mat diff;
	//cv::threshold(img, img, 16, 255, cv::THRESH_BINARY);

	cv::absdiff(real, model, diff);
	differenceImage.setFromPixels(diff.data, realImage.width, realImage.height);
	int diffCount = cv::countNonZero(diff);
	return (float)diffCount / (float)realCount;
}

float ofxHandTracker::getImageMatching(ofxCvGrayscaleImage &realImage, 
									ofxCvGrayscaleImage &modelImage,  
									ofxCvGrayscaleImage &differenceImage) {
					
	//ofPixels realPixels = realImage.getPixelsRef();
	//realPixels[0]

	unsigned char *real = realImage.getPixels();
	//unsigned char *model = modelImage.getPixels();

	//ofPixels real = realImage.getPixelsRef();
	ofPixels model = modelImage.getPixelsRef();

	int w = realImage.getWidth();
	int h = realImage.getHeight();

	float allDiff = 0;
	float handSum = 0;
	float modelSum = 0;

	for (int i=0; i<w*h; i++) {
		handSum += real[i];
		real[i] =  abs(model[i] - real[i]);
		allDiff += real[i];
	}
	
	differenceImage.setFromPixels(real, w, h);
	//differenceImage.erode();

	float matching = (handSum - allDiff)/handSum;
	matching = matching * 10.0/6.0;
	return matching;
}

float ofxHandTracker::getImageMatching(ofxCvGrayscaleImage &differenceImage) {

	unsigned char *diff = differenceImage.getPixels();

	int w = differenceImage.getWidth();
	int h = differenceImage.getHeight();

	float allDiff = 0;

	for (int i=0; i<w*h; i++) {
		allDiff += diff[i];
	}
	
	float matching = allDiff/(w*h);
	//matching = matching * 10.0/6.0;
	return matching;
}


float ofxHandTracker::getImageMatching(ofImage &realImage,   
									ofImage &diffImage) {
					
	ofPixels realPixels = realImage.getPixelsRef();
	ofPixels diffPixels = diffImage.getPixelsRef();

	int w = realImage.getWidth();
	int h = realImage.getHeight();

	float allDiff = 0;
	float handSum = 0;
	float modelSum = 0;

	for (int i=0; i<w; i++) {
		for (int j=0; j<h; j++) {
			handSum += realPixels.getColor(i,j).getBrightness();
			allDiff += diffPixels.getColor(i,j).getBrightness();
		}
	}

	float matching = (handSum - allDiff)/handSum;
	matching = matching * 10.0/6.0;
	return matching;
}
/*
float ofxHandTracker::getImageMatching(ofxCvGrayscaleImage &realImage,   
									ofxCvGrayscaleImage &diffImage) {
					
	ofPixels realPixels = realImage.getPixelsRef();
	ofPixels diffPixels = diffImage.getPixelsRef();

	int w = realImage.getWidth();
	int h = realImage.getHeight();

	float allDiff = 0;
	float handSum = 0;
	float modelSum = 0;

	for (int i=0; i<w; i++) {
		for (int j=0; j<h; j++) {
			handSum += realPixels.getColor(i,j).getBrightness();
			allDiff += diffPixels.getColor(i,j).getBrightness();
		}
	}

	float matching = (handSum - allDiff)/handSum;
	matching = matching * 10.0/6.0;
	return matching;
}*/

ofPoint ofxHandTracker::getCentroid(vector<ofPoint> &points){
		float centroidX = 0, centroidY = 0, centroidZ = 0;

		// calculating of centroid
		for(std::vector<ofPoint>::iterator it = points.begin(); it != points.end(); ++it) {
			/* std::cout << *it; ... */
			ofPoint &p = *it;
			centroidX += p.x;
			centroidY += p.y;
			centroidZ += p.z;
		}

		if(points.size() > 0) {
			centroidX /= points.size();
			centroidY /= points.size();
			centroidZ /= points.size();
		}

		ofPoint centroid = ofPoint(centroidX, centroidY, centroidZ);
		return centroid;
}


//Bitmap/Bresenham's line algorithm - source: http://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm#C
/*void ofxHandTracker::drawLine(ofImage *img, int x0, int y0, int z0, int x1, int y1, int z1) {

	z0 += 120;
	z1 += 120;
	int dz = (z1-z0)/20;
	//if(z0 > z1){
	//	int temp = z0;
	//	z0 = z1;
	//	z1 = temp;
	//}

	//cout << "z0: " << z0 << " z1: " << z1 << endl;

	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
	int err = (dx>dy ? dx : -dy)/2, e2;
 
	
	int startX = x0;
	//float stepZ;
	//if(dx != 0)
	//	stepZ = dz/dx;
	//else if(dy != 0)
	//	stepZ = dz/dy;
	//else
	//	stepZ = 1;
	//int step = 2;
	for(;;){
		//if(step%2 == 0)
			img->setColor(x0, y0, ofColor(z0, 255));
		//step++;
		
		z0 += dz;

		if (x0==x1 && y0==y1) break;
		e2 = err;
		if (e2 > -dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}*/

// section with methods which provide useful data from tracker

vector<ofPoint> ofxHandTracker::getActiveFingerTips() {
	return activeFingerTips;
}

ofxHandModel& ofxHandTracker::getHandModelRef() {
	return h;
}

int	ofxHandTracker::getNumFingerTips() {
	return fingerTipsCounter;
}

/*
void ofxHandTracker::drawLine(ofImage *img, int x1, int y1, int z1, int x2, int y2, int z2)
{
	z1 += 120;
	z2 += 120;

	int dz = (z2-z1)/20;

        // Bresenham's line algorithm
	const bool steep = (abs(y2 - y1) > abs(x2 - x1));
	if(steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}
 
	if(x1 > x2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}
 
	const float dx = x2 - x1;
	const float dy = abs(y2 - y1);
 
	float error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = (int)y1;
 
	const int maxX = (int)x2;
 
	for(int x=(int)x1; x<maxX; x++)
	{
		if(steep)
                {
					img->setColor(y, x, ofColor(z1, 255));
                }
		else
                {
					img->setColor(x, y, ofColor(z1, 255));
                }
 
		//z1 += dz;

                error -= dy;
	        if(error < 0)
	        {
		        y += ystep;
		        error += dx;
	        }
	}
}*/