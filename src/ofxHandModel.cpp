#include "ofxHandModel.h"

ofxHandModel::ofxHandModel(void)
{
	origin = ofPoint();
	scaling = ofPoint(FIXED_SCALE);

	f[1] = new ofxFingerModel(ofPoint(0, -80+40-60, 75));
	f[2] = new ofxFingerModel(ofPoint(0, -100+40-60, 25));
	f[3] = new ofxFingerModel(ofPoint(0, -90+40-60, -25));
	f[4] = new ofxFingerModel(ofPoint(0, -70+40-60, -75));
	f[0] = new ofxThumbModel(f[1]->root.origin - ofPoint(0, f[1]->palm.length, 0));

	f[1]->root.angleX = 11;
	f[2]->root.angleX = 0;
	f[3]->root.angleX = -10; 
	f[4]->root.angleX = -17;
	
	f[0]->setTipLength(47.5);
	f[1]->setTipLength(35);
	f[2]->setTipLength(40);
	f[3]->setTipLength(37.5);
	f[4]->setTipLength(30);

	f[0]->root.origin.set(f[1]->root.origin - ofPoint(0, f[1]->palm.length, 0));

	// Quaternion rotation init
	//this slows down the rotate a little bit
	dampen = .4;
	curRot.makeRotate(90, ofVec3f(0,1,0));

	// init of experimental section with fbos & shaders
	ofFbo::Settings s = ofFbo::Settings();  
	s.width = IMG_DIM;  
	s.height = IMG_DIM;  
	//s.useDepth = true;  
	//s.useStencil = true;  
	//s.depthStencilAsTexture = true;
	meshFbo.allocate(s);  
	meshFbo.setUseTexture(true);

	//meshFbo.allocate(IMG_DIM, IMG_DIM);
	dilateFbo.allocate(IMG_DIM, IMG_DIM);

	cout << "FBO objects supported: " << meshFbo.checkGLSupport() << endl;

	dilateShader.load("shaders/dilate");

	projImg.allocate(IMG_DIM, IMG_DIM, OF_IMAGE_GRAYSCALE);
	projImg.setImageType(OF_IMAGE_GRAYSCALE);

	projPix.allocate(IMG_DIM, IMG_DIM, OF_IMAGE_GRAYSCALE);
}


ofxHandModel::~ofxHandModel(void)
{
	for(int i=0; i<NUM_FINGERS; i++) {
		delete f[i];
		f[i] = NULL;
	}
}

