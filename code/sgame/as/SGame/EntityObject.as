#include "SGame/EntityState.as"
#include "SGame/StateSystem.as"
#include "SGame/PlayrObject.as"

namespace TheNomad::SGame {
	const uint ENTITYNUM_INVALID = uint( ~0 );

    class EntityObject {
		EntityObject( TheNomad::GameSystem::EntityType type, uint id, const vec3& in origin ) {
			Init( type, id, origin );
		}
		EntityObject() {
		}
		
		void Init( TheNomad::GameSystem::EntityType type, uint id, const vec3& in origin ) {
			// just create a temporary bbox to link it in, we'll rebuild every frame anyway
			TheNomad::GameSystem::BBox bounds( 1.0f, 1.0f, origin );
			m_Link = TheNomad::GameSystem::LinkEntity( origin, bounds, id, uint( type ) );
		}
		
		//
		// getters
		//
		float GetHealth() const {
			return m_nHealth;
		}
		const string& GetName() const {
			return m_Name;
		}
		const EntityState@ GetState() const {
			return @m_State;
		}
		EntityState@ GetState() {
			return @m_State;
		}
		const ref@ GetData() const {
			return @m_Data;
		}
		ref@ GetData() {
			return @m_Data;
		}
		int GetShader() const {
			return m_hShader;
		}
		uint GetFlags() const {
			return m_Flags;
		}
		int GetFacing() const {
			return m_Facing;
		}
		uint GetId() const {
			return m_Link.m_nEntityId;
		}
		uint GetEntityNum() const {
			return m_Link.m_nEntityNumber;
		}
		const TheNomad::GameSystem::BBox& GetBounds() const {
			return m_Link.m_Bounds;
		}
		const TheNomad::GameSystem::LinkEntity& GetLink() const {
			return m_Link;
		}
		TheNomad::GameSystem::LinkEntity& GetLink() {
			return m_Link;
		}
		TheNomad::GameSystem::DirType GetDirection() const {
			return m_Direction;
		}
		const InfoSystem::InfoLoader@ GetInfo() const {
			return @m_InfoData;
		}
		const vec3& GetOrigin() const {
			return m_Link.m_Origin;
		}
		const vec3& GetVelocity() const {
			return m_Velocity;
		}
		vec3& GetVelocity() {
			return m_Velocity;
		}
		TheNomad::GameSystem::EntityType GetType() const {
			return TheNomad::GameSystem::EntityType( m_Link.m_nEntityType );
		}
		bool IsProjectile() const {
			return m_bProjectile;
		}
		SpriteSheet@ GetSpriteSheet() {
			return @m_SpriteSheet;
		}
		const SpriteSheet@ GetSpriteSheet() const {
			return @m_SpriteSheet;
		}

		//
		// setters
		//
		void SetHealth( float nHealth ) {
			m_nHealth = nHealth;
		}
		void SetState( uint statenum ) {
			
		}
		void SetState( EntityState@ state ) {
			
		}
		void SetFlags( uint flags ) {
			m_Flags = EntityFlags( flags );
		}
		void SetProjectile( bool bProjectile ) {
			m_bProjectile = bProjectile;
		}
		void SetVelocity( const vec3& in vel ) {
			m_Velocity = vel;
		}
		void SetOrigin( const vec3& in origin ) {
			m_Link.m_Origin = origin;
		}
		void SetDirection( TheNomad::GameSystem::DirType dir ) {
			m_Direction = dir;
		}
		void SetFacing( int facing ) {
			m_Facing = facing;
		}

		//
		// functions that should be implemented by the derived class
		//

		protected void LoadBase( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) {
			m_Link.m_Origin = section.LoadVec3( "origin" );
			m_nHealth = section.LoadFloat( "health" );
			m_Flags = EntityFlags( section.LoadUInt( "flags" ) );
			m_nAngle = section.LoadFloat( "angle" );
			m_Facing = section.LoadInt( "torsoFacing" );
			m_Direction = TheNomad::GameSystem::DirType( section.LoadUInt( "direction" ) );
			m_bProjectile = Convert().ToBool( section.LoadUInt( "isProjectile" ) );
		}

		protected void SaveBase( const TheNomad::GameSystem::SaveSystem::SaveSection& in section ) const {
			section.SaveVec3( "origin", m_Link.m_Origin );
			section.SaveFloat( "health", m_nHealth );
			section.SaveUInt( "flags", uint( m_Flags ) );
			section.SaveFloat( "angle", m_nAngle );
			section.SaveInt( "torsoFacing", m_Facing );
			section.SaveUInt( "direction", uint( m_Direction ) );
			section.SaveUInt( "isProjectile", Convert().ToUInt( m_bProjectile ) );
			section.SaveUInt( "type", uint( m_Link.m_nEntityType ) );
			section.SaveUInt( "id",  m_Link.m_nEntityId );
		}
		
		//
		// EntityObject::Load: should only return false if we're missing something
		// in the save data
		//
		bool Load( const TheNomad::GameSystem::SaveSystem::LoadSection& in section ) {
			ConsoleWarning( "EntityObject::Load: called\n" );
			return true;
		}
		void Save( const TheNomad::GameSystem::SaveSystem::SaveSection& in section ) const {
			ConsoleWarning( "EntityObject::Save: callled\n" );
		}
		void Damage( float nAmount ) {
			ConsoleWarning( "EntityObject::Damage: called\n" );
		}
		void Think() {
			ConsoleWarning( "EntityObject::Think: called\n" );
		}
		void Spawn( uint, const vec3& in ) {
			ConsoleWarning( "EntityObject::Spawn: called\n" );
		}

		//
		// utilities
		//
		bool CheckFlags( uint flags ) const {
			return ( m_Flags & flags ) != 0;
		}
		
		// the entity's current state
		protected EntityState@ m_State = null;
		
		// can only be a reference to a class that inherits from EntityObject, otherwise, it'll crash
		protected ref@ m_Data = null;
		
		// engine data, for physics
		protected TheNomad::GameSystem::LinkEntity m_Link;
		
		// mostly meant for debugging
		protected string m_Name;
		
		// need for speed
		protected vec3 m_Velocity = vec3( 0.0f );
		
		// current effect the entity's suffereing from
		protected AttackEffect m_Debuff = AttackEffect::None;
		
		// DUH.
		protected float m_nHealth = 0.0f;
		
		// flags, some are specific
		protected EntityFlags m_Flags = EntityFlags::None;
		
		// angle's really only used for telling direction
		protected float m_nAngle = 0.0f;
		protected TheNomad::GameSystem::DirType m_Direction = TheNomad::GameSystem::DirType::North;
		
		// is it a projectile?
		protected bool m_bProjectile = false;
		
		// for direction based sprite drawing
		protected int m_Facing = 0;
		
		// cached info
		protected const InfoSystem::InfoLoader@ m_InfoData = null;
		
		//
		// renderer data
		//
		protected int m_hShader = FS_INVALID_HANDLE;
		protected SpriteSheet@ m_SpriteSheet = null;
		
		// linked list stuff
		EntityObject@ next = null;
		EntityObject@ prev = null;
	};
};