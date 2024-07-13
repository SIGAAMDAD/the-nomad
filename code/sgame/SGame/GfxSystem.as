namespace TheNomad::SGame {
	class MarkPoly {
		MarkPoly() {
		}
		
		void Spawn( const vec3& in org, const vec3& in velocity, uint lifeTime, const SpriteSheet@ spriteSheet,
			const ivec2& in spriteOffset, EntityState@ state )
		{
			m_Origin = org;
			m_Velocity = velocity;
			m_nStartTime = TheNomad::Engine::System::Milliseconds();
			m_nLifeTime = lifeTime;
			m_SpriteOffset = spriteOffset;
			@m_SpriteSheet = @spriteSheet;
			@m_Animation = @state;
		}
		void Spawn( const vec3& in org, const vec3& in velocity, uint lifeTime, const SpriteSheet@ spriteSheet,
			const ivec2& in spriteOffset )
		{
			m_Origin = org;
			m_Velocity = velocity;
			m_nStartTime = TheNomad::Engine::System::Milliseconds();
			m_nLifeTime = lifeTime;
			m_SpriteOffset = spriteOffset;
			@m_SpriteSheet = @spriteSheet;
			@m_Animation = null;
		}
		void Spawn( const vec3& in origin, const vec3& in velocity, uint lifeTime, int hShader ) {
			m_Origin = origin;
			m_Velocity = velocity;
			m_nStartTime = TheNomad::Engine::System::Milliseconds();
			m_nLifeTime = lifeTime;
			m_hShader = hShader;
			@m_SpriteSheet = null;
		}
		
		void RunTic() {
			TheNomad::Engine::Renderer::RenderEntity refEntity;

			m_Origin += m_Velocity;
			if ( @m_Animation !is null ) {
				m_Animation.Run();
			}

			refEntity.origin = m_Origin;
			refEntity.scale = 1.0f;
			if ( @m_SpriteSheet is null ) {
				refEntity.sheetNum = -1;
				refEntity.spriteId = m_hShader;
			} else {
				refEntity.sheetNum = m_SpriteSheet.GetShader();
				refEntity.spriteId = m_SpriteOffset.y * m_SpriteSheet.GetSpriteCountX() + m_SpriteOffset.x;
				if ( @m_Animation !is null ) {
					refEntity.spriteId += m_Animation.GetAnimation().GetFrame();
				}
			}
			refEntity.Draw();
		}
		
		vec3 m_Origin = vec3( 0.0f );
		vec3 m_Velocity = vec3( 0.0f );
		ivec2 m_SpriteOffset = ivec2( 0 );
		const SpriteSheet@ m_SpriteSheet = null;
		EntityState@ m_Animation = null;
		MarkPoly@ m_Next = null;
		MarkPoly@ m_Prev = null;
		uint m_nStartTime = 0;
		uint m_nLifeTime = 0;
		int m_hShader = FS_INVALID_HANDLE;
	};
	
	class GfxSystem : TheNomad::GameSystem::GameObject {
		GfxSystem() {
		}

		void OnInit() {
		}
		void OnShutdown() {
		}

		void OnLoad() {
		}
		void OnSave() const {
		}
		void OnRunTic() {
		}
		void OnPlayerDeath( int ) {
		}
		void OnCheckpointPassed( uint ) {
		}
		void OnLevelStart() {
			const uint numGfx = 512;
			m_PolyList.Resize( numGfx );

			@m_ActiveMarkPolys.m_Next = @m_ActiveMarkPolys;
			@m_ActiveMarkPolys.m_Prev = @m_ActiveMarkPolys;
			@m_FreeMarkPolys = @m_PolyList[0];

			for ( uint i = 0; i < m_PolyList.Count() - 1; i++ ) {
				@m_PolyList[i].m_Next = @m_PolyList[i + 1];
			}

			TheNomad::Engine::ResourceCache.GetSpriteSheet( "gfx/effects/fireBlast", 480, 384, 64, 64 );
		}
		void OnLevelEnd() {
			m_PolyList.Clear();
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
				FreeMarkPoly( @m_ActiveMarkPolys.m_Prev );
			}

			@poly = @m_FreeMarkPolys;
			@m_FreeMarkPolys = @m_FreeMarkPolys.m_Next;

			// link into active list
			@poly.m_Next = @m_ActiveMarkPolys.m_Next;
			@poly.m_Prev = @m_ActiveMarkPolys;
			@m_ActiveMarkPolys.m_Next.m_Prev = @poly;
			@m_ActiveMarkPolys.m_Next = @poly;

			return @poly;
		}
		//
		// FreeMarkPoly: only MarkPoly when its finished should ever call this, or AllocMarkPoly
		//
		void FreeMarkPoly( MarkPoly@ poly ) {
			if ( @poly.m_Prev is null ) {
				GameError( "GfxManager::FreeMarkPoly: not active" );
			}

			// remove from doubly linked list
			@poly.m_Prev.m_Next = @poly.m_Next;
			@poly.m_Next.m_Prev = @poly.m_Prev;

			// the free list is only singly linked
			@poly.m_Next = @m_FreeMarkPolys;
			@m_FreeMarkPolys = @poly;
		}


		void OnRenderScene() {
			MarkPoly@ poly;
			MarkPoly@ next;

			// walk the list backwards, so any new local entities generated
			// (trails, marks, etc) will be present this frame
			@poly = @m_ActiveMarkPolys.m_Prev;
			for ( ; @poly !is @m_ActiveMarkPolys; @poly = @next ) {
				// grab next now, so if the local entity is freed we
				// still have it
				@next = @poly.m_Prev;

				if ( TheNomad::Engine::System::Milliseconds() - poly.m_nStartTime >= poly.m_nLifeTime ) {
					FreeMarkPoly( @poly );
					continue;
				}

				poly.RunTic();
			}
		}

		void AddDustPoly( const vec3& in origin, const vec3& in vel, uint lifeTime, int hShader ) {
			MarkPoly@ poly;

			@poly = AllocMarkPoly();
			poly.Spawn( origin, vel, lifeTime, hShader );
		}

		void AddDustPoly( const vec3& in origin, const vec3& in vel, uint lifeTime, SpriteSheet@ sheet, const ivec2& in offset ) {
			MarkPoly@ poly;

			@poly = AllocMarkPoly();
			poly.Spawn( origin, vel, lifeTime, @sheet, offset );
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
		private MarkPoly@ m_FreeMarkPolys = null;
		private bool m_bAllowGfx = true;
	};
	
	GfxSystem@ GfxManager = null;
};