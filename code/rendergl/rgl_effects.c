/*
===========================================================================
Copyright (C) 2023-2024 GDR Games

This file is part of The Nomad source code.

The Nomad source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

The Nomad source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "rgl_local.h"

typedef struct gfx_s {
	vec3_t origin;
	vec3_t velocity;
	struct gfx_s *next;
	struct gfx_s *prev;
	uint32_t startTime;
	uint32_t lifeTime;
	float rotation;
	nhandle_t hShader;
	qboolean gravity;
} gfx_t;

void R_InitEffects( void )
{

}


void OnRenderScene() {
			TheNomad::Engine::Renderer::LocalEntity@ ent;
			TheNomad::Engine::Renderer::LocalEntity@ next;

			TheNomad::Engine::ProfileBlock block( "GfxSystem::OnRenderScene" );

			// walk the list backwards, so any new local entities generated
			// (trails, marks, etc) will be present this frame
			@ent = @m_ActiveLocalEnts.m_Prev;
			for ( ; @ent !is @m_ActiveLocalEnts; @ent = @next ) {
				// grab next now, so if the local entity is freed we
				// still have it
				@next = @ent.m_Prev;

				if ( TheNomad::GameSystem::GameManager.GetGameTic() - ent.m_nStartTime >= ent.m_nLifeTime ) {
					FreeLocalEntity( @ent );
					continue;
				}

				ent.RunTic();
			}
		}

		private void InitLocalEntities() {
			const uint numGfx = 512;
			m_LocalEnts.Resize( numGfx );

			@m_ActiveLocalEnts.m_Next = @m_ActiveLocalEnts;
			@m_ActiveLocalEnts.m_Prev = @m_ActiveLocalEnts;
			@m_FreeLocalEnts = @m_LocalEnts[0];

			for ( uint i = 0; i < m_LocalEnts.Count() - 1; i++ ) {
				@m_LocalEnts[i].m_Next = @m_LocalEnts[i + 1];
			}
		}

		private TheNomad::Engine::Renderer::LocalEntity@ AllocLocalEntity() {
			TheNomad::Engine::Renderer::LocalEntity@ ent;
			uint time;

			if ( @m_FreeLocalEnts is null ) {
				// no free polys, so free the one at the end of the chain
				// remove the oldest active entity
				FreeLocalEntity( @m_ActiveLocalEnts.m_Prev );
			}

			@ent = @m_FreeLocalEnts;
			@m_FreeLocalEnts = @m_FreeLocalEnts.m_Next;

			// link into active list
			@ent.m_Next = @m_ActiveLocalEnts.m_Next;
			@ent.m_Prev = @m_ActiveLocalEnts;
			@m_ActiveLocalEnts.m_Next.m_Prev = @ent;
			@m_ActiveLocalEnts.m_Next = @ent;

			return @ent;
		}
		//
		// FreeLocalEntity: only LocalEntity when its finished should ever call this, or AllocLocalEntity
		//
		private void FreeLocalEntity( TheNomad::Engine::Renderer::LocalEntity@ ent ) {
			if ( @ent.m_Prev is null ) {
				GameError( "GfxManager::FreeLocalEntity: not active" );
			}

			// remove from doubly linked list
			@ent.m_Prev.m_Next = @ent.m_Next;
			@ent.m_Next.m_Prev = @ent.m_Prev;

			// the free list is only singly linked
			@ent.m_Next = @m_FreeLocalEnts;
			@m_FreeLocalEnts = @ent;
		}


		//
		// GfxSystem::Bleed: this is a spurt of blood when an entity gets hit
		//
		TheNomad::Engine::Renderer::LocalEntity@ Bleed( const vec3& in origin ) {
			TheNomad::Engine::Renderer::LocalEntity@ ent = null;

			if ( sgame_Blood.GetInt() == 0 ) {
				return null;
			}

			float x, y;
			const uint randX = Util::PRandom();
			const uint randY = Util::PRandom();

			@ent = AllocLocalEntity();

			x = origin.x - ( 1.25f / ( randX == 0 ? 1 : randX ) );
			y = origin.y - ( 1.25f / ( randY == 0 ? 1 : randY ) );

			ent.m_nStartTime = TheNomad::GameSystem::GameManager.GetGameTic();
			ent.m_nLifeTime = 1000;
			ent.m_Velocity = vec3( 0.05f, -0.03f, 0.0f );

			ent.m_Origin = vec3( x, y - 0.2f, 0.0f );
			ent.m_hShader = TheNomad::Engine::ResourceCache.GetShader( "gfx/bloodSplatter0" );
			ent.m_bGravity = true;

			return @ent;
		}

		void SmokePuff( const vec3& in origin, const vec3& in vel ) {
		}

		void AddDustPoly( const vec3& in origin, const vec3& in vel, uint lifeTime, int hShader ) {
			TheNomad::Engine::Renderer::LocalEntity@ ent;

			@ent = AllocLocalEntity();

			ent.m_nStartTime = TheNomad::GameSystem::GameManager.GetGameTic();
			ent.m_nLifeTime = lifeTime;
			ent.m_Velocity = vel;

			ent.m_bGravity = false;
			ent.m_Origin = origin;
			ent.m_hShader = hShader;
		}
		
		void AddMarkPoly() {
		}
		void AddSmokePoly() {
		}
		void AddFlarePoly() {
		}

		void AddExplosionGfx( const vec3& in origin ) {
		}

		private array<TheNomad::Engine::Renderer::LocalEntity> m_LocalEnts;
		private TheNomad::Engine::Renderer::LocalEntity m_ActiveLocalEnts;
		private TheNomad::Engine::Renderer::LocalEntity@ m_FreeLocalEnts = null;

		// a single pre-allocated array of polys cuz angelscript won't let me use
		// stack allocated arrays
		private TheNomad::Engine::Renderer::PolyVert[] m_DrawVerts( 4 );

		private bool m_bAllowGfx = true;