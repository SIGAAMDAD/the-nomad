#include "main.as"

namespace TheNomad {
	//
	// ConVar: an implementation of the vmCvar_t, but to make it easier to manage,
	// its been put here. This is a shared object
	// DO NOT MODIFY
	//
	shared class ConVar
	{
		ConVar() {
		}
		ConVar( const string& in name, const string& in value, uint flags, bool bTrackChanges ) {
			m_Name = name;
			m_Value = value;
			m_Flags = flags;
			m_bTrackChanges = bTrackChanges;
			Cvar_Register( name, value, flags, m_IntValue, m_FloatValue, m_nModificationCount, m_nCvarHandle );
		}
		
		void Register( const string& in name, const string& in value, uint flags, bool bTrackChanges ) {
			m_Name = name;
			m_Value = value;
			m_Flags = flags;
			m_bTrackChanges = bTrackChanges;
			Cvar_Register( name, value, flags, m_IntValue, m_FloatValue, m_nModificationCount, m_nCvarHandle );
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
		
		void Set( const string& value ) {
			Cvar_Set( m_Name, m_Value );
		}
		void Update() {
			Cvar_Update( m_Name );
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
	
	class CvarSystem {
		CvarSystem() {
			TheNomad::Engine::CmdArgs::AddCommand( "sgame.list_cvars", this.ListVars_f );
		}
		
		ConVar@ AddCvar( const string& in name, const string& in value, uint flags, bool bTrackChanges ) {
			ConVar@ var = ConVar( name, value, flags, bTrackChanges );
			m_CvarCache.emplace_back( var );
			return var;
		}
		
		void ListVars_f() {
			ConsolePrint( "VM " + ModuleInfo.GetName() + " cvars:\n" );
			
			for ( int i = 0; i < m_CvarCache.size(); i++ ) {
				ConsolePrint( m_CvarCache[i].GetName() + " " + m_CvarCache[i].GetValue() );
			}
		}
		
		void DrawCvarList() {
			ImGui::Window window;
		}
		
		private array<ConVar@> m_CvarCache;
	};
	
	CvarSystem@ CvarManager = cast<CvarSystem>( TheNomad::GameSystem::AddSystem( CvarSystem() ) );
};