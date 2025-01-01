// GFX.SHADER
// 
// this file contains shaders that are used by the programmers to
// generate special effects not attached to specific geometry.  This
// also has 2D shaders such as fonts, etc.
//

white
{
	nomipmaps
	nopicmip
	{
		texFilter bilinear
		map $whiteimage
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/bigchars
{
	nomipmaps
	nopicmip
	{
		texFilter bilinear
		map gfx/bigchars.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/fonts
{
	nomipmaps
	{
		texFilter bilinear
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

/*
console
{
	nopicmip
	nomipmaps
	{
		texFilter bilinear
		map gfx/console01.tga
		blendFunc GL_ONE GL_ZERO
		tcMod scroll .02  0
		tcmod scale 2 1
	}
	{
		texFilter bilinear
		map gfx/console02.jpg
		//map gfx/effects/flamegotga3.tga
		blendFunc add
		tcMod turb 0 .1 0 .1
		tcMod scale 2 1
		tcmod scroll 0.2  .1
	}
}
*/

//
// powerup icons
//
icons/iconpw_pewpew
{
	nopicmip
	nomipmaps
	{
		map textures/icons/iconpw_pewpew.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen wave sin 0.3 5.0 0.0 0.4
	}
}

icons/iconpw_angry
{
	nopicmip
	nomipmaps
	{
		map textures/icons/iconpw_angry.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen wave sin 0.3 5.0 0.0 0.4
	}
}

//
// weapon icons
//
icons/iconw_murstar
{
	nopicmip
	nomipmaps
	{
		map textures/icons/iconw_murstar.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

icons/iconw_adb
{
	nopicmip
	nomipmaps
	{
		texFilter nearest
		map icons/iconw_adb.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

icons/iconw_fab
{
	nopicmip
	nomipmaps
	{
		map textures/icons/iconw_fullauto_shotty.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

icons/iconw_plasma
{
	nopicmip
	nomipmaps
	{
		map textures/icons/iconw_plasma.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

icons/iconw_frag
{
	nopicmip
	nomipmaps
	{
		map textures/icons/iconw_frag.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

icons/iconw_incendiary
{
	nopicmip
	nomipmaps
	{
		map textures/icons/iconw_incendiary.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

icons/iconw_smoke
{
	nopicmip
	nomipmaps
	{
		map textures/icons/iconw_smoke.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

//
// misc
//
icons/icon_healthpack
{
	nopicmip
	nomipmaps
	{
		map icons/icon_health.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

//
// ammo icons
//
icons/icona_bullets
{
	nopicmip
	nomipmaps
	{
		map textures/icons/icona_bullets.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

icons/icona_shells
{
	nopicmip
	nomipmaps
	{
		map textures/icons/icona_shells.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

icons/icona_rockets
{
	nopicmip
	nomipmaps
	{
		map textures/icons/icona_rockets.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

icons/icona_frag
{
	nopicmip
	nomipmaps
	{
		map textures/icons/icona_frag.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

icons/icona_incendiary
{
	nopicmip
	nomipmaps
	{
		map textures/icons/icona_incendiary.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

icons/icona_smoke
{
	nopicmip
	nomipmaps
	{
		map textures/icons/icona_smoke.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

gfx/effects/flame
{
	nomipmaps
	nopicmip
	{
		map gfx/effects/flame.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
//		rgbGen wave inverseSawtooth 0 1 0 10
	}
}

gfx/effects/flame1
{
	nomipmaps
	nopicmip
	{
		map gfx/effects/flame1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
//		rgbGen wave inverseSawtooth 0 1 0 10
	}
}

gfx/effects/flame2
{
	nomipmaps
	nopicmip
	{
		map gfx/effects/flame2.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
//		rgbGen wave inverseSawtooth 0 1 0 10
	}
}

gfx/effects/flame3
{
	nomipmaps
	nopicmip
	{
		map gfx/effects/flame3.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
//		rgbGen wave inverseSawtooth 0 1 0 10
	}
}

gfx/effects/flame4
{
	nomipmaps
	nopicmip
	{
		map gfx/effects/flame4.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
//		rgbGen wave inverseSawtooth 0 1 0 10
	}
}

gfx/effects/flame5
{
	nomipmaps
	nopicmip
	{
		map gfx/effects/flame5.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
//		rgbGen wave inverseSawtooth 0 1 0 10
	}
}

gfx/effects/flame6
{
	nomipmaps
	nopicmip
	{
		map gfx/effects/flame6.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
//		rgbGen wave inverseSawtooth 0 1 0 10
	}
}

/*
gfx/effects/flame
{
	nomipmaps
	nopicmip
	{
		animMap 10 gfx/effects/flame1.jpg gfx/effects/flame2.jpg gfx/effects/flame3.jpg gfx/effects/flame4.jpg gfx/effects/flame5.jpg gfx/effects/flame6.jpg gfx/effects/flame7.jpg gfx/effects/flame8.jpg
		blendFunc GL_ONE GL_ONE
		rgbGen wave inverseSawtooth 0 1 0 10
	}
	{
		animMap 10 gfx/effects/flame2.jpg gfx/effects/flame3.jpg gfx/effects/flame4.jpg gfx/effects/flame5.jpg gfx/effects/flame6.jpg gfx/effects/flame7.jpg gfx/effects/flame8.jpg gfx/effects/flame1.jpg
		blendFunc GL_ONE GL_ONE
		rgbGen wave sawtooth 0 1 0 10
	}
	{
		map gfx/effects/flameball.jpg
		blendFunc GL_ONE GL_ONE
		rgbGen wave sin .6 .2 0 .6	
	}
}
*/

gfx/checkpoint
{
	{
		map particle_effects/Spritesheets/campfire.png
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/completed_checkpoint
{
	{
		map particle_effects/Spritesheets/unlit.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

//
// particle effects
//

gfx/env/dustScreen
{
	{
		map particle_effects/Spritesheets/dustcloud.png
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/env/flameBall
{
	{
		map particle_effects/Spritesheets/flameball.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/env/landing
{
	{
		map particle_effects/Spritesheets/land.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/env/smokeTrail
{
	{
		map particle_effects/Spritesheets/Smoke2-Sheet.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/env/smokePuff
{
	{
		map particle_effects/Spritesheets/dust_trail.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/env/jump
{
	{
		map particle_effects/Spritesheets/jump.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/env/land
{
	{
		map particle_effects/Spritesheets/land.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/spurt
{
	{
		texFilter nearest
		clampmap particle_effects/Spritesheets/blood_splatter.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/splatter
{
	{
		texFilter nearest
		clampmap particle_effects/Spritesheets/Splatter-Sheet.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

//
// environmental artifacts
//
gfx/env/hanging_body
{
	nopicmip
	nomipmaps
	{
		map gfx/env/gore1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/env/impaled_body
{
	nopicmip
	nomipmaps
	{
		map gfx/env/gore0.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

gfx/env/tent
{
	nopicmip
	nomipmaps
	{
		map gfx/env/tent.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

//
// wall marks
//
gfx/bloodSplatter0
{
	nopicmip
	{
		clampmap gfx/env/blood_spurt.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/env/bullet_hole
{
	nopicmip
	nomipmaps
	{
		clampmap gfx/env/bullet_hole.png
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		alphaGen vertex
	}
}

gfx/bulletMarks
{
	nomipmaps
	nopicmip
	polygonOffset
	{
		map gfx/bullet_mrk.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}

bloodMark
{
	nopicmip			// make sure a border remains
	polygonOffset
	{
		clampmap gfx/env/blood_stain.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identityLighting
		alphaGen vertex
	}
}

bloodTrail
{
		
	nopicmip			// make sure a border remains
	entityMergable		// allow all the sprites to be merged together
	{
		//clampmap gfx/misc/blood.tga
				clampmap gfx/damage/blood_spurt.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen		vertex
		alphaGen	vertex
	}
}

gfx/damage/bullet_mrk
{
	polygonOffset
	{
		map gfx/damage/bullet_mrk.tga
		blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
		rgbGen exactVertex
	}
}
gfx/damage/burn_med_mrk
{
	polygonOffset
	{
		map gfx/damage/burn_med_mrk.tga
		blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
		rgbGen exactVertex
	}
}
gfx/damage/hole_lg_mrk
{
	polygonOffset
	{
		map gfx/damage/hole_lg_mrk.tga
		blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
		rgbGen exactVertex
	}
}
gfx/damage/plasma_mrk
{
	polygonOffset
	{
		map gfx/damage/plasma_mrk.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		alphaGen vertex
	}
}


// markShadow is a very cheap blurry blob underneath the player
markShadow
{
	{
		map gfx/env/shadow.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

// wake is the mark on water surfaces for paddling players
wake
{
	if ( $r_textureDetail < 2 ) {
		clampmap gfx/env/low/splash.jpg
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		tcmod rotate 250
		tcMod stretch sin .9 0.1 0 0.7
		rgbGen wave sin .7 .3 .25 .5
	}
	elif ( $r_textureDetail == 2 ) {
		clampmap gfx/env/standard/splash.jpg
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		tcmod rotate 250
		tcMod stretch sin .9 0.1 0 0.7
		rgbGen wave sin .7 .3 .25 .5
	}
	elif ( $r_textureDetail > 2 ) {
		clampmap gfx/env/high/splash.jpg
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		tcmod rotate 250
		tcMod stretch sin .9 0.1 0 0.7
		rgbGen wave sin .7 .3 .25 .5
	}

	if ( $r_textureDetail < 2 ) {
		clampmap gfx/env/low/splash.jpg
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		tcmod rotate -230
		tcMod stretch sin .9 0.05 0 0.9
		rgbGen wave sin .7 .3 .25 .4
	}
	elif ( $r_textureDetail == 2 ) {
		clampmap gfx/env/standard/splash.jpg
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		tcmod rotate -230
		tcMod stretch sin .9 0.05 0 0.9
		rgbGen wave sin .7 .3 .25 .4
	}
	elif ( $r_textureDetail > 2 ) {
		clampmap gfx/env/high/splash.jpg
		blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		tcmod rotate -230
		tcMod stretch sin .9 0.05 0 0.9
		rgbGen wave sin .7 .3 .25 .4
	}
}

waterBubble
{
	sort	underwater
	entityMergable		// allow all the sprites to be merged together
	{
		map sprites/bubble.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen		vertex
		alphaGen	vertex
	}
}

shotgunSmokePuff
{
	{
		map gfx/misc/smokepuff2b.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		alphaGen entity		
		rgbGen entity
	}
}

lightningBolt
{
	{
		map gfx/misc/lightning3.tga
		blendFunc GL_ONE GL_ONE
//                rgbgen wave sin 1 5.1 0 7.1
				rgbgen wave sin 1 0.5 0 7.1
				 tcmod scale  2 1
		tcMod scroll -5 0
	}
	{
		map gfx/misc/lightning3.tga
		blendFunc GL_ONE GL_ONE
//                rgbgen wave sin 1 8.3 0 8.1
				rgbgen wave sin 1 0.8 0 8.1
				tcmod scale  -1.3 -1
		tcMod scroll -7.2 0
	}
}

bulletExplosion
{
	cull disable
	{
		animmap 5 gfx/env/weaphits/bullet1.tga  gfx/env/weaphits/bullet2.tga  gfx/env/weaphits/bullet3.tga gfx/colors/black.tga
		rgbGen wave inversesawtooth 0 1 0 5
		blendfunc add
	}
	{
		animmap 5 gfx/env/weaphits/bullet2.tga  gfx/env/weaphits/bullet3.tga  gfx/colors/black.tga gfx/colors/black.tga
		rgbGen wave sawtooth 0 1 0 5
		blendfunc add
	}
}

rocketExplosion
{
	cull disable
	{
		animmap 8 gfx/env/weaphits/rlboom/rlboom_1.jpg  gfx/env/weaphits/rlboom/rlboom_2.jpg gfx/env/weaphits/rlboom/rlboom_3.jpg gfx/env/weaphits/rlboom/rlboom_4.jpg gfx/env/weaphits/rlboom/rlboom_5.jpg gfx/env/weaphits/rlboom/rlboom_6.jpg gfx/env/weaphits/rlboom/rlboom_7.jpg gfx/env/weaphits/rlboom/rlboom_8.jpg
		rgbGen wave inversesawtooth 0 1 0 8
		blendfunc add
	}
	{
		animmap 8 gfx/env/weaphits/rlboom/rlboom_2.jpg gfx/env/weaphits/rlboom/rlboom_3.jpg gfx/env/weaphits/rlboom/rlboom_4.jpg gfx/env/weaphits/rlboom/rlboom_5.jpg gfx/env/weaphits/rlboom/rlboom_6.jpg gfx/env/weaphits/rlboom/rlboom_7.jpg gfx/env/weaphits/rlboom/rlboom_8.jpg gfx/colors/black.jpg
		rgbGen wave sawtooth 0 1 0 8
		blendfunc add
	}
}

grenadeExplosion
{
	cull disable
	{
		animmap 5 gfx/env/weaphits/glboom/glboom_1.tga  gfx/env/weaphits/glboom/glboom_2.tga gfx/env/weaphits/glboom/glboom_3.tga
		rgbGen wave inversesawtooth 0 1 0 5
		blendfunc add
	}
	{
		animmap 5 gfx/env/weaphits/glboom/glboom_2.tga  gfx/env/weaphits/glboom/glboom_3.tga gfx/colors/black.tga
		rgbGen wave sawtooth 0 1 0 5
		blendfunc add
	}
}

gfx/effects/flameBltga
{
	nopicmip
	nomipmaps
	{
		texFilter nearest
		map gfx/effects/flame.ttga		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}