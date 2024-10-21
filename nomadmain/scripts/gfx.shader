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
// hud elements
//
gfx/hud/blood_screen
{
	nopicmip
	nomipmaps
	{
		texFilter bilinear
		map textures/hud/PAIN1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		tcGen vertex
	}
	{
		texFilter bilinear
		map textures/hud/ARBLS1.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		tcMod scroll 0.0 -0.9
	}
	{
		texFilter bilinear
		map textures/hud/ARBLS2.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		tcMod scroll 0.0 -0.4
	}
	{
		texFilter bilinear
		map textures/hud/ARBLS3.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
		tcMod scroll 0.0 -0.25
	}
	{
		texFilter bilinear
		map textures/hud/ARBLS4.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
	{
		texFilter bilinear
		map textures/hud/ARBLS5.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

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

icons/iconw_db
{
	nopicmip
	nomipmaps
	{
		texFilter nearest
		map textures/icons/iconw_db.tga
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
		map textures/icons/icon_health.tga
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

//
// hud elements
//
gfx/hud/health_bar
{
	nomipmaps
	nopicmip
	{
		texFilter bilinear
		map $whiteimage
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
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
		rgbGen identityLighting
		alphaGen vertex
	}
}

gfx/bloodSplatter1
{

}

gfx/bloodSplatter2
{

}

gfx/bloodSplatter3
{

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
		clampmap gfx/damage/blood_stain.dds
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
	nopicmip
	nomipmaps
	polygonOffset
	{
		map gfx/shadow.tga
		blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
		rgbGen exactVertex
	}
}

// wake is the mark on water surfaces for paddling players
wake
{
	{
		clampmapmap sprites/splash.tga
		blendFunc GL_ONE GL_ONE
		rgbGen vertex
				tcmod rotate 250
				tcMod stretch sin .9 0.1 0 0.7
		rgbGen wave sin .7 .3 .25 .5
	}	
	{
		clampmap sprites/splash.tga
		blendFunc GL_ONE GL_ONE
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

gfx/env/smokePuff
{
	nomipmaps
	nopicmip
//	entityMergable		// allow all the sprites to be merged together
	{
		map gfx/env/smokepuff3.dds
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen		vertex
//		alphaGen	vertex
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
