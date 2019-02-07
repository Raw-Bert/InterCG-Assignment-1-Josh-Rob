#version 420

uniform sampler2D tex;
uniform sampler3D lutTexture;

in vec2 texCoordVarying;
out vec4 outColor;

uniform float lutSize;
uniform float AMOUNT;

void main()
{
	vec3 rawColor = texture2D(tex,texCoordVarying).rgb;
	
	vec3 scale = vec3((lutSize -1.0) / lutSize);
	vec3 offset = vec3(1.0/(2.0 * lutSize));
	
	vec3 applyLut = texture(lutTexture, scale * rawColor + offset).rgb;
	
	vec3 finalColor = mix(rawColor, applyLut, AMOUNT);
	
	outColor = vec4(finalColor,1.0);

}