void ofxHandModel::update()
{
	//interpolationTimer.update();

	// update all fingers
	for(int i=0; i<NUM_FINGERS; i++) {
		f[i]->update();
	}

	// here we should store all new vertices to some mesh or vbo object
	// to be able to use it in shaders, for better drawing, dilatation etc.
	modelMesh.clear();
	palmMesh.clear();

	ofPoint s = scaling; // save scaling
	scaling.set(FIXED_SCALE);

	ofColor colors[] = {ofColor::black, ofColor::red, ofColor::green, ofColor::blue, ofColor::magenta, ofColor::cyan, ofColor::yellow};
	int numColors = 1; // if numColors == 1, model all white
	for(int i=0; i<NUM_FINGERS; i++) {
		
		modelMesh.addVertex(f[i]->root.origin);
		modelMesh.addVertex(f[i]->mid.origin);
		modelMesh.addVertex(f[i]->mid.origin);
		modelMesh.addVertex(f[i]->top.origin);
		modelMesh.addVertex(f[i]->top.origin);
		modelMesh.addVertex(f[i]->fingerTip);

		// for proper depth coloring
		modelMesh.addColor(getPointColor(f[i]->root.origin) - colors[i%numColors]);
		modelMesh.addColor(getPointColor(f[i]->mid.origin) - colors[i%numColors]);
		modelMesh.addColor(getPointColor(f[i]->mid.origin) - colors[i%numColors]);
		modelMesh.addColor(getPointColor(f[i]->top.origin) - colors[i%numColors]);
		modelMesh.addColor(getPointColor(f[i]->top.origin) - colors[i%numColors]);
		modelMesh.addColor(getPointColor(f[i]->fingerTip) - colors[i%numColors]);
	}

	// setup vertices & depth colors for faces of palm (TRIANGLE_FAN style)
	//--------------------------------------------------------------------------------------------------
	ofPoint f0r = f[0]->root.origin;
	ofPoint f0m = f[0]->mid.origin; 
	ofPoint f1r = f[1]->root.origin;

	ofPoint f2r = f[2]->root.origin;
	ofPoint f3r = f[3]->root.origin;
	ofPoint f4r = f[4]->root.origin;
	
	// custom points for palm
	ofPoint f4c = ofPoint(f[4]->root.origin.x, f[4]->root.origin.y - f[4]->palm.length, f[4]->root.origin.z);
	ofPoint f3c = ofPoint(f[3]->root.origin.x, /*2*/(f[3]->root.origin.y - f[3]->palm.length), f[3]->root.origin.z);
	ofPoint f0c = ofPoint(f[1]->root.origin.x, /*5*/(f[1]->root.origin.y - f[1]->palm.length), f[2]->root.origin.z);

	palmMesh.addVertex(f0r);
	palmMesh.addVertex(f0m);
	palmMesh.addVertex(f1r);

	palmMesh.addColor(getPointColor(f0r) /*- ofColor::yellow*/);
	palmMesh.addColor(getPointColor(f0m) /*- ofColor::yellow*/);
	palmMesh.addColor(getPointColor(f1r) /*- ofColor::yellow*/);

	palmMesh.addVertex(f2r);
	palmMesh.addVertex(f3r);
	palmMesh.addVertex(f4r);

	palmMesh.addColor(getPointColor(f2r) /*- ofColor::yellow*/);
	palmMesh.addColor(getPointColor(f3r) /*- ofColor::yellow*/);
	palmMesh.addColor(getPointColor(f4r) /*- ofColor::yellow*/);

	palmMesh.addVertex(f4c);
	palmMesh.addVertex(f3c);
	palmMesh.addVertex(f0c);

	palmMesh.addColor(getPointColor(f4c) /*- ofColor::yellow*/);
	palmMesh.addColor(getPointColor(f3c) /*- ofColor::yellow*/);
	palmMesh.addColor(getPointColor(f0c) /*- ofColor::yellow*/);
	//--------------------------------------------------------------------------------------------------

	scaling.set(s); // restore scaling
}

ofFloatColor ofxHandModel::getPointColor(ofPoint p) {
	float lim = 45.0f;
	ofPoint pc = getWorldCoord(p, origin);
	ofFloatColor fc = ofFloatColor(((pc.z - (origin.z - 30))/lim)/2.0);
	return fc;
}

void ofxHandModel::drawMesh() {
	glMatrixMode(GL_MODELVIEW);
	
	//ofEnableDepthTest(); // or
	glEnable(GL_DEPTH_TEST);

	glPushMatrix();

	//glTranslatef(origin.x, origin.y, origin.z);
	glTranslatef(IMG_DIM/2, IMG_DIM/2, origin.z);
	
	//Extract the rotation from the current rotation
    ofVec3f axis;  
    float angle;  
    curRot.getRotate(angle, axis);  
	
	glRotatef(angle, axis.x, axis.y, axis.z); //apply the quaternion's rotation
	glScalef(scaling.x, scaling.y, scaling.z);
	glScalef(0.5f, 0.5f, 0.5f);

	palmMesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
	palmMesh.drawFaces();

	modelMesh.setMode(OF_PRIMITIVE_LINES);
	modelMesh.draw();

	glPopMatrix();

	glDisable(GL_DEPTH_TEST);
}

