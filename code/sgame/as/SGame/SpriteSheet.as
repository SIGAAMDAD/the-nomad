#include "SGame/Sprite.as"

namespace TheNomad::SGame {
    class SpriteSheet {
		SpriteSheet() {
		}
		SpriteSheet( const string& in fileName, const vec2& in sheetSize, const vec2& in spriteSize ) {
			Load( fileName, sheetSize, spriteSize );
		}

		void Load( const string& in fileName, const vec2& in sheetSize, const vec2& in spriteSize ) {
			if ( !( ( uint( sheetSize.x ) % 2 ) == 0 && ( uint( sheetSize.y ) % 2 ) == 0 )
				|| !( ( uint( spriteSize.x ) % 2 ) == 0 && ( uint( spriteSize.y ) % 2 ) == 0 ) )
			{
				GameError( "SpriteSheet::Load: please ensure that your sheetSize and spriteSize are powers of two" );
			}

			m_SheetSize = sheetSize;
			m_SpriteSize = spriteSize;

			m_nSpriteCountX = uint( sheetSize.x / spriteSize.x );
			m_nSpriteCountY = uint( sheetSize.y / spriteSize.y );

			m_hShader = TheNomad::Engine::Renderer::RegisterShader( fileName );
			if ( m_hShader == FS_INVALID_HANDLE ) {
				ConsoleWarning( "SpriteSheet::Load: bad shader '" + fileName + "'!\n" );
				return;
			}

			m_SpriteData.Reserve( m_nSpriteCountX * m_nSpriteCountY );
			for ( uint y = 0; y < m_nSpriteCountY; y++ ) {
				for ( uint x = 0; x < m_nSpriteCountX; x++ ) {
					m_SpriteData.Add( Sprite( ivec2( x, y ), sheetSize, spriteSize, m_hShader ) );
				}
			}
			DebugPrint( "Generated " + m_SpriteData.size() + " sprites for '" + fileName + "'\n" );
		}
		const Sprite& opIndex( uint nIndex ) const {
			return m_SpriteData[ nIndex ];
		}
		Sprite& opIndex( uint nIndex ) {
			return m_SpriteData[ nIndex ];
		}

		const vec2& GetSpriteSize() const {
			return m_SpriteSize;
		}

		int GetShader() const {
			return m_hShader;
		}
		uint SpriteCountX() const {
			return m_nSpriteCountX;
		}
		uint SpriteCountY() const {
			return m_nSpriteCountY;
		}

		private vec2 m_SpriteSize;
		private vec2 m_SheetSize;
		private array<Sprite> m_SpriteData;
		private uint m_nSpriteCountX = 0;
		private uint m_nSpriteCountY = 0;
		private int m_hShader = FS_INVALID_HANDLE;
	};
};