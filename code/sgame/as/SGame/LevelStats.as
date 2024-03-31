namespace TheNomad::SGame {
    class LevelStats {
		LevelStats() {
			drawString.reserve( MAX_STRING_CHARS );
			time.Run();
		}
		LevelStats() {
		}
		
		void Draw( bool endOfLevel ) {
			ImGuiWindowFlags windowFlags = ImGui::MakeWindowFlags( ImGuiWindowFlags::NoTitleBar | ImGuiWindowFlags::NoMove | ImGuiWindowFlags::NoResize );

			if ( !TheNomad::Engine::IsKeyDown( TheNomad::Engine::KeyNum::Key_Tab ) ) {
				return;
			}
			
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
			
			drawString = time.ElapsedMinutes();
			drawString += ":";
			drawString += time.ElapsedSeconds();
			drawString += ".";
			drawString += time.ElapsedMilliseconds();
			ImGui::Text( drawString );
			ImGui::TextColored( sgame_RankStringColors[ time_Rank ], sgame_RankStrings[ time_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// kills
			//
			ImGui::TableNextColumn();
			ImGui::Text( "KILLS:" );
			ImGui::TableNextColumn();
			drawString = numKills;
			ImGui::Text( drawString );
			ImGui::TextColored( sgame_RankStringColors[ kills_Rank ], sgame_RankStrings[ kills_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// deaths
			//
			ImGui::TableNextColumn();
			ImGui::Text( "DEATHS:" );
			ImGui::TableNextColumn();
			drawString = numDeaths;
			ImGui::Text( drawString );
			ImGui::TextColored( sgame_RankStringColors[ deaths_Rank ], sgame_RankStrings[ deaths_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// style
			//
			ImGui::TableNextColumn();
			ImGui::Text( "STYLE:" );
			ImGui::TableNextColumn();
			drawString = stylePoints;
			ImGui::Text( drawString );
			ImGui::TextColored( sgame_RankStringColors[ style_Rank ], sgame_RankStrings[ style_Rank ] );
			
			ImGui::EndTable();
			
			ImGui::End();
		}
		
		private string drawString;
		TheNomad::Engine::Timer time;
		uint stylePoints = 0;
		uint numKills = 0;
		uint numDeaths = 0;
		uint collateralScore = 0;
		bool isClean = true;

		LevelRank time_Rank;
		LevelRank style_Rank;
		LevelRank kills_Rank;
		LevelRank deaths_Rank;
	};
};