void ofxHandModel::draw() 
{
	/*
	//ofQuaternion constructor: angle, ofVec3f axis
	ofQuaternion qr (rotation.z, ofVec3f(1,0,0));	// quat roll.
	ofQuaternion qp (rotation.x, ofVec3f(0,0,1));	// quat pitch.
	ofQuaternion qh (rotation.y, ofVec3f(0,1,0));	// quat heading or yaw.
	//ofQuaternion qt;					// quat total.

	// The order IS IMPORTANf[0]-> Apply roll first, then pitch, then heading.
	curRot = qr * qp * qh;
	*/
	glMatrixMode(GL_MODELVIEW);
	
	glPushMatrix();
	ofPushStyle();
	glTranslatef(origin.x, origin.y, origin.z);

	/*
	//if(rotation.x != 0)
		glRotatef(rotation.x, 1, 0, 0);
	//if(rotation.y != 0)

	// y and z axis are changed while we rotate around x, so rotations below wont rotate locally -> instead we need to rotate around new (changed) y and z !!!
	// upper line is not totally correct - we need to calculate local axis of the hand each time and rotate around them
	// actually transformations are stacked so rotation around z comes first -> that means first rotation is local, others that are following are not
		glRotatef(rotation.y, 0, 1, 0);
		//glRotatef(rotation.y, sin(rotation.z * PI/180), cos(rotation.z * PI/180), 0);
	//if(rotation.z != 0)
		glRotatef(rotation.z, 0, 0, 1);
		
		glRotatef(90, 0, 1, 0);
		*/
	
	//Extract the rotation from the current rotation
    ofVec3f axis;  
    float angle;  
    curRot.getRotate(angle, axis);  
	
	glRotatef(angle, axis.x, axis.y, axis.z); //apply the quaternion's rotation
	glScalef(scaling.x, scaling.y, scaling.z);
	//glScalef(-scaling.x, scaling.y, scaling.z); // right handed
	
    ofSetColor(ofColor::white);
	ofSphere(ofPoint(0,0,0), 15);
	
	for(int i=0; i<NUM_FINGERS; i++) {
		f[i]->draw();
	}


	ofSetColor(ofColor::green);
	//ofLine(f[0]->root.origin, ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, -f[0]->root.origin.z));
	//ofLine(f[0]->root.origin, ofPoint(f[0]->root.origin.x, f[0]->root.origin.y+20, 0));
	
	/*ofLine(ofPoint(f[0]->mid.origin.x, f[0]->mid.origin.y, f[0]->mid.origin.z),
		   f[1]->root.origin);

	ofLine(f[3]->root.origin,
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[3]->root.origin.z));
	ofLine(f[2]->root.origin,
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[2]->root.origin.z));
	*/
	//ofLine(f[1]->root.origin,
	//	   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[1]->root.origin.z));
	ofLine(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[1]->root.origin.z), 
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y+20, 0));
	ofLine(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y+20, 0), 
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[4]->root.origin.z));
	ofLine(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[4]->root.origin.z),
		   f[4]->root.origin);

	// draw hand local coord. system
	ofSetLineWidth(2);
	ofSetColor(ofColor::red);
	ofLine(ofPoint(0,0,0), ofPoint(100,0,0));
	ofSetColor(ofColor::green);
	ofLine(ofPoint(0,0,0), ofPoint(0,100,0));
	ofSetColor(ofColor::blue);
	ofLine(ofPoint(0,0,0), ofPoint(0,0,100));
	ofSetColor(ofColor::white);
	ofPopStyle();
	glPopMatrix();
}

void ofxHandModel::drawProjection() 
{
	glMatrixMode(GL_MODELVIEW);	
	glPushMatrix();

	glTranslatef(IMG_DIM/2, IMG_DIM/2, 0);
	
	//Extract the rotation from the current rotation
    ofVec3f axis;  
    float angle;  
    curRot.getRotate(angle, axis);  
	glRotatef(angle, axis.x, axis.y, axis.z); //apply the quaternion's rotation
	glScalef(scaling.x, scaling.y, scaling.z);
	glScalef(0.5f, 0.5f, 0.5f);
	
	// right handed
	//glScalef(-scaling.x, scaling.y, scaling.z);
	
    //ofSetColor(ofColor::white);
	ofSphere(ofPoint(0,0,0), 15);
	//ofSetLineWidth(1);	
	//ofSphere(origin, 15);

	//glLineWidth(8.0f);
	
	for(int i=0; i<NUM_FINGERS; i++) {
		drawFingerProjection(*f[i]);
	}

	//ofSetColor(ofColor::green);
	//ofLine(f[0]->root.origin, ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, -f[0]->root.origin.z));
	//ofLine(f[0]->root.origin, ofPoint(f[0]->root.origin.x, f[0]->root.origin.y+20, 0));
	ofSetLineWidth(2.0f);
	ofLine(ofPoint(f[0]->mid.origin.x, f[0]->mid.origin.y, f[0]->mid.origin.z),
		   f[1]->root.origin);

	ofLine(f[3]->root.origin,
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[3]->root.origin.z));
	ofLine(f[2]->root.origin,
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[2]->root.origin.z));

	ofLine(f[1]->root.origin,
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[1]->root.origin.z));
	ofLine(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[1]->root.origin.z), 
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y+20, 0));
	ofLine(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y+20, 0), 
		   ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[4]->root.origin.z));
	ofLine(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[4]->root.origin.z),
		   f[4]->root.origin);
	ofSetLineWidth(1.0f);

	glPopMatrix();
}

