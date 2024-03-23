#include "entity.as"
#include "game.as"

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

			m_nSpriteCountX = uint( sheetSize.x / spriteSize.x );
			m_nSpriteCountY = uint( sheetSize.y / spriteSize.y );

			m_hShader = TheNomad::Engine::Renderer::RegisterShader( fileName );
			if ( m_hShader == FS_INVALID_HANDLE ) {
				ConsoleWarning( "SpriteSheet::Load: bad shader '" + fileName + "'!\n" );
				return;
			}

			m_SpriteData.reserve( m_nSpriteCountX * m_nSpriteCountY );
			for ( uint y = 0; y < m_nSpriteCountY; y++ ) {
				for ( uint x = 0; x < m_nSpriteCountX; x++ ) {
					m_SpriteData.push_back( Sprite( ivec2( x, y ), sheetSize, spriteSize ) );
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

		int GetShader() const {
			return m_hShader;
		}
		uint SpriteCountX() const {
			return m_nSpriteCountX;
		}
		uint SpriteCountY() const {
			return m_nSpriteCountY;
		}

		private array<Sprite> m_SpriteData;
		private uint m_nSpriteCountX = 0;
		private uint m_nSpriteCountY = 0;
		private int m_hShader = FS_INVALID_HANDLE;
	};

	class MarkPoly {
		MarkPoly() {
		}
		
		void Spawn( const vec3& in org, uint lifeTime, const SpriteSheet@ spriteSheet, const ivec2& in spriteOffset ) {
			origin = org;
			startTime = TheNomad::GameSystem::GameManager.GetGameMsec();
			endTime = startTime + lifeTime;
			this.spriteOffset = spriteOffset;
			@this.spriteSheet = @spriteSheet;
		}
		
		void RunTic() {
			lifeTime += TheNomad::GameSystem::GameManager.GetDeltaMsec();
			origin += vel;

			DrawRect( origin, vec2( 1.0f, 1.0f ), @spriteSheet, spriteOffset );
			
			if ( lifeTime >= endTime ) {
				GfxManager.FreeMarkPoly( @this );
			}
		}
		
		vec3 origin = vec3( 0.0f );
		vec3 vel = vec3( 0.0f );
		ivec2 spriteOffset = ivec2( 0 );
		const SpriteSheet@ spriteSheet;
		uint startTime;
		uint endTime;
		uint lifeTime;
		MarkPoly@ next;
		MarkPoly@ prev;
	};
	
	class GfxSystem : TheNomad::GameSystem::GameObject {
		GfxSystem() {
			m_PolyList.resize( TheNomad::Engine::CvarVariableInteger( "sgame_GfxDetail" ) * 15 );

			@m_ActiveMarkPolys.next = @m_ActiveMarkPolys;
			@m_ActiveMarkPolys.prev = @m_ActiveMarkPolys;
			@m_FreeMarkPolys = @m_PolyList[0];

			for ( uint i = 0; i < m_PolyList.size() - 1; i++ ) {
				@m_PolyList[i].next = @m_PolyList[i + 1];
			}
		}

		void OnLoad() {
		}
		void OnSave() const {
		}
		void OnRunTic() {
			for ( uint i = 0; i < m_PolyList.size(); i++ ) {
				m_PolyList[i].RunTic();
			}
		}
		bool OnConsoleCommand( const string& in cmd ) {
			return false;
		}
		void OnLevelStart() {
			// ensure we have the correct amount of polygons allocated
			
			if ( m_PolyList.size() != uint( TheNomad::Engine::CvarVariableInteger( "sgame_GfxDetail" ) ) ) {
				m_PolyList.resize( TheNomad::Engine::CvarVariableInteger( "sgame_GfxDetail" ) * 15 );
			}
		}
		void OnLevelEnd() {
		}
		const string& GetName() const override {
			return "GfxManager";
		}

		private MarkPoly@ AllocMarkPoly() {
			MarkPoly@ poly;
			uint time;

			if ( @m_FreeMarkPolys is null ) {
				// no free polys, so free the one at the end of the chain
				// remove the oldest active entity
				time = m_ActiveMarkPolys.prev.lifeTime;
				while ( @m_ActiveMarkPolys.prev !is null && time == m_ActiveMarkPolys.prev.lifeTime ) {
					FreeMarkPoly( @m_ActiveMarkPolys.prev );
				}
			}

			@poly = @m_FreeMarkPolys;
			@m_FreeMarkPolys = @m_FreeMarkPolys.next;

			// link into active list
			@poly.next = @m_ActiveMarkPolys.next;
			@poly.prev = @m_ActiveMarkPolys;
			@m_ActiveMarkPolys.next.prev = @poly;
			@m_ActiveMarkPolys.next = @poly;

			return @poly;
		}


		//
		// FreeMarkPoly: only MarkPoly when its finished should ever call this, or AllocMarkPoly
		//
		void FreeMarkPoly( MarkPoly@ poly ) {
			if ( @poly is null ) {
				GameError( "GfxManager::FreeMarkPoly: not active" );
			}

			// remove from doubly linked list
			@poly.prev.next = @poly.next;
			@poly.next.prev = @poly.prev;

			// the free list is only singly linked
			@poly.next = @m_FreeMarkPolys;
			@m_FreeMarkPolys = @poly;
		}
		
		void AddMarkPoly() {
			
		}
		void AddSmokePoly() {
			
		}
		void AddFlarePoly() {
			
		}

		void AddExplosionGfx( const vec3& in origin ) {
		}
		
		private array<MarkPoly> m_PolyList;
		private MarkPoly m_ActiveMarkPolys;
		private MarkPoly@ m_FreeMarkPolys;
		private bool m_bAllowGfx;
	};
	
	GfxSystem@ GfxManager;
};