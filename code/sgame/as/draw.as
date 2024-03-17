#include "entity.as"
#include "game.as"

namespace TheNomad::SGame {
	shared class MarkPoly {
		MarkPoly() {
		}
		
		void Spawn( const vec3& in origin, uint lifeTime, int shader, int spriteOffset, ModuleObject@ main ) {
			m_Origin = origin;
			m_hShader = shader;
			m_nLifeTime = lifeTime;
			m_nSpriteOffset = spriteOffset;
			m_nElapsed = 0;
			m_bAlive = true;
			@ModObject = @main;
		}
		
		void RunTic() {
			if ( !m_bAlive ) {
				DebugPrint( "RunTic called on dead poly" );
				return;
			}
			
			m_nElapsed += ModObject.GameManager.GetDeltaTics();
			
			TheNomad::Engine::Renderer::AddSpriteToScene( m_Origin, m_hShader, m_nSpriteOffset );
			
			if ( m_nElapsed > m_nLifeTime ) {
				m_bAlive = false;
			}
		}
		
		void Clear() {
			m_Origin = vec3( 0.0f );
			m_hShader = 0;
			m_nVelocity = 0.0f;
		}
		
		vec3 m_Origin;
		float m_nVelocity;
		bool m_bAlive;
		int m_hShader;
		uint m_nSpriteOffset;
		uint m_nLifeTime;
		uint m_nElapsed;
		private ModuleObject@ ModObject;
	};
	
	shared class GfxManager : TheNomad::GameSystem::GameObject {
		GfxManager( ModuleObject@ main ) {
			m_PolyList.resize( TheNomad::Engine::CvarVariableInteger( "sgame_GfxDetail" ) * 15 );
			@ModObject = @main;
		}
		
		void AddPoly() {
			for ( uint i = 0; i < m_PolyList.size(); i++ ) {
				if ( !m_PolyList[i].m_bAlive ) {
					m_PolyList[i].Spawn( vec3( 0.0f ), 0, 0, 0, @ModObject );
				}
			}
		}

		void OnLoad() {
		}
		void OnSave() const {
		}
		void OnRunTic() {
			for ( uint i = 0; i < m_PolyList.size(); i++ ) {
				if ( !m_PolyList[i].m_bAlive ) {
					continue;
				}
				m_PolyList[i].RunTic();
			}
		}
		void OnConsoleCommand() {
		}
		void OnLevelStart() {
			// ensure we have the correct amount of polygons allocated
			
			if ( m_PolyList.size() != uint( TheNomad::Engine::CvarVariableInteger( "sgame_GfxDetail" ) ) ) {
				m_PolyList.resize( TheNomad::Engine::CvarVariableInteger( "sgame_GfxDetail" ) * 15 );
			}
		}
		void OnLevelEnd() {
			// clear all gfx
			
			DebugPrint( "GfxManager::OnLevelEnd: clearing gfx data...\n" );
			
			for ( uint i = 0; i < m_PolyList.size(); i++ ) {
				m_PolyList[i].Clear();
			}
		}
		const string& GetName() const override {
			return "GfxManager";
		}
		
		void AddMarkPoly() {
			
		}
		void AddSmokePoly() {
			
		}
		void AddFlarePoly() {
			
		}
		
		void RegisterSfx() {
			
		}
		
		const array<int>& GetExplosionShaders() const {
			return m_ExplosionShaders;
		}
		const array<int>& GetExplosionSfx() const {
			return m_ExplosionSfx;
		}
		
		private array<MarkPoly> m_PolyList;
		private bool m_bAllowGfx;
		
		private array<int> m_ExplosionShaders;
		private array<int> m_ExplosionSfx;

		private ModuleObject@ ModObject;
	};
};