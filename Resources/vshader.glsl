#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 viewProjection;
uniform mat4 normalizeModel;
uniform mat4 modelMatrix;

attribute vec4 a_position;
attribute vec2 a_texcoord;
attribute vec3 a_normal;
attribute vec3 a_polyNorm;
attribute vec3 a_polyTan;
attribute vec3 a_polyBiTan;

varying vec2 v_surfaceUV;
varying vec3 v_surfacePosition;
varying vec3 v_surfaceNormal;
varying vec3 v_polyNorm;
varying vec3 v_polyTan;
varying vec3 v_polyBiTan;

void main()
{
	// Calculate vertex position in screen space
	gl_Position = viewProjection * normalizeModel * modelMatrix * a_position;

	// Pass data to fragment shader
	// Value will be automatically interpolated to fragments inside polygon faces
	v_surfaceUV = a_texcoord;
	v_surfacePosition = vec3(normalizeModel * modelMatrix * a_position);
	v_surfaceNormal = a_normal;
	v_polyNorm = a_polyNorm;
	v_polyTan = a_polyTan;
	v_polyBiTan = a_polyBiTan;
}
