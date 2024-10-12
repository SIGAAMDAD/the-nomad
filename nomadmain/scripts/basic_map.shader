"Desert Tileset"
{
	nomipmaps
	nopicmip
	{
		texFilter nearest
		map textures/desert_tilesets/DesertTilemap16x16.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		specularScale 0.5 2.5
		rgbGen vertex
		stage diffuseMap
	}
	{
		texFilter nearest
		map textures/desert_tilesets/DesertTilemap16x16_n.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage normalMap
	}
	{
		texFilter nearest
		map textures/desert_tilesets/DesertTilemap16x16_s.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage specularMap
	}
}

