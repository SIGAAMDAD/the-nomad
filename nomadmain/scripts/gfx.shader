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
        //map textures/sfx/firegorre3.tga
        blendFunc add
        tcMod turb 0 .1 0 .1
        tcMod scale 2 1
        tcmod scroll 0.2  .1
	}
}

//
// hud elements
//
gfx/hud/blood_screen
{
    nopicmip
    nomipmaps
    {
        map textures/
        rgbGen vertex
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
        map icons/iconw_murstar.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_db
{
    nopicmip
    nomipmaps
    {
        texFilter nearest
        map icons/iconw_db.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_fullauto_shotty
{
    nopicmip
    nomipmaps
    {
        map icons/iconw_fullauto_shotty.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_plasma
{
    nopicmip
    nomipmaps
    {
        map icons/iconw_plasma.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_frag
{
    nopicmip
    nomipmaps
    {
        map icons/iconw_frag.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_incendiary
{
    nopicmip
    nomipmaps
    {
        map icons/iconw_incendiary.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/iconw_smoke
{
    nopicmip
    nomipmaps
    {
        map icons/iconw_smoke.tga
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
        map icons/icona_bullets.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_shells
{
    nopicmip
    nomipmaps
    {
        map icons/icona_shells.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_rockets
{
    nopicmip
    nomipmaps
    {
        map icons/icona_rockets.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_frag
{
    nopicmip
    nomipmaps
    {
        map icons/icona_frag.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_incendiary
{
    nopicmip
    nomipmaps
    {
        map icons/icona_incendiary.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

icons/icona_smoke
{
    nopicmip
    nomipmaps
    {
        map icons/icona_smoke.tga
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

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
        clampmap gfx/bloodSplatter0.tga
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
        map gfx/bullet_mrk.jpg
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    }
}

bloodMark
{
	nopicmip			// make sure a border remains
	polygonOffset
	{
		clampmap gfx/damage/blood_stain.tga
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

smokePuff
{
	entityMergable		// allow all the sprites to be merged together
	{
		map gfx/misc/smokepuff3.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen		vertex
		alphaGen	vertex
	}
}

shotgunSmokePuff
{
	{
		map gfx/misc/smokepuff2b.tga
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
