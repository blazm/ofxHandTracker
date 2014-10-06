#pragma once

#include "ofMain.h"

namespace ofxHT {

class ImageMatcher
{
	//public:
	//	ImageMatcher(void);
	//	~ImageMatcher(void);

	public:
		ImageMatcher(void) {
			setup();
		}
		ImageMatcher(int _w, int _h){
			setup(_w, _h);
		}

		~ImageMatcher(void) {};
    
		ofImage imagesAbsDiff(ofImage _image1, ofImage _image2) {
			img1.setFromPixels(_image1.getPixelsRef());
			img2.setFromPixels(_image2.getPixelsRef());

			absDiffFbo.begin();
				absDiffShader.begin(); // freakin abs diff shader wont work, multiple textures are not acessed properly (second one)
					absDiffShader.setUniformTexture("tex1", img1.getTextureReference(), 1);
					absDiffShader.setUniformTexture("tex2", img2.getTextureReference(), 2);
					//absDiffShader.setUniform2f("mouse", ofGetMouseX(), ofGetHeight() - ofGetMouseY());
		
					//ofRect(0,0, img1.width, img1.height);
					img1.draw(0,0);
					//img2.draw(0,0);

				absDiffShader.end();
			absDiffFbo.end();
            
            absDiffFbo.readToPixels(resultPixels);
            resultImage.setFromPixels(resultPixels);

			return resultImage;
		}

		float matchImages(ofImage _image1, ofImage _image2) {
            return matchImage(imagesAbsDiff(_image1, _image2));
		}

        float matchImage(ofImage _image) {
            
			int kernel_w = 4;
			int kernel_h = 4;

			matchFbo.begin();
			ofClear(0, 0, 0, 0);
			matchShader.begin();
				matchShader.setUniformTexture("sampler0", _image.getTextureReference(), 3);
				matchShader.setUniform1i("kernel_width", kernel_w); // set constant kernel width
				matchShader.setUniform1i("kernel_height", kernel_h);
				// now working right now
				//matchShader.setUniform2i("kernel_size", 5, 5); // w, h of kernel
				matchShader.setUniform1i("frag_width", 1); // neighbour fragment distance (w,h)
				matchShader.setUniform1i("frag_height", 1 );

				_image.draw(0, 0);
			matchShader.end();
			matchFbo.end();

		matchFbo.readToPixels(resultPixels);
		resultImage.setFromPixels(resultPixels);

		    ofSetColor(255);
						ofNoFill();
			ofRect(150-1, 150-1, resultImage.width, resultImage.height);
			ofFill();
			resultImage.draw(150, 150);


		float all = 0;
		float max = (resultImage.width/kernel_w) * (resultImage.height/kernel_h) * 255;
		int maxCount = 1;

		for(int i=0; i<resultImage.width; i+=kernel_w)
			for(int j=0; j<resultImage.height; j+=kernel_h){
				float b = resultImage.getColor(i, j).getBrightness();
				all += b;
				if (b > 0.001f) {
					maxCount++;
				}
			}

		//cout << " matching: " << all/(float)(maxCount) << endl;

		return all/(float)(maxCount);
        }

		void setup(int _w = 300, int _h = 300){
			absDiffShader.load("shaders/absdiff");
            matchShader.load("shaders/match");

			//TODO: change 300 to some params
			img1.allocate(_w, _h, OF_IMAGE_COLOR_ALPHA);
			img2.allocate(_w, _h, OF_IMAGE_COLOR_ALPHA);
			absDiffFbo.allocate(_w, _h);
			matchFbo.allocate(_w, _h);
			
			resultImage.allocate(_w, _h, OF_IMAGE_COLOR_ALPHA);
			resultPixels.allocate(_w, _h, OF_IMAGE_COLOR_ALPHA);
        };
    
    private:
		// abs diff section
		ofShader	absDiffShader;
		ofImage		img1;
		ofImage		img2;
		ofFbo		absDiffFbo;

		// match section
        ofShader    matchShader;
		ofFbo		matchFbo;
    
		// results
		ofPixels	resultPixels;
		ofImage		resultImage;
};

}