skins/raio/torso
{
	{
		texFilter nearest
		map textures/sprites/players/raio/low/torso.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

skins/raio/arms_right
{
	if ( $r_textureDetail < 2 ) {
		texFilter nearest
		map textures/sprites/players/raio/low/arms_right.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	elif ( $r_textureDetail == 2 ) {
		texFilter nearest
		map textures/sprites/players/raio/standard/arms_right.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	elif ( $r_textureDetail == 3 ) {
		texFilter nearest
		map textures/sprites/players/raio/high/arms_right.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	elif ( $r_textureDetail > 3 ) {
		texFilter nearest
		map textures/sprites/players/raio/very_high/arms_right.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

skins/raio/arms_left
{
	if ( $r_textureDetail < 2 ) {
		texFilter nearest
		map textures/sprites/players/raio/low/arms_left.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	elif ( $r_textureDetail == 2 ) {
		texFilter nearest
		map textures/sprites/players/raio/standard/arms_left.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	elif ( $r_textureDetail == 3 ) {
		texFilter nearest
		map textures/sprites/players/raio/high/arms_left.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
	elif ( $r_textureDetail > 3 ) {
		texFilter nearest
		map textures/sprites/players/raio/very_high/arms_left.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

skins/raio/legs
{
	nomipmaps
	nopicmip
	{
		texFilter nearest
		map textures/sprites/players/raio/low/legs.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

sprites/mobs/shotty/base
{
	nomipmaps
	nopicmip
	{
		texFilter nearest
		map textures/sprites/mobs/shotty/base.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		stage diffuseMap
	}
}

sprites/mobs/shotty/wounded
{
	nomipmaps
	nopicmip
	{
		texFilter nearest
		map textures/sprites/mobs/shotty/wounded.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		stage diffuseMap
	}
}

sprites/mobs/shotty/fatal
{
	nomipmaps
	nopicmip
	{
		texFilter nearest
		map textures/sprites/mobs/shotty/fatal.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		stage diffuseMap
	}
}

sprites/mobs/grunt/base
{
	nomipmaps
	nopicmip
	{
		texFilter nearest
		map textures/sprites/mobs/grunt/base.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		stage diffuseMap
	}
}

sprites/mobs/grunt/wounded
{
	nomipmaps
	nopicmip
	{
		texFilter nearest
		map textures/sprites/mobs/grunt/wounded.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		stage diffuseMap
	}
}

sprites/mobs/grunt/fatal
{
	nomipmaps
	nopicmip
	{
		texFilter nearest
		map textures/sprites/mobs/grunt/fatal.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		stage diffuseMap
	}
}

sprites/weapons/double_barrel
{
	{
		texFilter nearest
		map textures/sprites/weapons/adb.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

sprites/weapons/plasma_smg
{
	nomipmaps
	{
		texFilter nearest
		map textures/sprites/weapons/plasma_smg.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}