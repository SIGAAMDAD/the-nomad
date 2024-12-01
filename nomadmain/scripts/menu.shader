menu/fmodLogo
{
	nomipmaps
	nopicmip
	{
		texFilter bilinear
		map fmod_logo.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
	}
}

menu/companyLogo
{
	nomipmaps
	nopicmip
	{
		texFilter bilinear
		map GDRLogo.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
	}
}

menu/engineLogo
{
	nomipmaps
	nopicmip
	{
		texFilter bilinear
		map SIREngineLogo.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
	}
}

menu/mainbackground
{
	nomipmaps
	nopicmip
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/fromeaglespeak.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
	}
	elif ( $r_textureDetail == 2 ) {
		texFilter bilinear
		map textures/menu/standard/fromeaglespeak.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
	}
	elif ( $r_textureDetail > 2 ) {
		texFilter bilinear
		map textures/menu/high/fromeaglespeak.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
	}
}

menu/mainbackground_blur
{
	nomipmaps
	nopicmip
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/fromeaglespeak_blur.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
	}
	elif ( $r_textureDetail == 2 ) {
		texFilter bilinear
		map textures/menu/standard/fromeaglespeak_blur.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
	}
	elif ( $r_textureDetail == 3 ) {
		texFilter bilinear
		map textures/menu/high/fromeaglespeak_blur.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
	}
	elif ( $r_textureDetail == 4 ) {
		texFilter bilinear
		map textures/menu/extreme/fromeaglespeak_blur.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
	}
}

menu/playBackground
{
	nomipmaps
	nopicmip
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/standard/playBackground.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
	}
	elif ( $r_textureDetail == 2 ) {
		texFilter bilinear
		map textures/menu/high/playBackground.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
	}
	elif ( $r_textureDetail > 2 ) {
		texFilter bilinear
		map textures/menu/extreme/playBackground.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen texture
		rgbGen vertex
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
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/back_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/back_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/backbutton1
{
	nomipmaps
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/back_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/back_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/save_0
{
	nomipmaps
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/save_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/save_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/save_1
{
	nomipmaps
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/save_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/save_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/reset_0
{
	nomipmaps
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/reset_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/reset_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/reset_1
{
	nomipmaps
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/reset_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/reset_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/accept_0
{
	nomipmaps
	if  ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/accept_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/accept_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/accept_1
{
	nomipmaps
	if  ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/accept_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/accept_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/play_0
{
	nomipmaps
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/play_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/play_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/play_1
{
	nomipmaps
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/play_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/play_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/load_0
{
	nomipmaps
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/load_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/load_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/load_1
{
	nomipmaps
	if ( $r_textureDetail == 0 || $r_textureDetail == 1 ) {
		texFilter bilinear
		map textures/menu/low/load_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/load_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/delete_0
{
	nomipmaps
	if ( $r_textureDetail < 2 ) {
		texFilter bilinear
		map textures/menu/low/delete_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/delete_0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

menu/delete_1
{
	nomipmaps
	if ( $r_textureDetail == 0 || $r_textureDetail == 1 ) {
		texFilter bilinear
		map textures/menu/low/delete_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	else {
		texFilter bilinear
		map textures/menu/standard/delete_1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}
