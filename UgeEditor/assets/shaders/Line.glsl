/*********************
 * Line Shader    *
**********************/
#type vertex
#version 450 core
				
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in int a_EntityID;



layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};


layout (location = 0) out vec4 v_Color;
layout (location = 1) out flat int v_EntityID;

			
void main()
{
	v_Color = a_Color;
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

layout (location = 0) in vec4 v_Color;
layout (location = 1) in flat int v_EntityID;





// The scene target is linear RGBA16F and the resolve pass sRGB-encodes it once for the whole
// frame, so this shader has to hand it linear values. Sprite textures and the vertex colours
// they are tinted by are both authored in sRGB; without this decode they would be encoded a
// second time at resolve and the 2D layer would come out washed out.
//
// The decode is applied to the finished colour rather than to each factor. That is the exact
// inverse of what the resolve does, which is what keeps the 2D path pixel-identical to how it
// looked before the HDR target existed. @see Uge::PostProcess
vec3 SrgbToLinear(vec3 color)
{
	vec3 low = color / 12.92;
	vec3 high = pow((color + 0.055) / 1.055, vec3(2.4));
	return mix(low, high, step(vec3(0.04045), color));
}

void main()
{

	fragColor = vec4(SrgbToLinear(v_Color.rgb), v_Color.a);
	entityID  = v_EntityID;

}