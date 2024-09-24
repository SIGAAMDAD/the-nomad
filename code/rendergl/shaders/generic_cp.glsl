layout( local_size_x = 64, local_size_y = 16, local_size_z = 1 ) in;
layout( rgba32f, binding = 0 ) uniform image2D screen;

void main() {
	imageStore( screen, ivec2( gl_GlobalInvocationID.xy ), vec4( 1.0 ) );
}