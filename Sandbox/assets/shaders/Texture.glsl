// Texture Shader
#type vertex
#version 330 core
				
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 1) in vec2 a_TextCoord;

uniform mat4 u_ViewProjection;


out vec2 v_TextCoord;
out vec4 v_Color;

			
void main()
{
	v_TextCoord = a_TextCoord;
	v_Color = a_Color;
	gl_Position = u_ViewProjection  * vec4(a_Position, 1.0);

				
}


#type fragment
#version 330 core 
				
layout(location = 0) out vec4 fragColor;

in vec4 v_Color;
in vec2 v_TextCoord;

uniform vec4 u_Color;
uniform float u_TilingFactor;
uniform sampler2D u_Texture;
			
void main()
{
	// TODO: Add variables so that for instance "texture(u_Texture, v_TextCoord * 1.0) * vec4(color)" could work
	//fragColor = texture(u_Texture, v_TextCoord * u_TilingFactor) * u_Color;
	fragColor = v_Color;
}