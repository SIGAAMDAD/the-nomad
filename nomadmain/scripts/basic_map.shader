"Desert Tileset"
{
    nomipmaps
    nopicmip
    if ( $r_genNormalMaps == 0 && $r_normalMapping == 1 ) {
        texFilter nearest
        map textures/desert_tilesets/DesertTilemap16x16_n.jpg
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        stage normalMap
    }
    if ( $r_specularMapping == 1 ) {
        texFilter nearest
        map textures/desert_tilesets/DesertTilemap16x16_s.jpg
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        stage specularMap
    }
    {
        texFilter nearest
        map textures/desert_tilesets/DesertTilemap16x16_hdr.jpg
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
        stage diffuseMap
    }
}

