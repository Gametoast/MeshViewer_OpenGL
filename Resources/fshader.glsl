#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat3 normalMatrix;
uniform vec3 cameraPosition;

uniform sampler2D tx0;
uniform sampler2D tx1;

uniform struct Material {
	float shininess;
	vec3 specularColor;
	bool isTransparent;
	bool hasSpecularmap;
	bool hasNormalmap;
	bool isGlow;
} material;

uniform bool useLight;

uniform struct Light {
	vec4 position;
	vec3 intensities;
	float attenuationFactor;
	float ambientCoefficient;
} light;

varying vec2 v_surfaceUV;
varying vec3 v_surfacePosition;
varying vec3 v_surfaceNormal;
varying vec3 v_polyNorm;
varying vec3 v_polyTan;
varying vec3 v_polyBiTan;

void main()
{
	if(useLight && !material.isGlow)
	{
		// get the color and undo gamma correction
		vec4 surfaceColor = vec4(texture2D(tx0, v_surfaceUV));
		surfaceColor.rgb = pow(surfaceColor.rgb, vec3(2.2));

		// attenutation depending on the distance to the light
		float distanceToLight = length(light.position.xyz - v_surfacePosition);
		float attenuation = 1.0 / (1.0 + light.attenuationFactor * pow(distanceToLight, 2));

		// normal vector
		vec3 normal = normalize(normalMatrix * v_surfaceNormal);

		// direction from surface to light depending on the light type
		vec3 surfaceToLight;
		if(light.position.w == 0.0)		// directional light
			surfaceToLight = normalize(light.position.xyz);
		else							// point light
			surfaceToLight = normalize(light.position.xyz - v_surfacePosition);

		// direction from surface to camera
		vec3 surfaceToCamera = normalize(cameraPosition - v_surfacePosition);

		// adjust the values if material has normal map
		if(material.hasNormalmap)
		{
			vec3 surfaceTangent = normalize(normalMatrix * v_polyTan);
			vec3 surfaceBitangent = normalize(normalMatrix * -v_polyBiTan);
			vec3 surfaceNormal = normalize(normalMatrix * v_surfaceNormal);
			mat3 tbn = transpose(mat3(surfaceTangent, surfaceBitangent, surfaceNormal));
			normal = texture2D(tx1, v_surfaceUV).rgb;
			normal = normalize(normal * 2.0 -1.0);
			surfaceToLight = normalize(tbn * surfaceToLight);
			surfaceToCamera = normalize(tbn * surfaceToCamera);
		}


	/////////////////////////////////////////////////////////////////////////////////////
	// ambient component

		vec3 ambient = light.ambientCoefficient * surfaceColor.rgb * light.intensities;


	/////////////////////////////////////////////////////////////////////////////////////
	// diffuse component

		float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
		vec3 diffuse = diffuseCoefficient * surfaceColor.rgb * light.intensities;


	/////////////////////////////////////////////////////////////////////////////////////
	// specular component

		float specularCoefficient = 0.0;
		if(diffuseCoefficient > 0.0)
			specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), material.shininess);
		
		float specularWeight = 1;
		if(material.hasSpecularmap)
			specularWeight = surfaceColor.a;
		vec3 specColor = specularWeight * 1/255 * material.specularColor;

		vec3 specular = specularCoefficient * specColor * light.intensities;

	/////////////////////////////////////////////////////////////////////////////////////
	// linear color before gamma correction
		vec3 linearColor = ambient + attenuation * (diffuse + specular);

	/////////////////////////////////////////////////////////////////////////////////////
	// gama correction
		vec3 gamma = vec3(1.0/2.2);

		if(!material.isTransparent)
			surfaceColor.a = 1.0;
		
		gl_FragColor = vec4(pow(linearColor, gamma), surfaceColor.a);
	}
	// don't use light
	else
	{
		vec4 surfaceColor = vec4(texture2D(tx0, v_surfaceUV));

		if(!material.isTransparent)
			surfaceColor.a = 1.0;

		gl_FragColor = surfaceColor;
	}
}
