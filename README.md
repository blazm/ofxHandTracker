ofxHandTracker
===============

Introduction
---------------
A Hand Tracking Addon using Kinect for openFrameworks (early alpha):

ofxHandTracker is an addon for openFrameworks (version 073+) that demonstrates hand tracking using Kinect Sensor.
It uses some other addons. These are ofxOpenCV and ofxOpenNI. To be able to run most of it, also Kinect sensor and drivers for it are needed. Also some parts are using shaders, so decent graphics accelerator is needed.
Some examples and demo projects included. They show features and demo presentation. At the moment only project files for MS VS are generated. Later on also XCode project files will be added.

This addon is result of my Bachelor thesis project at Faculty of Computer and Information Science, Ljubljana, Slovenia. Currently it comes in very early alpha version.
Beacuse of fast development some parts of code are not properly structured, may be repeated, or are not optimally written, etc. In the future the addon structure will most likely change - also its functionality will be improved. 
Main issue is still about robustness of hand tracking and its alignment with kinematic model. 

Latest updates are available in branch 0.8.1.

Requirements
--------------
ofxOpenNI (also install OpenNI, NITE), SensorKinect drivers: <br/>
https://github.com/gameoverhack/ofxOpenNI <br/>
https://github.com/avin2/SensorKinect

License
--------------
The code in this repository is available under the [CC BY-SA 2.5 License] (http://creativecommons.org/licenses/by-sa/2.5/)

Images (to be updated in the future)
--------------
![ofxAddons Thumbnail](ofxaddons_thumbnail.png?raw=true "ofxAddons thumbnail")
