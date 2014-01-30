// dilation.fs
//
// maximum of 3x3 kernel

uniform sampler2DRect sampler0;
//uniform vec2 mouse; // if testing with mouse
uniform int kernel_size;

// useful random function?
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{	/* // alter fragments only around mouse coords
	float dist = distance(gl_FragCoord.xy, mouse); 
	float radius = 240.0;
	
	if(dist < radius) {
		int size = kernel_size;
		vec4 maxValue = vec4(0.0, 0.0, 0.0, 0.0);
		vec4 currValue;
		for (int i = -size; i<=size; i++) {
			for (int j = -size; j<=size; j++) {
				currValue = texture2DRect(sampler0, gl_TexCoord[0].st + vec2(float(i), float(j)));
				maxValue = max(currValue, maxValue);
			}
		}
		gl_FragColor = maxValue* (1.4 * (1 - dist/radius));
	}
	else {
		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	}
	*/
	
	// run dilation on each fragment 
	int size = kernel_size;
	vec4 maxValue = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 currValue;
	for (int i = -size; i<=size; i++) {
		for (int j = -size; j<=size; j++) {
			float dist = distance(vec2(0.0, 0.0), vec2(i, j)); 
			float radius = float(size);
			if(dist <= radius) { // exclude corners of the kernel
				currValue = texture2DRect(sampler0, gl_TexCoord[0].st + vec2(float(i), float(j)));
				maxValue = max(currValue, maxValue);
			}
		}
	}
	gl_FragColor = maxValue;
	//gl_FragColor.x = gl_FragDepth/10;
	//gl_FragColor = vec4(maxValue.x, maxValue.y+gl_FragCoord.z, maxValue.z);
}
