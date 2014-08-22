varying float DEPTH;
uniform float FARPLANE;  // send this in as a uniform to the shader

void main() {
	//gl_TexCoord[0] = gl_MultiTexCoord0;
	//gl_Position = ftransform();
	
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	DEPTH = gl_Position.z / FARPLANE ; // do not divide by w
} 
