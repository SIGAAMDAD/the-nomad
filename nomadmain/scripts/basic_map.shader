"Desert Tileset"
{
	nomipmaps
	nopicmip
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
	if ( $r_textureDetail < 2 ) {
		texFilter nearest
		map textures/desert_tilesets/DesertTilemap16x16_n.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage normalMap
	} elif ( $r_textureDetail >= 2 ) {
		texFilter nearest
		map textures/desert_tilesets/high/DesertTilemap16x16_n.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage normalMap
	}
	if ( $r_textureDetail < 2 ) {
		texFilter nearest
		map textures/desert_tilesets/DesertTilemap16x16_s.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage specularMap
	} elif ( $r_textureDetail >= 2 ) {
		texFilter nearest
		map textures/desert_tilesets/high/DesertTilemap16x16_s.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		stage specularMap
	}
}

