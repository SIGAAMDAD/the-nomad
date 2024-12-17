"Desert Tileset"
{
	if ( $r_textureDetail < 2 ) {
		texFilter nearest
		map textures/desert_tilesets/low/DesertTilemap16x16.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	elif ( $r_textureDetail == 2 ) {
		texFilter nearest
		map textures/desert_tilesets/standard/DesertTilemap16x16.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	elif ( $r_textureDetail == 3 ) {
		texFilter nearest
		map textures/desert_tilesets/high/DesertTilemap16x16.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	elif ( $r_textureDetail > 3 ) {
		texFilter nearest
		map textures/desert_tilesets/very_high/DesertTilemap16x16.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