void ofxHandModel::drawFingerProjection(ofxFingerModel f) {
	glBegin(GL_LINES);

	ofPoint hRootWorld = ofPoint(IMG_DIM/2, IMG_DIM/2, 0);
	ofPoint fRootWorld = getWorldCoord(f.root.origin, hRootWorld);
	hRootWorld.z = -100;
	float zDiff = 255 + (((hRootWorld.z - fRootWorld.z)));
	
	//cout << "DIFF root - f.root" << (hRootWorld.z - fRootWorld.z) << endl;

	glColor3d(zDiff, zDiff, zDiff);
	glVertex3f(f.root.origin.x, f.root.origin.y, f.root.origin.z);

	fRootWorld = getWorldCoord(f.mid.origin, hRootWorld);
	zDiff = 255 - (((hRootWorld.z - fRootWorld.z))+128);
	glColor3d(zDiff, zDiff, zDiff);

	glVertex3f(f.mid.origin.x, f.mid.origin.y, f.mid.origin.z);

	glVertex3f(f.mid.origin.x, f.mid.origin.y, f.mid.origin.z);
	
	fRootWorld = getWorldCoord(f.top.origin, hRootWorld);
	zDiff = 255 - (((hRootWorld.z - fRootWorld.z)));
	glColor3d(zDiff, zDiff, zDiff);
	glVertex3f(f.top.origin.x, f.top.origin.y, f.top.origin.z);

	glVertex3f(f.top.origin.x, f.top.origin.y, f.top.origin.z);
	
	fRootWorld = getWorldCoord(f.fingerTip, hRootWorld);
	zDiff = 255 - (((hRootWorld.z - fRootWorld.z))-128);
	glColor3d(zDiff, zDiff, zDiff);
	glVertex3f(f.fingerTip.x, f.fingerTip.y, f.fingerTip.z);

	glEnd();
}

ofImage ofxHandModel::getProjection(ofPoint _palmCenter, int _kernelSize) {

	ofPoint s = scaling;
	scaling = ofPoint(FIXED_SCALE);

	meshFbo.begin();
	ofClear(0,0,0);
	//ofRect(0,0,IMG_DIM, IMG_DIM);
	drawMesh();
	meshFbo.end();

	scaling = s;

	//meshFbo.draw(IMG_DIM, 0, IMG_DIM, IMG_DIM); // this is not drawn?
	/*ofPixels projPixels;
	meshFbo.readToPixels(projPixels);

	ofImage projImg;
	projImg.setFromPixels(projPixels);
	*/


	//ofFbo resultFbo;
	//resultFbo.allocate(IMG_DIM, IMG_DIM);
	
	//meshFbo.begin();
	dilateFbo.begin();
		ofClear(0,0,0);
		dilateShader.begin();
			dilateShader.setUniformTexture("sampler0", meshFbo.getTextureReference(), 0);
			dilateShader.setUniform1i("kernel_size", _kernelSize);
			meshFbo.draw(_palmCenter.x - IMG_DIM/2, _palmCenter.y - IMG_DIM/2,IMG_DIM, IMG_DIM);
			//ofRect(0,0,0,IMG_DIM, IMG_DIM); // not required in oF.8.1
		dilateShader.end();
	dilateFbo.end();
	//meshFbo.end();
	
	//meshFbo.draw(0,0);
	//resultFbo.draw(0,0);
	

	dilateFbo.readToPixels(projPix);
	projImg.setFromPixels(projPix);

	//resultFbo.draw(0, 0, IMG_DIM, IMG_DIM);
	//resultImg.draw(0, 640, 100, 100);
	
	//projImg.setAnchorPoint(_palmCenter.x, _palmCenter.y);
	//projImg.setAnchorPercent(_palmCenter.x/IMG_DIM, _palmCenter.y/IMG_DIM);

	return projImg;
}

