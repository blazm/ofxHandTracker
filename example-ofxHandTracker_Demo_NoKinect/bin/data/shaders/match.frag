// match.frag (calcs abs diff image matching in squared regions) - by blaz

uniform sampler2DRect sampler0;
//uniform vec2 mouse; // if testing with mouse
uniform int kernel_width;
uniform int kernel_height;

uniform int frag_width;
uniform int frag_height;

//uniform vec2i kernel_size; // if we could use this would be better (but it wont work on with i)

void main(void)
{	
/*---------------------------------------------------------------------------------------------------------
	//float kernel_w_f = float(kernel_width);
	//float kernel_h_f = float(kernel_width);
	
	vec2 kernel_size_f = vec2(float(kernel_width), float(kernel_height));
	
	vec2 frag_coord = gl_TexCoord[0].st;
	
	float max_val = kernel_width * kernel_height; // max value for later normalization
	vec4 maxValue = vec4(max_val, max_val, max_val, max_val);
	
	int scaled_kernel_width = kernel_width * frag_width;
	int scaled_kernel_height = kernel_height * frag_height;
	
	// run kernel sum operation only in %kernel == 0 regions
	if((int(frag_coord.x) + scaled_kernel_width/2)%(scaled_kernel_width) == 0 && (int(frag_coord.y) + kernel_height/2)%(scaled_kernel_height) == 0) {
	
		int size_w = kernel_width/2;
		int size_h = kernel_height/2;
		
		vec4 sumValue = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 currValue;
		
		for (int i = -size_w; i<size_w; i++) {
			for (int j = -size_h; j<size_h; j++) {
				currValue = texture2DRect(sampler0, gl_TexCoord[0].st + vec2(float(i*frag_width), float(j*frag_width)));		
				sumValue = sumValue + currValue;
			}
		}
		
		sumValue = sumValue / maxValue;
		sumValue = vec4(sumValue.x, 1.0 - sumValue.x, 0.0, 1.0); // debuging
		gl_FragColor = sumValue;
	}
	else {
		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	}
---------------------------------------------------------------------------------------------------------*/

	vec2 kernel_size_f = vec2(float(kernel_width), float(kernel_height));
	
	vec2 frag_coord = gl_TexCoord[0].st;
	
	float max_val = kernel_width * kernel_height; // max value for later normalization
	vec4 maxValue = vec4(max_val, max_val, max_val, max_val);
	
	int scaled_kernel_width = kernel_width * frag_width;
	int scaled_kernel_height = kernel_height * frag_height;
	
	// run kernel sum operation only in %kernel == 0 regions
	if((int(frag_coord.x)) % (kernel_width) == 0 && (int(frag_coord.y)) % (kernel_height) == 0) {
	
		int size_w = kernel_width;
		int size_h = kernel_height;
		
		vec4 sumValue = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 currValue;
		
		//int maxVal = 1;
		
		for (int i = 0; i<size_w; i++) {
			for (int j = 0; j<size_h; j++) {
				currValue = texture2DRect(sampler0, gl_TexCoord[0].st + vec2(float(i * frag_width), float(j * frag_height)));	

				//if (currValue.x > 0.01 || currValue.y > 0.01 || currValue.z > 0.01) {
				//	maxVal++;
					sumValue = sumValue + currValue;
				//}
			}
		}
		
		sumValue = sumValue / maxValue;
		//sumValue = vec4(sumValue.x, 1.0 - sumValue.x, 0.0, 1.0); // debuging
		gl_FragColor = sumValue;
	}
	else {
		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	}
}
