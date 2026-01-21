// Texture Shader
#type vertex
#version 330 core
				
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TextCoord;

uniform mat4 u_ViewProjection;


out vec2 v_TextCoord;
out vec4 v_Color;
			
void main()
{
	v_TextCoord = a_TextCoord;
	gl_Position = u_ViewProjection  * vec4(a_Position, 1.0);

				
}


#type fragment
#version 330 core 
				
layout(location = 0) out vec4 fragColor;

in vec2 v_TextCoord;

uniform sampler2D u_Texture;
uniform float u_TilingFactor;
			
void main()
{
	// TODO: Add variables so that for instance "texture(u_Texture, v_TextCoord * 1.0) * vec4(color)" could work
	// fragColor = texture(u_Texture, v_TextCoord ) ;

	fragColor = texture(u_Texture, v_TextCoord) ;
	
}