void ofxHandModel::drawFboProjection(ofPoint _position, ofPoint _palmCenter, int _kernelSize) {

	ofPoint s = scaling;
	scaling = ofPoint(FIXED_SCALE);
	
	meshFbo.begin();
	ofClear(0,0,0);
	//ofRect(0,0,IMG_DIM, IMG_DIM);
	drawMesh();
	meshFbo.end();

	
	scaling = s;

	//meshFbo.draw(IMG_DIM, 0, IMG_DIM, IMG_DIM); // this is not drawn?
	/*ofPixels projPixels;
	meshFbo.readToPixels(projPixels);

	ofImage projImg;
	projImg.setFromPixels(projPixels);
	*/

	//ofFbo resultFbo;
	//resultFbo.allocate(IMG_DIM, IMG_DIM);
	
	//meshFbo.begin();
	dilateFbo.begin();
		ofClear(0,0,0);
		dilateShader.begin();
			dilateShader.setUniformTexture("sampler0", meshFbo.getTextureReference(), 0);
			dilateShader.setUniform1i("kernel_size", _kernelSize);
			meshFbo.draw(_palmCenter.x - IMG_DIM/2, _palmCenter.y - IMG_DIM/2,IMG_DIM, IMG_DIM);
			//ofRect(0,0,0,IMG_DIM, IMG_DIM); // not required in oF.8.1
		dilateShader.end();
	dilateFbo.end();
	//meshFbo.end();
	dilateFbo.draw(_position);
}

void ofxHandModel::keyPressed(int key)
{
	//cout << "KEY:" << (char)key << " " << key << endl;

	switch(key) {
		// finger control keys
		case 'q':	f[1]->keyPressed('+');	break;	
		case 'a':	f[1]->keyPressed('-');	break;	

		case 'w':	f[2]->keyPressed('+');	break;	
		case 's':	f[2]->keyPressed('-');	break;	

		case 'e':	f[3]->keyPressed('+');	break;	
		case 'd':	f[3]->keyPressed('-');	break;	

		case 'r':	f[4]->keyPressed('+');	break;	
		case 'f':	f[4]->keyPressed('-');	break;	

		case 't':	f[0]->keyPressed('+');	break;	
		case 'g':	f[0]->keyPressed('-');	break;	

		case 'z':	f[0]->keyPressed('*');	
					/*f[1]->keyPressed('*');*/	break;	

		case 'h':	f[0]->keyPressed('/');	
					/*f[1]->keyPressed('/');*/	break;

		// rotation keys
		case '8':	curRot *= ofQuaternion(1, ofVec3f(1,0,0));	break;
		case '2':	curRot *= ofQuaternion(-1, ofVec3f(1,0,0));	break;
	
		case '4':	curRot *= ofQuaternion(1, ofVec3f(0,1,0));	break;
		case '6':	curRot *= ofQuaternion(-1, ofVec3f(0,1,0));	break;

		case '5':	curRot *= ofQuaternion(1, ofVec3f(0,0,1));	break;
		case '0':	curRot *= ofQuaternion(-1, ofVec3f(0,0,1));	break;
	
		default: break;
	}
}

void ofxHandModel::mouseDragged(int x, int y, int button){
	//every time the mouse is dragged, track the change
	//accumulate the changes inside of curRot through multiplication
    ofVec2f mouse(x,y);  
    ofQuaternion yRot((x-lastMouse.x)*dampen, ofVec3f(0,1,0));  
    ofQuaternion xRot((y-lastMouse.y)*dampen, ofVec3f(-1,0,0));  
    curRot *= yRot*xRot;  
    lastMouse = mouse;  
}

void ofxHandModel::mousePressed(int x, int y, int button){
    //store the last mouse point when it's first pressed to prevent popping
	lastMouse = ofVec2f(x,y);
}

ofPoint ofxHandModel::getIndexFingerWorldCoord()
{
	ofPoint worldCoords;
	ofMatrix4x4 m;
	m.glTranslate(origin.x, origin.y, origin.z);

	//Extract the rotation from the current rotation
    float angle; ofVec3f axis;  
    curRot.getRotate(angle, axis);  
	m.glRotate(angle, axis.x, axis.y, axis.z); //apply the quaternion's rotation 
	m.glScale(scaling.x, scaling.y, scaling.z); // - right hand, + left hand

	worldCoords = m.preMult(f[1]->fingerTip);
	return worldCoords;
}

