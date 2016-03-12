// Attribute Variables
attribute vec3 v_position; // This gets the position from the main
attribute vec2 v_texture; // This gets the x y coordinates for the texture
attribute vec3 v_normals; // Normals from the .obj file

// Uniform Variables
uniform mat4 mvpMatrix;
uniform mat4 model;
uniform mat4 view;

// Varying variables
varying vec2 texture; //Output texture
varying vec3 N; // Normal that will be passed into the fragment shader
varying vec3 l_Position;
varying vec3 model_fragPos;


varying vec3 vv;
void main(void)
{
	// Normalized, calculates the N, essentially the inverse multiplied by each and every
	// normal coming in from the main, so it should be a single object on the ground with 
	// normal vectors passed in different directions
	N = normalize( (view * model*vec4(v_normals,0)).xyz);

	// Holds the vector for E which is the eye space, 
	vv = vec3(mvpMatrix * vec4(v_position, 1.0f));

	// Calculates the models position to calculate the distance for the pointLight
	model_fragPos = vec3(model * vec4(v_position, 1.0f));

	// Calculates gl_Position to be passed
	gl_Position = mvpMatrix * vec4(v_position, 1.0);
	texture = v_texture;	// Passes texture to frag
	l_Position= v_position; // Passes v_position to frag
}
