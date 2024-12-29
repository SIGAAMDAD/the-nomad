#include "SGame/MapCheckpoint.as"
#include "SGame/MapSpawn.as"
#include "SGame/MapSecret.as"

namespace TheNomad::SGame {
	uint NumLevels() {
		return MapTileData.Count();
	}
	uint GetLevel( const TheNomad::Engine::Physics::Bounds& in bounds ) {
		const uint level = uint( floor( bounds.m_nMins.z + bounds.m_nMaxs.z ) );
		return 0;
	}
	uint64 GetTile( const vec3& in origin, const TheNomad::Engine::Physics::Bounds& in bounds ) {
		const uint level = GetLevel( bounds );
		const int x = int( ceil( origin.x ) );
		const int y = int( ceil( origin.y ) );
		if ( level >= MapTileData.Count() || x < 0 || y < 0 || x >= MapWidth || y >= MapHeight ) {
			return 0;
		}
		return MapTileData[ level ][ y * MapWidth + x ];
	}

	int MapWidth = 0;
	int MapHeight = 0;
	array<array<uint64>> MapTileData;
	array<MapSpawn> MapSpawns;
	array<MapCheckpoint> MapCheckpoints;
	array<MapSecret> MapSecrets;

	void LoadMapData( int hMap ) {
		MapWidth = 0;
		MapHeight = 0;

		uint nCheckpoints, nSpawns, nTiles;
		TheNomad::GameSystem::SetActiveMap( hMap, nCheckpoints, nSpawns, nTiles, MapWidth, MapHeight );
		TheNomad::GameSystem::GetTileData( @MapTileData );
		TheNomad::Engine::Renderer::LoadWorld( TheNomad::Engine::CvarVariableString( "mapname" ) );
			
		//
		// load the checkpoints
		//
		MapCheckpoints.Reserve( nCheckpoints );
		for ( uint i = 0; i < nCheckpoints; ++i ) {
			uvec2 areaLock = uvec2( 0 );
			uvec3 xyz = uvec3( 0 );

			TheNomad::GameSystem::GetCheckpointData( xyz, areaLock, i );
			MapCheckpoints.Add( MapCheckpoint( xyz, areaLock, i ) );
		}
		
		//
		// load in spawns
		//
		MapSpawns.Reserve( nSpawns );
		for ( uint i = 0; i < nSpawns; ++i ) {
			MapSpawn spawn;
			uint id = 0;
			uint type = 0;
			uint checkpoint = 0;
			uvec3 xyz = uvec3( 0 );
				
			TheNomad::GameSystem::GetSpawnData( xyz, type, id, i, checkpoint );
			spawn = MapSpawn( xyz, id, TheNomad::GameSystem::EntityType( type ) );

			DebugPrint( "Spawn " + i + " at [ " + xyz.x + ", " + xyz.y + " ] linked to checkpoint " + checkpoint + "\n" );
			@spawn.m_Checkpoint = @MapCheckpoints[ checkpoint ];
			spawn.m_Checkpoint.m_Spawns.Add( @spawn );

			MapSpawns.Add( spawn );
		}

		DebugPrint( "Map \"" + TheNomad::Engine::CvarVariableString( "mapname" ) + "\" loaded with "
			+ MapCheckpoints.Count() + " checkpoints, "
			+ MapSpawns.Count() + " spawns, and "
			+ MapSecrets.Count() + " secrets. "
			+ "Width is " + MapWidth + " and Height is " + MapHeight + "\n" );
	}
};