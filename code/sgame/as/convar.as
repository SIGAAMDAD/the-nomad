#include "main.as"

namespace TheNomad {
	//
	// ConVar: an implementation of the vmCvar_t, but to make it easier to manage,
	// its been put here
	// DO NOT MODIFY
	//
	class ConVar {
		ConVar() {
		}
		
		void Register( const string& in name, const string& in value, uint flags, bool bTrackChanges ) {
			m_Name = name;
			m_Value = value;
			m_Flags = flags;
			m_bTrackChanges = bTrackChanges;
			Engine::CvarRegister( name, value, flags, m_IntValue, m_FloatValue, m_nModificationCount, m_nCvarHandle );
		}
		
		const string& GetName() const {
			return m_Name;
		}
		const string& GetValue() const {
			return m_Value;
		}
		int64 GetInt() const {
			return m_IntValue;
		}
		float GetFloat() const {
			return m_FloatValue;
		}
		uint GetFlags() const {
			return m_Flags;
		}
		int GetModificationCount() const {
			return m_nModificationCount;
		}
		bool Track() const {
			return m_bTrackChanges;
		}
		
		void Set( const string& in value ) {
			Engine::CvarSet( m_Name, value );
		}
		void Update() {
			Engine::CvarUpdate( m_Value, m_IntValue, m_FloatValue, m_nModificationCount, m_nCvarHandle );
		}
		
		private string m_Name;
		private string m_Value;
		private int64 m_IntValue;
		private float m_FloatValue;
		private uint m_Flags;
		private int m_nModificationCount;
		private int m_nCvarHandle;
		private bool m_bTrackChanges;
	};

	class CvarTableEntry {
		CvarTableEntry() {
		}
		CvarTableEntry( ConVar@ var ) {
			@m_Handle = @var;
		}

		ConVar@ m_Handle = null;
		int m_nModificationCount = 0;
	};
	
	class CvarSystem : TheNomad::GameSystem::GameObject {
		CvarSystem() {
			TheNomad::Engine::CmdAddCommand( "sgame.list_cvars" );
		}
		
		ConVar@ AddCvar( const string& in name, const string& in value, uint flags, bool bTrackChanges ) {
			CvarTableEntry@ var = CvarTableEntry( ConVar() );
			var.m_Handle.Register(  name, value, flags, bTrackChanges  );
			m_CvarCache.Add( @var );
			return @var.m_Handle;
		}
		
		void ListVars_f() {
			ConsolePrint( "VM " + MODULE_NAME + " Cvars:\n" );

			for ( uint i = 0; i < m_CvarCache.size(); i++ ) {
				ConsolePrint( m_CvarCache[i].m_Handle.GetName() + " " + m_CvarCache[i].m_Handle.GetValue() );
			}
		}

		bool OnConsoleCommand( const string& in cmd ) {
			return false;
		}
		void OnLoad() {
		}
		void OnSave() const {
		}
		void OnRunTic() override {
			// update all cvars
			for ( uint i = 0; i < m_CvarCache.Count(); i++ ) {
				m_CvarCache[i].m_Handle.Update();
				if ( m_CvarCache[i].m_nModificationCount != m_CvarCache[i].m_Handle.GetModificationCount()
					&& m_CvarCache[i].m_Handle.Track() )
				{
					ConsolePrint( "Changed \"" + m_CvarCache[i].m_Handle.GetName() + "\" to \""
						+ m_CvarCache[i].m_Handle.GetValue() + "\"\n" );
				}
			}
		}
		void OnLevelStart() {
		}
		void OnLevelEnd() {
		}
		const string& GetName() const {
			return "CvarSystem";
		}
		
		void DrawCvarList() {
		}
		
		private array<CvarTableEntry@> m_CvarCache;
	};

	CvarSystem@ CvarManager;
};