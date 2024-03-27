#include "util/util.as"
#include "entity.as"
#include "level.as"
#include "convar.as"
#include "draw.as"

namespace TheNomad::GameSystem {
	interface GameObject {
		void OnLoad();
		void OnSave() const;
		void OnRunTic();
		void OnLevelStart();
		void OnLevelEnd();
		bool OnConsoleCommand( const string& in );
		const string& GetName() const;
	};

	array<GameObject@> GameSystems;

	GameObject@ AddSystem( GameObject@ SystemHandle ) {
		ConsolePrint( "Added GameObject System \"" + SystemHandle.GetName() + "\"\n" );
		GameSystems.Add( @SystemHandle );
		return SystemHandle;
	}

	class LoadSection {
		LoadSection( const string& in name ) {
			handle = FindSaveSection( name );
		}

		bool Found() const {
			return handle != FS_INVALID_HANDLE;
		}

		uint8 LoadByte( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt8( name, handle );
		}
		uint16 LoadUShort( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt16( name, handle );
		}
		uint32 LoadUInt( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt32( name, handle );
		}
		uint64 LoadULong( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt64( name, handle );
		}

		uint8 LoadUInt8( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt8( name, handle );
		}
		uint16 LoadUInt16( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt16( name, handle );
		}
		uint32 LoadUInt32( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt32( name, handle );
		}
		uint64 LoadUInt64( const string& in name ) const {
			return TheNomad::GameSystem::LoadUInt64( name, handle );
		}

		int8 LoadChar( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt8( name, handle );
		}
		int16 LoadShort( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt16( name, handle );
		}
		int32 LoadInt( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt32( name, handle );
		}
		int64 LoadLong( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt64( name, handle );
		}

		int8 LoadInt8( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt8( name, handle );
		}
		int16 LoadInt16( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt16( name, handle );
		}
		int32 LoadInt32( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt32( name, handle );
		}
		int64 LoadInt64( const string& in name ) const {
			return TheNomad::GameSystem::LoadInt64( name, handle );
		}

		private int handle;
	};

	class SaveSection {
		SaveSection( const string& in name ) {
			BeginSaveSection( name );
		}
		~SaveSection() {
			EndSaveSection();
		}
	
		void SaveIntArray( const string& in name, const array<int>& in value ) const {
			SaveArray( name, value );
		}
		void SaveFloatArray( const string& in name, const array<float>& in value ) const {
			SaveArray( name, value );
		}
		void SaveString( const string& in name, const string& in value ) const {
			TheNomad::GameSystem::SaveString( name, value );
		}
		void SaveChar( const string& in name, int8 value ) const {
			TheNomad::GameSystem::SaveInt8( name, value );
		}
		void SaveShort( const string& in name, int16 value ) const {
			TheNomad::GameSystem::SaveInt16( name, value );
		}
		void SaveInt( const string& in name, int32 value ) const {
			TheNomad::GameSystem::SaveInt32( name, value );
		}
		void SaveLong( const string& in name, int64 value ) const {
			TheNomad::GameSystem::SaveInt64( name, value );
		}
		void SaveByte( const string& in name, uint8 value ) const {
			TheNomad::GameSystem::SaveInt8( name, value );
		}
		void SaveUShort( const string& in name, uint16 value ) const {
			TheNomad::GameSystem::SaveUInt16( name, value );
		}
		void SaveUInt( const string& in name, uint32 value ) const {
			TheNomad::GameSystem::SaveUInt32( name, value );
		}
		void SaveULong( const string& in name, uint64 value ) const {
			TheNomad::GameSystem::SaveUInt64( name, value );
		}
		void SaveInt8( const string& in name, int8 value ) const {
			TheNomad::GameSystem::SaveInt8( name, value );
		}
		void SaveInt16( const string& in name, int16 value ) const {
			TheNomad::GameSystem::SaveInt16( name, value );
		}
		void SaveInt32( const string& in name, int32 value ) const {
			TheNomad::GameSystem::SaveInt32( name, value );
		}
		void SaveInt64( const string& in name, int64 value ) const {
			TheNomad::GameSystem::SaveInt64( name, value );
		}
		void SaveUInt8( const string& in name, uint8 value ) const {
			TheNomad::GameSystem::SaveUInt8( name, value );
		}
		void SaveUInt16( const string& in name, uint16 value ) const {
			TheNomad::GameSystem::SaveUInt16( name, value );
		}
		void SaveUInt32( const string& in name, uint32 value ) const {
			TheNomad::GameSystem::SaveUInt32( name, value );
		}
		void SaveUInt64( const string& in name, uint64 value ) const {
			TheNomad::GameSystem::SaveUInt64( name, value );
		}
	};
	
	class CampaignManager : GameObject {
		CampaignManager() {
			m_nGameMsec = 0;
			m_nDeltaTics = 0;

			GetGPUGameConfig( m_GPUConfig );
		}
		
		void OnLoad() {
			int hSection;
			int numEntities;

			hSection = FindSaveSection( GetName() );
			if ( hSection == FS_INVALID_HANDLE ) {
				return;
			}
		}
		bool OnConsoleCommand( const string& in cmd ) {
			return false;
		}
		void OnSave() const {
			BeginSaveSection( GetName() );
			
			EndSaveSection();
		}
		void OnRunTic() {
		}
		void OnLevelStart() {
		}
		void OnLevelEnd() {
		}
		const string& GetName() const {
			return "CampaignManager";
		}
		
		uint GetDeltaMsec() const {
			return m_nDeltaTics;
		}
		uint GetGameTic() const {
			return m_nGameTic;
		}
		uint GetGameMsec() const {
			return m_nGameMsec;
		}
		void SetMsec( uint msec ) {
			m_nDeltaTics = msec - m_nGameMsec;
			m_nGameMsec = msec;
		}

		TheNomad::Engine::Renderer::GPUConfig& GetGPUConfig() {
			return m_GPUConfig;
		}

		void SetMousePos( const ivec2& in mousePos ) {
			m_MousePos = mousePos;
		}
		ivec2& GetMousePos() {
			return m_MousePos;
		}
		const ivec2& GetMousePos() const {
			return m_MousePos;
		}
		
		private ivec2 m_MousePos;
		private uint m_nDeltaTics;
		private uint m_nGameMsec;
		private uint m_nGameTic;
		private TheNomad::Engine::Renderer::GPUConfig m_GPUConfig;
	};

	class RayCast {
		RayCast() {
		}

		vec3 m_Start = vec3( 0.0f );
		vec3 m_End = vec3( 0.0f );
		vec3 m_Origin = vec3( 0.0f );
	    uint32 m_nEntityNumber = 0;
		float m_nLength = 0.0f;
		float m_nAngle = 0.0f;
	    uint32 m_Flags = 0; // unused for now
	};

	CampaignManager@ GameManager;
};