skins/raio
{
	{
		texFilter bilinear
		map textures/sprites/players/raio/skin.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

skins/raio/torso
{
	{
		texFilter nearest
		map textures/sprites/players/raio/low/torso.png
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

skins/raio/arms_right
{
	{
		texFilter nearest
		map textures/sprites/players/raio/low/arms_right.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

skins/raio/arms_left
{
	{
		texFilter nearest
		map textures/sprites/players/raio/low/arms_left.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

skins/raio/legs
{
	nomipmaps
	nopicmip
	{
		texFilter nearest
		map textures/sprites/players/raio/low/legs.png
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

sprites/mobs/mercenary/shotgunner
{
	nomipmaps
	nopicmip
	{
		texFilter nearest
		map textures/sprites/mobs/mercenary/shotty.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

sprites/mobs/mercenary/gatling_gunner
{
	nomipmaps
	nopicmip
	{
		texFilter nearest
		map textures/sprites/mobs/mercenary/gatling.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

sprites/weapons/double_barrel
{
	{
		texFilter nearest
		map textures/sprites/weapons/double_barrel.png
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