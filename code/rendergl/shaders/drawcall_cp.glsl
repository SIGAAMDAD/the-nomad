layout( local_size_x = 128, local_size_y = 4, local_size_z = 1 ) in;

struct DrawCommand {
	GLuint count;
	GLuint instanceCount;
	GLuint firstIndex;
	GLint baseVertex;
	GLuint baseInstance;
	GLuint numVertices;
	GLuint textureID;
};

layout( std140, binding = 0 ) buffer u_DrawCalls {
	DrawCommand u_CommandBuffer[];
};

void main() {

}