#version 420

layout(std140, binding = 2) uniform LightScene
{
	uniform vec3 uSceneAmbient;
};

layout(std140, binding = 3) uniform Light
{
	uniform vec3 uLightColor;
	uniform vec3 uLightPosition;
};

uniform float uMaterialSpecularExponent = 16.0;

struct vData
{
	vec2 texcoord;
	vec3 norm;
	vec3 pos;
};
layout(location = 0) in vData o;

out vec4 outColor;

void main()
{
	outColor.rgb = uSceneAmbient; 

	// Fix length after rasterizer interpolates
	vec3 normal = normalize(o.norm);

	vec3 lightVec = uLightPosition - o.pos;
	float dist = length(lightVec);
	vec3 lightDir = lightVec / dist;

	float NdotL = dot(normal, lightDir);

	// If the normal is facing the light
	if (NdotL > 0.0)
	{
		// Normalized vector pointing towards the camera
		vec3 eye = normalize(-o.pos);
		
		// Calculate attenuation (falloff)
		// Add a small number to avoid divide by zero.
		float attenuation = 1.0 / (1.0 + dist * dist * 0.1);

		// Calculate the diffuse contribution
		outColor.rgb += uLightColor * NdotL * attenuation;
		
		vec3 reflection = reflect(-lightDir, normal);
		
		float specularStrength = dot(reflection, eye);
		specularStrength = max(specularStrength, 0.0f); // don't let it fall before zero

		// Calculate the specular contribution
		outColor.rgb += uLightColor * pow(specularStrength, uMaterialSpecularExponent) * attenuation;
	}
}
