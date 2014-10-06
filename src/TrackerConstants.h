#pragma once

namespace ofxHT {
	namespace Const {

		// golden ratio
		static float _GOLDEN_RATIO = 1.618f; //033988749894848204586834;
		static float _GOLDEN_RATIO_x2 = _GOLDEN_RATIO * _GOLDEN_RATIO;
		static float _GOLDEN_RATIO_x3 = _GOLDEN_RATIO_x2 * _GOLDEN_RATIO;

		// finger side and forward angles
		// left and right thumb swing
		static float THUMB_MIN_ANGLE_X = -45;//-30 -> less opened
		static float THUMB_MAX_ANGLE_X = 0; //0 

		// front and back thumb swing
		static float THUMB_MIN_ANGLE_Z = 0; // 0
		static float THUMB_MAX_ANGLE_Z = 37.5; // 20

		// define other fingers front and back swing limits (actual angle of first segment, value is then propagated to others)
		static float FINGER_MIN_ANGLE_Z	= 0;
		static float FINGER_MAX_ANGLE_Z	= 90;

		// also need to define non-thumb finer x angles -> but they are different for each finger
		static float FINGER_MIN_ANGLE_X = 0; // legacy, remove soon
		static float FINGER_MAX_ANGLE_X	= 10; // legacy, remove soon

		static float FINGER_1_MIN_ANGLE_X = -5;
		static float FINGER_2_MIN_ANGLE_X = -1.5;
		static float FINGER_3_MIN_ANGLE_X = 2.5;
		static float FINGER_4_MIN_ANGLE_X = 5;

		static float FINGER_1_MAX_ANGLE_X = 25;
		static float FINGER_2_MAX_ANGLE_X = 7.5;
		static float FINGER_3_MAX_ANGLE_X = -12.5;
		static float FINGER_4_MAX_ANGLE_X = -25;

		// image dimensions
		/*
		#define IMG_DIM	500.0 
		#define FIXED_SCALE 0.9
		#define FIXED_KERNEL 10
		*//*
		static const float IMG_DIM = 250.0;
		static const float FIXED_SCALE = 0.45;
		static const int FIXED_KERNEL = 6;
		*/
		/*
		static const float IMG_DIM = 64.0;
		static const float FIXED_SCALE = 0.16;
		static const int FIXED_KERNEL = 2;
		*/

		static const float IMG_DIM = 150.0;
		static const float FIXED_SCALE = 0.3;
		static const int FIXED_KERNEL = 4; 
		/*
		#define IMG_DIM	100.0 
		#define FIXED_SCALE 0.2 
		#define FIXED_KERNEL 2 
		/*
		#define IMG_DIM	50.0
		#define FIXED_SCALE 0.1 
		#define FIXED_KERNEL 0
		*/

		// miscellaneous
		static const int NUM_FINGERS = 5;

		static const int TINY_IMG_DIM =	150;
		static const int FTIP_HIST_SIZE	= 3; // fingertip history array size

/*		// not used
		#define	HAND_GRABBED			"hand_grabbed"
		#define HAND_RELEASED			"hand_released"
		#define HAND_PICKED				"hand_picked"
*/
		static const float MIN_HAND_DEPTH = 500.0;
		static const float MAX_HAND_DEPTH = 2000.0;
	}
}