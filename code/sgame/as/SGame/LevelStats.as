namespace TheNomad::SGame {
    class LevelStats {
		LevelStats() {
		}
		LevelStats() {
		}
		
		void Draw( bool endOfLevel, const TheNomad::Engine::Timer& in timer ) {
			ImGuiWindowFlags windowFlags = ImGui::MakeWindowFlags( ImGuiWindowFlags::NoTitleBar | ImGuiWindowFlags::NoMove | ImGuiWindowFlags::NoResize );

			if ( !TheNomad::Engine::IsKeyDown( TheNomad::Engine::KeyNum::Key_Tab ) ) {
				return;
			}

			m_TimeMilliseconds = timer.ElapsedMilliseconds();
			m_TimeSeconds = timer.ElapsedSeconds();
			m_TimeMinutes = timer.ElapsedMinutes();
			
			ImGui::Begin( "##LevelStatsShow", null, windowFlags );
			if ( endOfLevel ) {
				time.Stop();
				if ( TheNomad::Engine::IsAnyKeyDown() ) {
					selectedSfx.Play();
				}
			}
			
			ImGui::BeginTable( "##LevelStatsNumbers", 2 );
			
			//
			// time
			//
			ImGui::TableNextColumn();
			ImGui::Text( "TIME:" );
			ImGui::TableNextColumn();

			ImGui::Text( m_TimeMinutes + ":" + m_TimeSeconds + "." + m_TimeMilliseconds );
			ImGui::TextColored( sgame_RankStringColors[ time_Rank ], sgame_RankStrings[ time_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// kills
			//
			ImGui::TableNextColumn();
			ImGui::Text( "KILLS:" );
			ImGui::TableNextColumn();
			ImGui::Text( numKills );
			ImGui::TextColored( sgame_RankStringColors[ kills_Rank ], sgame_RankStrings[ kills_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// deaths
			//
			ImGui::TableNextColumn();
			ImGui::Text( "DEATHS:" );
			ImGui::TableNextColumn();
			ImGui::Text( numDeaths );
			ImGui::TextColored( sgame_RankStringColors[ deaths_Rank ], sgame_RankStrings[ deaths_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// style
			//
			ImGui::TableNextColumn();
			ImGui::Text( "STYLE:" );
			ImGui::TableNextColumn();
			ImGui::Text( stylePoints );
			ImGui::TextColored( sgame_RankStringColors[ style_Rank ], sgame_RankStrings[ style_Rank ] );
			
			ImGui::EndTable();
			
			ImGui::End();
		}

		uint m_TimeMilliseconds = 0;
		uint m_TimeSeconds = 0;
		uint m_TimeMinutes = 0;

		uint stylePoints = 0;
		uint numKills = 0;
		uint numDeaths = 0;
		uint collateralScore = 0;
		bool isClean = true;

		LevelRank time_Rank = LevelRank::RankS;
		LevelRank style_Rank = LevelRank::RankS;
		LevelRank kills_Rank = LevelRank::RankS;
		LevelRank deaths_Rank = LevelRank::RankS;
	};
};
