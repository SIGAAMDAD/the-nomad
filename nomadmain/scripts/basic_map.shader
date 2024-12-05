"Desert Tileset"
{
	if ( $r_textureDetail < 2 ) {
		texFilter nearest
		map textures/desert_tilesets/DesertTilemap16x16.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		specularScale 0.5 2.5
		rgbGen vertex
		stage diffuseMap
	} elif ( $r_textureDetail >= 2 ) {
		texFilter nearest
		map textures/desert_tilesets/high/DesertTilemap16x16.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		specularScale 0.5 2.5
		rgbGen vertex
		stage diffuseMap
	}
}

