#include "SGame/MapCheckpoint.as"
#include "SGame/MapSpawn.as"
#include "SGame/MapSecret.as"

namespace TheNomad::SGame {
    class MapData {
		MapData() {
		}

		void Init( const string& in mapName, uint nMapLevels ) {
			m_nWidth = 0;
			m_nHeight = 0;
			m_Name = mapName;
		}
		
		private int GetCheckpointIndex( const MapCheckpoint& in cp ) const {
			return m_Checkpoints.Find( cp );
		}
		
		/*
		//
		// SaveCheckpointCache: creates a cachefile containing spawn to checkpoint bindings
		// to reduce map loading time
		//
		void SaveMapCache() const {
			TheNomad::Engine::FileSystem::OutputStream file;
			const uint mapCacheMagic = 0xffa53dfe;
			
			if ( file.Open( "_cache/" + m_Name + "_mcd.dat" ) == false ) {
				DebugPrint( "ERROR: failed to create a checkpoint cache for '" + m_Name + "'!\n" );
				return;
			}
			
			DebugPrint( "Saving cached checkpoint data...\n" );
			
			file.WriteUInt( mapCacheMagic );
			file.WriteUInt( m_Spawns.Count() );
			file.WriteUInt( m_Checkpoints.Count() );
			
			for ( uint i = 0; i < m_Spawns.Count(); i++ ) {
				file.WriteUInt( GetCheckpointIndex( m_Spawns[i].m_Checkpoint ) );
			}
		}
		
		bool LoadCheckpointCache() {
			TheNomad::Engine::FileSystem::InputStream file;
			uint magic;
			uint num, index;
			const uint mapCacheMagic = 0xffa53dfe;
			
			if ( file.Open( "_cache/" + m_Name + "_mdc.dat" ) == false ) {
				ConsolePrint( "no map data cache found.\n" );
				return false;
			}
			
			file.ReadUInt( magic );
			if ( magic != mapCacheMagic ) {
				ConsoleWarning( "LoadCheckpointCache( " + m_Name + " ): found data cache file, but wrong magic! refusing to load.\n" );
				return false;
			}
			
			file.ReadUInt( num );
			if ( num != m_Spawns.Count() ) {
				ConsolePrint( "spawn count in cache file isn't the same as the one loaded, outdated? refusing to load.\n" );
				return false;
			}
			file.ReadUInt( num );
			if ( num != m_Checkpoints.Count() ) {
				ConsolePrint( "checkpoint count in cache file isn't the same as the one loaded, outdated? refusing to load.\n" );
				return false;
			}
			
			for ( uint i = 0; i < m_Spawns.Count(); i++ ) {
				file.ReadUInt( index );
				if ( index >= m_Checkpoints.Count() ) {
					ConsolePrint( "checkpoint index in data cache file isn't valid, corruption? refusing to load.\n" );
					return false;
				}
				@m_Spawns[i].m_Checkpoint = @m_Checkpoints[index];
			}
			
			return true;
		}
		*/

		void Load( int hMap ) {
			uint nCheckpoints, nSpawns, nTiles;
			uint i;
			uvec3 xyz;
			
			TheNomad::GameSystem::SetActiveMap( hMap, nCheckpoints, nSpawns,
				nTiles );

			TheNomad::GameSystem::GetTileData( @m_TileData );
			TheNomad::Engine::Renderer::LoadWorld( m_Name );
			
			//
			// load the checkpoints
			//
			for ( i = 0; i < nCheckpoints; i++ ) {
				MapCheckpoint cp;
				
				TheNomad::GameSystem::GetCheckpointData( xyz, i );
				cp = MapCheckpoint( xyz );
				
				m_Checkpoints.Add( cp );
			}
			
			//
			// load in spawns
			//
			for ( i = 0; i < nSpawns; i++ ) {
				MapSpawn spawn;
				uint id, type, checkpoint;
				
				TheNomad::GameSystem::GetSpawnData( xyz, type, id, i, checkpoint );
				spawn = MapSpawn( xyz, id, TheNomad::GameSystem::EntityType( type ) );

				DebugPrint( "Spawn " + i + " linked to checkpoint " + checkpoint + "\n" );
				@spawn.m_Checkpoint = @m_Checkpoints[ checkpoint ];
				
				m_Spawns.Add( spawn );
			}
			
			/*
//			if ( !LoadCheckpointCache() ) {
				// generate the indexes manually

				float dist;
				MapCheckpoint@ cp;
				uint c;
				
				for ( i = 0; i < nSpawns; i++ ) {
					dist = TheNomad::Util::Distance( m_Spawns[i].m_Origin, m_Checkpoints[0].m_Origin );
					for ( c = 0; c < nCheckpoints; c++ ) {
						@cp = @m_Checkpoints[c];
						if ( TheNomad::Util::Distance( cp.m_Origin, m_Spawns[i].m_Origin ) < dist ) {
							dist = TheNomad::Util::Distance( cp.m_Origin, m_Spawns[i].m_Origin );
						}
					}

					if ( @cp is null ) {
						DebugPrint( "spawn without a checkpoint, discarding.\n" );
						continue;
					}
					cp.AddSpawn( @m_Spawns[i] );
					DebugPrint( "Spawn " + i + " linked to checkpoint " + c + "\n" );
				}
//			}
			*/

			DebugPrint( "Map \"" + m_Name + "\" loaded with " + m_Checkpoints.Count() + " checkpoints, " +
				m_Spawns.Count() + " spawns.\n" );
		}
		
		const array<array<uint>>@ GetTiles() const {
			return @m_TileData;
		}
		array<array<uint>>@ GetTiles() {
			return @m_TileData;
		}
		
		uint NumLevels() const {
			return m_TileData.Count();
		}

		const array<MapSpawn>& GetSpawns() const {
			return m_Spawns;
		}
		const array<MapCheckpoint>& GetCheckpoints() const {
			return m_Checkpoints;
		}
		array<MapSpawn>& GetSpawns() {
			return m_Spawns;
		}
		array<MapCheckpoint>& GetCheckpoints() {
			return m_Checkpoints;
		}
		int GetWidth() const {
			return m_nWidth;
		}
		int GetHeight() const {
			return m_nHeight;
		}
		
		private string m_Name;
		private array<MapSecret> m_Secrets;
		private array<MapSpawn> m_Spawns;
		private array<MapCheckpoint> m_Checkpoints;
		private array<array<uint>> m_TileData;
		private int m_nWidth = 0;
		private int m_nHeight = 0;
	};
};