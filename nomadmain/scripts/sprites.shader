sprites/players/after_image_raio_arms_0
{
	nopicmip
	nomipmaps
	if ( $r_arb_texture_compression > 0 ) {
		texFilter nearest
		map textures/sprites/players/raio_arms_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen const ( 0.0 0.0 0.0 )
		alphaGen const 0.25
		stage diffuseMap
	}
}

sprites/players/after_image_raio_arms_1
{
	nopicmip
	nomipmaps
	if ( $r_arb_texture_compression > 0 ) {
		texFilter nearest
		map textures/sprites/players/raio_arms_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen const ( 0.0 0.0 0.0 )
		alphaGen const 0.25
		stage diffuseMap
	}
}

sprites/players/after_image_raio_legs_0
{
	nopicmip
	nomipmaps
	if ( $r_arb_texture_compression > 0 ) {
		texFilter nearest
		map textures/sprites/players/raio_legs_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen const ( 0.0 0.0 0.0 )
		alphaGen const 0.25
		stage diffuseMap
	}
}

sprites/players/after_image_raio_legs_1
{
	nopicmip
	nomipmaps
	if ( $r_arb_texture_compression > 0 ) {
		texFilter nearest
		map textures/sprites/players/raio_legs_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen const ( 0.0 0.0 0.0 )
		alphaGen const 0.25
		stage diffuseMap
	}
}

sprites/players/after_image_raio_torso
{
	nopicmip
	nomipmaps
	if ( $r_arb_texture_compression > 0 ) {
		texFilter nearest
		map textures/sprites/players/raio_torso.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen const ( 0.0 0.0 0.0 )
		alphaGen const 0.25
		stage diffuseMap
	}
}

sprites/players/raio_torso
{
	nopicmip
	nomipmaps
	if ( $r_arb_texture_compression > 0 ) {
		texFilter nearest
		map textures/sprites/players/raio_torso.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage diffuseMap
	}
	{
		texFilter nearest
		map textures/sprites/players/raio_torso_n.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage normalMap
	}
}

sprites/players/raio_legs_0
{
	nopicmip
	nomipmaps
	if ( $r_arb_texture_compression > 0 ) {
		texFilter nearest
		map textures/sprites/players/raio_legs_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage diffuseMap
	}
	{
		texFilter nearest
		map textures/sprites/players/raio_legs_0_n.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage normalMap
	}
}

sprites/players/raio_legs_1
{
	nopicmip
	nomipmaps
	if ( $r_arb_texture_compression > 0 ) {
		texFilter nearest
		map textures/sprites/players/raio_legs_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage diffuseMap
	}
	{
		texFilter nearest
		map textures/sprites/players/raio_legs_1_n.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage normalMap
	}
}

sprites/players/raio_arms_0
{
	nopicmip
	nomipmaps
	if ( $r_arb_texture_compression > 0 ) {
		texFilter nearest
		map textures/sprites/players/raio_arms_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage diffuseMap
	}
	{
		texFilter nearest
		map textures/sprites/players/raio_arms_0_n.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage normalMap
	}
}

sprites/players/raio_arms_1
{
	nopicmip
	nomipmaps
	if ( $r_arb_texture_compression > 0 ) {
		texFilter nearest
		map textures/sprites/players/raio_arms_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage diffuseMap
	}
	{
		texFilter nearest
		map textures/sprites/players/raio_arms_1_n.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage normalMap
	}
}

sprites/mobs/grunt
{
	nopicmip
	nomipmaps
	if ( $r_arb_texture_compression > 0 ) {
		texFilter nearest
		map textures/sprites/mobs/grunt.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

sprites/mobs/mercenary
{
	nopicmip
	nomipmaps
	if ( $r_arb_texture_compression > 0 ) {
		texFilter nearest
		map textures/sprites/mobs/mercenary.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}
