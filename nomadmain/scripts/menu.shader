menu/mainbackground
{
    nomipmaps
    nopicmip
    if ( $r_textureDetail == 1 || $r_textureDetail == 0 ) {
        texFilter bilinear
        map textures/menu/standard/fromeaglespeak.jpg
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcGen texture
    }
    elif ( $r_textureDetail == 2 ) {
        texFilter bilinear
        map textures/menu/standard/fromeaglespeak.jpg
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcGen texture
    }
    elif ( $r_textureDetail == 3 || $r_textureDetail == 4 ) {
        texFilter bilinear
        map textures/menu/high/fromeaglespeak.jpg
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcGen texture
    }
    else {
        texFilter bilinear
        map textures/menu/standard/fromeaglespeak.jpg
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcGen texture
    }
}

menu/tales_around_the_campfire
{
    nomipmaps
    nopicmip
    {
        texFilter nearest
        map textures/menu/campfire.jpg
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcGen texture
    }
}

menu/backdrop
{
    nomipmaps
    nopicmip
    {
        texFilter bilinear
        map textures/menu/backdrop.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        tcGen texture
    }
}

menu/arrow_horz_left
{
    nomipmaps
    nopicmip
    {
        texFilter bilinear
        map textures/menu/arrows_horz_left.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/arrow_horz_right
{
    nomipmaps
    nopicmip
    {
        texFilter bilinear
        map textures/menu/arrows_horz_right.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/arrows_vert_0
{
    nomipmaps
    nopicmip
    {
        texFilter bilinear
        map textures/menu/arrows_vert_0.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/arrows_vert_top
{
    nomipmaps
    nopicmip
    {
        texFilter bilinear
        map textures/menu/arrows_vert_top.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/arrows_vert_bot
{
    nomipmaps
    nopicmip
    {
        texFilter bilinear
        map textures/menu/arrows_vert_bot.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/rb_on
{
    nomipmaps
    nopicmip
    {
        texFilter bilinear
        map textures/menu/switch_on.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/rb_off
{
    nomipmaps
    nopicmip
    {
        texFilter bilinear
        map textures/menu/switch_off.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/backbutton0
{
    nomipmaps
    {
        texFilter bilinear
        map textures/menu/back_0.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/backbutton1
{
    nomipmaps
    {
        texFilter bilinear
        map textures/menu/back_1.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/save_0
{
    nomipmaps
    {
        texFilter bilinear
        map textures/menu/save_0.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/save_1
{
    nomipmaps
    {
        texFilter bilinear
        map textures/menu/save_1.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/reset_0
{
    nomipmaps
    {
        texFilter bilinear
        map textures/menu/reset_0.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/reset_1
{
    nomipmaps
    {
        texFilter bilinear
        map textures/menu/reset_1.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/accept_0
{
    nomipmaps
    {
        texFilter bilinear
        map textures/menu/accept_0.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/accept_1
{
    nomipmaps
    {
        texFilter bilinear
        map textures/menu/accept_1.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/play_0
{
    nomipmaps
    {
        texFilter bilinear
        map textures/menu/play_0.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}

menu/play_1
{
    nomipmaps
    {
        texFilter bilinear
        map textures/menu/play_1.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen vertex
    }
}
