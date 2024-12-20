#include "Engine/ConVar.as"
#include "Engine/CvarTableEntry.as"

namespace TheNomad::Engine {
    class CvarSystem {
		CvarSystem() {
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @ListVars_f ), "sgame.list_cvars", false
			);
		}
		~CvarSystem() {
			for ( uint i = 0; i < m_CvarCache.Count(); i++ ) {
				@m_CvarCache[i].m_Handle = null;
			}
			m_CvarCache.Clear();
		}
		
		void AddCvar( ConVar@ cvar, const string& in name, const string& in value, uint flags, bool bTrackChanges ) {
			CvarTableEntry var = CvarTableEntry( @cvar );
			var.m_Handle.Register( name, value, flags, bTrackChanges );
			m_CvarCache.Add( var );
		}
		void UpdateCvars() {
			if ( TheNomad::SGame::sgame_DebugMode.GetBool() ) {
				ProfileBlock block( "CvarSystem::UpdateCvars" );
			}

			// update all cvars
			for ( uint i = 0; i < m_CvarCache.Count(); ++i ) {
				m_CvarCache[i].m_Handle.Update();
				if ( m_CvarCache[i].m_nModificationCount != m_CvarCache[i].m_Handle.GetModificationCount()
					&& m_CvarCache[i].m_Handle.Track() )
				{
					m_CvarCache[i].m_nModificationCount = m_CvarCache[i].m_Handle.GetModificationCount();
					ConsolePrint( "Changed \"" + m_CvarCache[i].m_Handle.GetName() + "\" to \""
						+ m_CvarCache[i].m_Handle.GetValue() + "\"\n" );
				}
			}
		}
		
		void ListVars_f() {
			ConsolePrint( "VM Cvars:\n" );

			for ( uint i = 0; i < m_CvarCache.Count(); i++ ) {
				ConsolePrint( m_CvarCache[i].m_Handle.GetName() + " " + m_CvarCache[i].m_Handle.GetValue() );
			}
		}
		
		private array<CvarTableEntry> m_CvarCache;
	};

	CvarSystem@ CvarManager = null;
};