ofPoint ofxHandModel::getWorldCoord(ofPoint localPoint, ofPoint translateOrigin)
{
	ofPoint worldPoint;
	ofMatrix4x4 m;

	m.glTranslate(translateOrigin.x, translateOrigin.y, translateOrigin.z);

	//Extract the rotation from the current rotation
    float angle; ofVec3f axis;  
    curRot.getRotate(angle, axis);  
	m.glRotate(angle, axis.x, axis.y, axis.z);  //apply the quaternion's rotation
	m.glScale(scaling.x, scaling.y, scaling.z); // - right hand, + left hand

	worldPoint = m.preMult(localPoint);
	return worldPoint;
}

vector<ofPoint> ofxHandModel::getFingerWorldCoord(int index)
{
	// safety, we return index finger coords if index exceeded
	if(index < 0 || index >= NUM_FINGERS)
		index = 1;

	vector<ofPoint> joints;

	ofMatrix4x4 m;
	m.glTranslate(origin.x, origin.y, origin.z);
	
	//Extract the rotation from the current rotation
    float angle; ofVec3f axis; 
    curRot.getRotate(angle, axis);  
	m.glRotate(angle, axis.x, axis.y, axis.z);  //apply the quaternion's rotation
	m.glScale(scaling.x, scaling.y, scaling.z); // - right hand, + left hand

	joints.push_back(m.preMult(f[index]->root.origin));
	joints.push_back(m.preMult(f[index]->mid.origin));
	joints.push_back(m.preMult(f[index]->top.origin));
	joints.push_back(m.preMult(f[index]->fingerTip));

	return joints;
}


vector<ofPoint> ofxHandModel::getFillWorldCoord()
{
	vector<ofPoint> fillPoints;

	ofMatrix4x4 m;
	m.glTranslate(origin.x, origin.y, origin.z);

	//Extract the rotation from the current rotation
    float angle; ofVec3f axis;  
    curRot.getRotate(angle, axis);  
	m.glRotate(angle, axis.x, axis.y, axis.z);  //apply the quaternion's rotation 
	m.glScale(scaling.x, scaling.y, scaling.z); // - right hand, + left hand (x axis?)

	fillPoints.push_back(m.preMult(ofPoint(f[0]->mid.origin.x, f[0]->mid.origin.y, f[0]->mid.origin.z)));
	fillPoints.push_back(m.preMult(f[1]->root.origin));

	fillPoints.push_back(m.preMult(f[3]->root.origin));
	fillPoints.push_back(m.preMult(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[3]->root.origin.z)));

	fillPoints.push_back(m.preMult(f[2]->root.origin));
	fillPoints.push_back(m.preMult(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[2]->root.origin.z)));

	fillPoints.push_back(m.preMult(f[1]->root.origin));
	fillPoints.push_back(m.preMult(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[1]->root.origin.z)));

	fillPoints.push_back(m.preMult(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[1]->root.origin.z)));
	fillPoints.push_back(m.preMult(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y+20, 0)));

	fillPoints.push_back(m.preMult(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y+20, 0)));
	fillPoints.push_back(m.preMult(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[4]->root.origin.z)));
	
	fillPoints.push_back(m.preMult(ofPoint(f[0]->root.origin.x, f[0]->root.origin.y, f[4]->root.origin.z)));
	fillPoints.push_back(m.preMult(f[4]->root.origin));

	return fillPoints;
}

void ofxHandModel::restoreFrom(ofxFingerParameters _localParams, bool _includeAngleX) {
	/* // to be included (setting left, right swing angles)
	f[1]->root.angleX = _localParams.fx1;
	f[2]->root.angleX = _localParams.fx2;
	f[3]->root.angleX = _localParams.fx3;
	f[4]->root.angleX = _localParams.fx4;
	*/
	/*
	f[1]->root.angleZ = _localParams.fz1;
	f[2]->root.angleZ = _localParams.fz2;
	f[3]->root.angleZ = _localParams.fz3;
	f[4]->root.angleZ = _localParams.fz4;

	f[0]->root.angleX = _localParams.tx;
	f[0]->root.angleZ = _localParams.tz;
	*/
	// proper way of setting angles, cause internal update is needed
	// (propagation of angle values towards finger tip segments)
	f[1]->setAngleZ(_localParams.fz1);
	f[2]->setAngleZ(_localParams.fz2);
	f[3]->setAngleZ(_localParams.fz3);
	f[4]->setAngleZ(_localParams.fz4);

	f[0]->setAngleX(_localParams.tx);
	f[0]->setAngleZ(_localParams.tz);

	if (_includeAngleX) {
		f[1]->setAngleX(_localParams.fx1);
		f[2]->setAngleX(_localParams.fx2);
		f[3]->setAngleX(_localParams.fx3);
		f[4]->setAngleX(_localParams.fx4);
	}

	update();
}

