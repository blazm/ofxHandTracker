#pragma once

#include "ofMain.h"

#include "TrackerConstants.h"

namespace ofxHT { // TODO: add structured namespaces, remove macro defines
	using namespace Const;

// used to store local finger parameters
class FingerParameters
{
	public:
		FingerParameters() {
			fx1 = 0;
			fx2 = fx1;
			fx3 = fx2;
			fx4 = fx3;

			fz1 = FINGER_MAX_ANGLE_Z;
			fz2 = FINGER_MAX_ANGLE_Z;
			fz3 = FINGER_MAX_ANGLE_Z;
			fz4 = FINGER_MAX_ANGLE_Z;

			params = 0;

			tx = 0; 
			tz = 0;
		};

		FingerParameters(float _fx1, float _fx2, float _fx3, float _fx4, float _tx){
			fx1 = _fx1;
			fx2 = _fx2;
			fx3 = _fx3;
			fx4 = _fx4;
			tx = _tx;
			tz = 0;
			
			fz1 = 0;
			fz2 = fz1;
			fz3 = fz2;
			fz4 = fz3;

			params = 0;

			clampParams();
		};

		FingerParameters(float _fz1, float _fz2, float _fz3, float _fz4, float _tx, float _tz){
			fz1 = _fz1;
			fz2 = _fz2;
			fz3 = _fz3;
			fz4 = _fz4;
			tx = _tx;
			tz = _tz;

			fx1 = 0;
			fx2 = fx1;
			fx3 = fx2;
			fx4 = fx3;

			params = 0;

			clampParams();
		};

		FingerParameters(int _fz1, int _fz2, int _fz3, int _fz4, int _tx, int _tz){
			fz1 = _fz1;
			fz2 = _fz2;
			fz3 = _fz3;
			fz4 = _fz4;
			tx = _tx;
			tz = _tz;

			fx1 = 0;
			fx2 = fx1;
			fx3 = fx2;
			fx4 = fx3;

			params = 0;

			clampParams();
		};
		//~FingerParameters(void){};

		FingerParameters(int _params) {
			/*fx1 = 0;
			fx2 = fx1;
			fx3 = fx2;
			fx4 = fx3;*/

			fx1 = FINGER_MAX_ANGLE_X*2;
			fx2 = 0;
			fx3 = FINGER_MIN_ANGLE_X;
			fx4 = FINGER_MIN_ANGLE_X*2;

			fz1 = FINGER_MAX_ANGLE_Z;
			fz2 = FINGER_MAX_ANGLE_Z;
			fz3 = FINGER_MAX_ANGLE_Z;
			fz4 = FINGER_MAX_ANGLE_Z;

			tx = 0; 
			tz = 0;
			
			params = _params;

			//params = params & 0x11111;

			//cout << "INPUT PARAMS: " << params << endl;

			int value = 0;
			int index = 0;

			int _temp_params = params;

			while(index < 5) {
				value = _temp_params & 0x1;
				states[index] = value;
				//cout << "PARAMS: " << params << " INDEX: " << index << " VALUE: " << value << endl;

				_temp_params = _temp_params >> 1; //params / 2;
				index++;
			}

			// binary setting finger parameters 
			// (only open/closed value)
			if(states[1]) 	fz1 = FINGER_MIN_ANGLE_Z;
			else			fz1 = FINGER_MAX_ANGLE_Z;
	
			if(states[2])   fz2 = FINGER_MIN_ANGLE_Z;
			else 			fz2 = FINGER_MAX_ANGLE_Z;

			if(states[3]) 	fz3 = FINGER_MIN_ANGLE_Z;
			else			fz3 = FINGER_MAX_ANGLE_Z;
	
			if(states[4])   fz4 = FINGER_MIN_ANGLE_Z;
			else 			fz4 = FINGER_MAX_ANGLE_Z;

			// thumb binary setting
			// TODO: test if min/max combination written correctly
			if(states[0]) { 
				tx = THUMB_MAX_ANGLE_X;
				tz = THUMB_MIN_ANGLE_Z;
			}
			else { 
				tx = THUMB_MIN_ANGLE_X;
		  		tz = THUMB_MAX_ANGLE_Z; 
			}

			/*if(states[0]) { 
				tx = THUMB_MAX_ANGLE_X;
				tz = THUMB_MAX_ANGLE_Z;
			}
			else { 
				tx = THUMB_MIN_ANGLE_X;
		  		tz = THUMB_MIN_ANGLE_Z; 
			}*/
		}

		/*ostream& operator<<(ostream& os)
		{
			os << "Z: " << fz1 << ',' << fz2 << ',' << fz3 << ',' << fz4 << ',' << '\n' << endl;
			return os;
		} 
		
		ofLog& operator<<(ofLog& os)
		{
			os << "Z: " << fz1 << ',' << fz2 << ',' << fz3 << ',' << fz4 << ',' << '\n' << endl;
			return os;
		} */

		FingerParameters operator+(const FingerParameters& other);
		FingerParameters operator-(const FingerParameters& other);
		FingerParameters operator*(const float factor);
       // FingerParameters& operator=(const FingerParameters& other);

		float fz1, fz2, fz3, fz4; 
		float fx1, fx2, fx3, fx4; // left, right finger rotation params - UNUSED
		float tx, tz;
		//float hx, hy, hz;

		float fz[NUM_FINGERS];
		float fx[NUM_FINGERS];

		// merged from DiscreteLocalParams;
		// TODO: later better to organize angles (fx*, fz*) as
		// arrays and directly acess them without states array
		// that way we simplify many code lines with loops
		int params;
		bool states[5];

	private:
		void clampParams() {
			// here we should do safety clamping
			fz1 = ofClamp(fz1, FINGER_MIN_ANGLE_Z, FINGER_MAX_ANGLE_Z); 
			fz2 = ofClamp(fz2, FINGER_MIN_ANGLE_Z, FINGER_MAX_ANGLE_Z); 
			fz3 = ofClamp(fz3, FINGER_MIN_ANGLE_Z, FINGER_MAX_ANGLE_Z); 
			fz4 = ofClamp(fz4, FINGER_MIN_ANGLE_Z, FINGER_MAX_ANGLE_Z); 

			fx1 = ofClamp(fx1, FINGER_MIN_ANGLE_X, FINGER_MAX_ANGLE_X); 
			fx2 = ofClamp(fx2, FINGER_MIN_ANGLE_X, FINGER_MAX_ANGLE_X); 
			fx3 = ofClamp(fx3, FINGER_MIN_ANGLE_X, FINGER_MAX_ANGLE_X); 
			fx4 = ofClamp(fx4, FINGER_MIN_ANGLE_X, FINGER_MAX_ANGLE_X); 
		};
};



// used to store global rotation - TODO
class ofxPalmParameters
{
public:
	ofxPalmParameters(float _ax, float _ay, float _az){
		ax = _ax;
		ay = _ay;
		az = _az;
	};

	float ax, ay, az; // euler rotation angles
};

}