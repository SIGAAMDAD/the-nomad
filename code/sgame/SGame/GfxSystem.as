namespace TheNomad::SGame {
	class MarkPoly {
		MarkPoly() {
		}
		
		void Spawn( const vec3& in org, uint lifeTime, const SpriteSheet@ spriteSheet, const ivec2& in spriteOffset ) {
			origin = org;
			startTime = TheNomad::GameSystem::GameManager.GetGameTic();
			this.lifeTime = lifeTime;
			this.spriteOffset = spriteOffset;
			@this.spriteSheet = @spriteSheet;
		}
		
		void RunTic() {
			origin += vel;
			TheNomad::Engine::Renderer::AddSpriteToScene( origin, spriteSheet.GetShader(),
				spriteOffset.y * spriteSheet.GetSpriteCountX() + spriteOffset.x, false );
			if ( TheNomad::GameSystem::GameManager.GetGameTic() - startTime >= lifeTime ) {
				GfxManager.FreeMarkPoly( @this );
			}
		}
		
		vec3 origin = vec3( 0.0f );
		vec3 vel = vec3( 0.0f );
		ivec2 spriteOffset = ivec2( 0 );
		const SpriteSheet@ spriteSheet;
		uint startTime;
		uint lifeTime;
		MarkPoly@ next;
		MarkPoly@ prev;
	};
	
	class GfxSystem : TheNomad::GameSystem::GameObject {
		GfxSystem() {
		}

		void OnInit() {
			const uint numGfx = sgame_GfxDetail.GetInt() * 15;

			for ( uint i = 0; i < numGfx; i++ ) {
				m_PolyList.Add( MarkPoly() );
			}

			@m_ActiveMarkPolys.next = @m_ActiveMarkPolys;
			@m_ActiveMarkPolys.prev = @m_ActiveMarkPolys;
			@m_FreeMarkPolys = @m_PolyList[0];

			for ( uint i = 0; i < m_PolyList.Count() - 1; i++ ) {
				@m_PolyList[i].next = @m_PolyList[i + 1];
			}
		}
		void OnShutdown() {
			m_PolyList.Clear();
		}

		void OnLoad() {
		}
		void OnSave() const {
		}
		void OnRunTic() {
			for ( uint i = 0; i < m_PolyList.Count(); i++ ) {
				m_PolyList[i].RunTic();
			}
		}
		bool OnConsoleCommand( const string& in cmd ) {
			return false;
		}
		void OnLevelStart() {
			// ensure we have the correct amount of polygons allocated

			while ( m_PolyList.Count() != uint( sgame_GfxDetail.GetInt() ) ) {
				m_PolyList.RemoveLast();
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

		void OnRenderScene() {
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
	
	GfxSystem@ GfxManager = null;
};