ofxFingerParameters	ofxHandModel::saveFingerParameters() {
	ofxFingerParameters p = ofxFingerParameters(f[1]->root.angleZ,
										f[2]->root.angleZ,
										f[3]->root.angleZ,
										f[4]->root.angleZ,
										f[0]->root.angleX,
										f[0]->root.angleZ);
	
	p.fx1 = f[1]->root.angleX;
	p.fx2 = f[2]->root.angleX;
	p.fx3 = f[3]->root.angleX;
	p.fx4 = f[4]->root.angleX;

//	p.params = 0;
/*
// front and back thumb swing
#define THUMB_MIN_ANGLE_Z		   0
#define THUMB_MAX_ANGLE_Z		  20

// define other fingers front and back swing limits (actual angle of first segment, value is then propagated to others)
#define FINGER_MIN_ANGLE_Z	 0
#define FINGER_MAX_ANGLE_Z	90
*/
	// thumb
	if(p.tx < 45 && p.tz < 10)
		p.params += 1;
	if(p.fz1 < 45)
		p.params += 2;
	if(p.fz2 < 45)
		p.params += 4;
	if(p.fz3 < 45)
		p.params += 8;
	if(p.fz4 < 45)
		p.params += 16;
	
	return p;
}
/*
GlobalParameters ofxHandModel::saveGlobalParameters() {
	GlobalParameters p = GlobalParameters();
	return p;
}*/

void ofxHandModel::close(float _factor, short _mask) {
	ofClamp(_factor, 0, 1);
	ofxFingerParameters params = saveFingerParameters(); // so we already have current x angles
	if ((_mask >> 1) & 1) params.fz1 = _factor * (FINGER_MAX_ANGLE_Z - FINGER_MIN_ANGLE_Z) + FINGER_MIN_ANGLE_Z;
	if ((_mask >> 2) & 1) params.fz2 = _factor * (FINGER_MAX_ANGLE_Z - FINGER_MIN_ANGLE_Z) + FINGER_MIN_ANGLE_Z;
	if ((_mask >> 3) & 1) params.fz3 = _factor * (FINGER_MAX_ANGLE_Z - FINGER_MIN_ANGLE_Z) + FINGER_MIN_ANGLE_Z;
	if ((_mask >> 4) & 1) params.fz4 = _factor * (FINGER_MAX_ANGLE_Z - FINGER_MIN_ANGLE_Z) + FINGER_MIN_ANGLE_Z;
	if ((_mask >> 0) & 1) {
		params.tz = _factor * (THUMB_MAX_ANGLE_Z - THUMB_MIN_ANGLE_Z) + THUMB_MIN_ANGLE_Z;
		params.tx = _factor * (THUMB_MIN_ANGLE_X - THUMB_MAX_ANGLE_X) + THUMB_MAX_ANGLE_X;
	}

	interpolate(params, _mask);
}

void ofxHandModel::open(float _factor, short _mask) {
	ofClamp(_factor, 0, 1);
	close(1 - _factor, _mask);
}

void ofxHandModel::spread(float _factor, short _mask) {
	ofClamp(_factor, 0, 1);
	ofxFingerParameters params = saveFingerParameters(); // so we already have current z angles
	if ((_mask >> 1) & 1) params.fx1 = _factor * (FINGER_1_MAX_ANGLE_X - FINGER_1_MIN_ANGLE_X) + FINGER_1_MIN_ANGLE_X;
	if ((_mask >> 2) & 1) params.fx2 = _factor * (FINGER_2_MAX_ANGLE_X - FINGER_2_MIN_ANGLE_X) + FINGER_2_MIN_ANGLE_X;
	if ((_mask >> 3) & 1) params.fx3 = _factor * (FINGER_3_MAX_ANGLE_X - FINGER_3_MIN_ANGLE_X) + FINGER_3_MIN_ANGLE_X;
	if ((_mask >> 4) & 1) params.fx4 = _factor * (FINGER_4_MAX_ANGLE_X - FINGER_4_MIN_ANGLE_X) + FINGER_4_MIN_ANGLE_X;
	if ((_mask >> 0) & 1) {
		//params.tz = _factor * (THUMB_MAX_ANGLE_Z - THUMB_MIN_ANGLE_Z) + THUMB_MIN_ANGLE_Z;
		params.tx = _factor * (THUMB_MAX_ANGLE_X - THUMB_MIN_ANGLE_X) + THUMB_MIN_ANGLE_X;
		params.tz = _factor * (THUMB_MAX_ANGLE_Z - THUMB_MIN_ANGLE_Z) + THUMB_MIN_ANGLE_Z;
		//params.tx = _factor * (THUMB_MIN_ANGLE_X - THUMB_MAX_ANGLE_X) + THUMB_MAX_ANGLE_X;
	}
	
	interpolate(params, _mask);
}

