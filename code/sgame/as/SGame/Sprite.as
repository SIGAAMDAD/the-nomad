#include "SGame/SpriteSheet.as"

namespace TheNomad::SGame {
    //
	// DrawRect: quick and easy utility function
	//
	void DrawRect( const vec3& in origin, const vec2& in size, const SpriteSheet@ sheet, const ivec2& in offset ) {
		TheNomad::Engine::Renderer::PolyVert[] verts( 4 );

		verts[0].xyz = vec3( origin.x + ( size.x / 2 ), origin.y, 0.0f );
		verts[1].xyz = vec3( origin.x + ( size.x / 2 ), origin.y + ( size.y / 2 ), 0.0f );
		verts[2].xyz = vec3( origin.x, origin.y + ( size.y / 2 ), 0.0f );
		verts[3].xyz = vec3( origin.x, origin.y, 0.0f );

		for ( uint i = 0; i < 4; i++ ) {
			verts[i].uv = sheet[offset.y * sheet.SpriteCountX() + offset.x][i];
		}

		TheNomad::Engine::Renderer::AddPolyToScene( sheet.GetShader(), verts );
	}

	class Sprite {
		Sprite() {
		}
		Sprite( const ivec2& in offset, const vec2& in sheetSize, const vec2& in spriteSize ) {
			Set( offset, sheetSize, spriteSize );
		}

		Sprite& opAssign( const Sprite& in other ) {
			for ( uint i = 0; i < 4; i++ ) {
				m_TexCoords[i] = other.m_TexCoords[i];
			}

			return this;
		}

		void Set( const ivec2& in offset, const vec2& in sheetSize, const vec2& in spriteSize ) {
			const vec2 min( ( ( float( offset.x ) + 1.0f ) * spriteSize.x ) / sheetSize.x,
				( ( float( offset.y ) + 1.0f ) * spriteSize.y ) / sheetSize.y );
			const vec2 max( ( float( offset.x ) * spriteSize.x ) / sheetSize.x,
				( float( offset.y ) * spriteSize.y ) / sheetSize.y );
			
			m_TexCoords[0][0] = min[0];
    		m_TexCoords[0][1] = max[1];

    		m_TexCoords[1][0] = min[0];
    		m_TexCoords[1][1] = min[1];

    		m_TexCoords[2][0] = max[0];
    		m_TexCoords[2][1] = min[1];
    
    		m_TexCoords[3][0] = max[0];
    		m_TexCoords[3][1] = max[1];
		}

		const vec2& opIndex( uint nIndex ) const {
			return m_TexCoords[ nIndex ];
		}
		vec2& opIndex( uint nIndex ) {
			return m_TexCoords[ nIndex ];
		}

		private vec2[] m_TexCoords( 4 );
	};
};