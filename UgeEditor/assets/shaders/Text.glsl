/***********************
 *   MSDF Text Shader  *
************************/

/***********************
 *   Vertex Shader     *
************************/
#type vertex
#version 450 core
				
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int a_EntityID;



layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;

};
layout (location = 0) out VertexOutput Output;
layout (location = 2) out flat int v_EntityID;

			
void main()
{
	Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;
	v_EntityID = a_EntityID;
	gl_Position = u_ViewProjection  * vec4(a_Position, 1.0);

				
}

/*********************
 * Fragment Shader   *
**********************/
#type fragment
#version 450 core
				
layout(location = 0) out vec4 fragColor;
layout(location = 1) out int entityID;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
};

layout (location = 0) in VertexOutput Input;
layout (location = 2) in flat int v_EntityID;



layout (binding = 0) uniform sampler2D u_FontAtlas;

vec4 getTexColor()
{

	vec4 texColor = Input.Color * texture(u_FontAtlas,   Input.TexCoord);
	

	return texColor;

}

float screenPxRange() {
	const float pxRange = 2.0; // set to distance field's pixel range
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_FontAtlas, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(Input.TexCoord);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

// The scene target is linear RGBA16F and the resolve pass sRGB-encodes it once for the whole
// frame, so text colours have to be decoded here or they are encoded twice and come out
// washed out. @see Uge::PostProcess
vec3 SrgbToLinear(vec3 color)
{
	vec3 low = color / 12.92;
	vec3 high = pow((color + 0.055) / 1.055, vec3(2.4));
	return mix(low, high, step(vec3(0.04045), color));
}

void main()
{
	
	vec4 texColor = Input.Color * texture(u_FontAtlas, Input.TexCoord);

	vec3 msd = texture(u_FontAtlas, Input.TexCoord).rgb;
    float sd = median(msd.r, msd.g, msd.b);
    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
	if (opacity == 0.0)
		discard;

	vec4 bgColor = vec4(0.0);
    fragColor = mix(bgColor, Input.Color, opacity);
	if (fragColor.a == 0.0)
		discard;

	// Alpha carries the glyph coverage and is not gamma encoded, so only rgb is decoded.
	fragColor.rgb = SrgbToLinear(fragColor.rgb);
	
	entityID = v_EntityID;

	
}