void ofxHandModel::narrow(float _factor, short _mask) {
	ofClamp(_factor, 0, 1);
	spread(1 - _factor, _mask);
}

void ofxHandModel::interpolateParam(float &_desired, float &_prev, float _smooth) {
	_desired = _prev + ((_desired - _prev)*_smooth);
}

// TODO: not functional at the moment 
// (should be realized similarly in a way that is 
// implemented in BezierConnection class - so no timers are used)
void ofxHandModel::interpolate(ofxFingerParameters _to, short _mask) {
	ofxFingerParameters prevParams = saveFingerParameters();
	ofxFingerParameters newParams;
	desiredParams = _to;

	// TODO: in parameters include operator +, - to simplify that kind of operations
	//desiredParams = prevParams + ((desiredParams - prevParams)*0.1f);

	// operators +,-,* wont work properly (yet), this works:
	if ((_mask >> 1) & 1) {	
		interpolateParam(desiredParams.fz1, prevParams.fz1);
		interpolateParam(desiredParams.fx1, prevParams.fx1);
	}
	else {				
		desiredParams.fz1 = prevParams.fz1;
		desiredParams.fx1 = prevParams.fx1;
	}

	if ((_mask >> 2) & 1) {	
		interpolateParam(desiredParams.fz2, prevParams.fz2);
		interpolateParam(desiredParams.fx2, prevParams.fx2);
	}
	else {				
		desiredParams.fz2 = prevParams.fz2;
		desiredParams.fx2 = prevParams.fx2;
	}

	if ((_mask >> 3) & 1) {	
		interpolateParam(desiredParams.fz3, prevParams.fz3);
		interpolateParam(desiredParams.fx3, prevParams.fx3);
	}
	else {				
		desiredParams.fz3 = prevParams.fz3;
		desiredParams.fx3 = prevParams.fx3;
	}

	if ((_mask >> 4) & 1) {	
		interpolateParam(desiredParams.fz4, prevParams.fz4);
		interpolateParam(desiredParams.fx4, prevParams.fx4);
	}
	else {				
		desiredParams.fz4 = prevParams.fz4;
		desiredParams.fx4 = prevParams.fx4;
	}
	
	if ((_mask >> 0) & 1)  {	
		interpolateParam(desiredParams.tz, prevParams.tz);
		interpolateParam(desiredParams.tx, prevParams.tx);
	}
	else {
		desiredParams.tz = prevParams.tz;
		desiredParams.tx = prevParams.tx;
	}
	
	restoreFrom(desiredParams, true);

	//rollAngle = prevRollAngle + ((rollAngle - prevRollAngle)*0.5f); // smoothing
}

void ofxHandModel::setScale(float _factor) {
	scaling.set(_factor);
}

void ofxHandModel::setScale(ofPoint _scale) {
	scaling.set(_scale);
}

ofPoint& ofxHandModel::getScaleRef() {
	return scaling;
}

ofPoint ofxHandModel::getScale() {
	return scaling;
}

void ofxHandModel::setOrigin(ofPoint _origin) {
	origin.set(_origin);
}

ofPoint& ofxHandModel::getOriginRef() {
	return origin;
}

ofPoint ofxHandModel::getOrigin() {
	return origin;
}

void ofxHandModel::setRotation(ofQuaternion _rotation) {
	curRot.set(_rotation);
}

ofQuaternion& ofxHandModel::getRotationRef() {
	return curRot;
}

ofQuaternion ofxHandModel::getRotation() {
	return curRot;
}