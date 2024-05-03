namespace TheNomad::SGame {
    class LevelStats {
		LevelStats() {
		}
		LevelStats() {
		}

		private void DrawEndOfLevelStats() const {
			const float fontScale = ImGui::GetFontScale();
			const float scale = TheNomad::GameSystem::GameManager.GetUIScale();
			ImGuiWindowFlags windowFlags = ImGui::MakeWindowFlags( ImGuiWindowFlags::NoTitleBar | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoResize | ImGuiWindowFlags::AlwaysHorizontalScrollbar | ImGuiWindowFlags::AlwaysVerticalScrollbar
				| ImGuiWindowFlags::NoBringToFrontOnFocus );
			
			ImGui::Begin( "##LevelStatsShow", null, windowFlags );

			ImGui::SetWindowPos( vec2( 256 * scale, 64 * scale ) );
			ImGui::SetWindowSize( vec2( 764 * scale, 500 * scale ) );

			TheNomad::Engine::UserInterface::SetActiveFont( TheNomad::Engine::UserInterface::Font_RobotoMono );
			ImGui::SetWindowFontScale( fontScale * 1.0f );

			ImGui::Text( TheNomad::Engine::CvarVariableString( "mapname" ) );
			
			ImGui::BeginTable( "##LevelStatsNumbersEndOfLevel", 2 );
			{
				ImGui::TableNextColumn();

				ImGui::BeginTable( "##LevelStatsIndividualEndOfLevel", 3 );
				{
					ImGui::SetWindowFontScale( ( fontScale * 2.0f ) );

					ImGui::TableNextColumn();
					ImGui::Text( "TIME" );
					ImGui::TableNextColumn();
					ImGui::Text( formatUInt( m_TimeMinutes ) + ":" + m_TimeSeconds + "." + m_TimeMilliseconds );
					ImGui::PushStyleColor( ImGuiCol::Text, sgame_RankStringColors[ time_Rank ] );
					ImGui::TableNextColumn();
					ImGui::Text( sgame_RankStrings[ time_Rank ] );
					ImGui::PopStyleColor();

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::Text( "KILLS" );
					ImGui::TableNextColumn();
					ImGui::Text( formatUInt( numKills ) );
					ImGui::PushStyleColor( ImGuiCol::Text, sgame_RankStringColors[ kills_Rank ] );
					ImGui::TableNextColumn();
					ImGui::Text( sgame_RankStrings[ kills_Rank ] );
					ImGui::PopStyleColor();

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::Text( "STYLE" );
					ImGui::TableNextColumn();
					ImGui::Text( formatUInt( stylePoints ) );
					ImGui::PushStyleColor( ImGuiCol::Text, sgame_RankStringColors[ style_Rank ] );
					ImGui::TableNextColumn();
					ImGui::Text( sgame_RankStrings[ style_Rank ] );
					ImGui::PopStyleColor();
				}
				ImGui::EndTable();

				ImGui::TableNextColumn();

				ImGui::PushStyleColor( ImGuiCol::Text, sgame_RankStringColors[ total_Rank ] );
				ImGui::SetWindowFontScale( ( fontScale * 4.5f ) );
				ImGui::Text( sgame_RankStrings[ total_Rank ] );
				ImGui::SetWindowFontScale( ( fontScale * 2.0f ) );
				ImGui::PopStyleColor();

				ImGui::TableNextRow();

				ImGui::TableNextColumn();

				ImGui::Text( "- " );
				ImGui::SameLine();
				ImGui::PushStyleColor( ImGuiCol::Text, colorRed );
				ImGui::Text( formatUInt( collateralScore ) );
				ImGui::PopStyleColor();
				ImGui::SameLine();
				ImGui::Text( " COLLATERAL AMOUNT" );

				ImGui::Text( "- " );
				ImGui::SameLine();
				ImGui::PushStyleColor( ImGuiCol::Text, colorRed );
				ImGui::Text( formatUInt( numDeaths ) );
				ImGui::PopStyleColor();
				ImGui::Text( " DEATHS" );

				ImGui::TableNextColumn();

				// TODO:
				ImGui::Text( "CHALLENGE: " );

				if ( isClean ) {
					ImGui::Text( "- " );
					ImGui::SameLine();
					ImGui::PushStyleColor( ImGuiCol::Text, colorGreen );
					ImGui::Text( "CLEAN RUN" );
					ImGui::PopStyleColor();
				}
				
				ImGui::Text( "TOTAL " + totalScore );
			}
			ImGui::EndTable();
			
			if ( ImGui::Button( "DONE" ) ) {
				selectedSfx.Play();
				GlobalState = GameState::EndOfLevel;
			}

			ImGui::End();
		}
		
		void Draw( bool endOfLevel, const TheNomad::Engine::Timer& in timer ) {
			ImGuiWindowFlags windowFlags = ImGui::MakeWindowFlags( ImGuiWindowFlags::NoTitleBar | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoResize );
			const float scale = TheNomad::GameSystem::GameManager.GetUIScale();

			if ( !TheNomad::Engine::IsKeyDown( TheNomad::Engine::KeyNum::Tab )
				&& !TheNomad::Engine::IsKeyDown( TheNomad::Engine::KeyNum::GamePad_Guide )
				&& GlobalState != GameState::StatsMenu )
			{
				return;
			}

			m_TimeMilliseconds = timer.ElapsedMilliseconds();
			m_TimeSeconds = timer.ElapsedSeconds();
			m_TimeMinutes = timer.ElapsedMinutes();
			
			if ( endOfLevel ) {
				DrawEndOfLevelStats();
				return;
			}

			ImGui::Begin( "##LevelStatsShow", null, windowFlags );
			ImGui::SetWindowPos( vec2( 16 * scale, 16 * scale ) );
			ImGui::SetWindowSize( vec2( 256 * scale, 72 * scale ) );
			
			ImGui::BeginTable( "##LevelStatsNumbers", 2 );
			
			//
			// time
			//
			ImGui::TableNextColumn();
			ImGui::Text( "TIME:" );
			ImGui::TableNextColumn();

			ImGui::Text( formatUInt( m_TimeMinutes ) + ":" + m_TimeSeconds + "." + m_TimeMilliseconds );
			ImGui::SameLine();
			ImGui::TextColored( sgame_RankStringColors[ time_Rank ], sgame_RankStrings[ time_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// kills
			//
			ImGui::TableNextColumn();
			ImGui::Text( "KILLS:" );
			ImGui::TableNextColumn();
			ImGui::Text( formatUInt( numKills ) );
			ImGui::SameLine();
			ImGui::TextColored( sgame_RankStringColors[ kills_Rank ], sgame_RankStrings[ kills_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// deaths
			//
			ImGui::TableNextColumn();
			ImGui::Text( "DEATHS:" );
			ImGui::TableNextColumn();
			ImGui::Text( formatUInt( numDeaths ) );
			ImGui::SameLine();
			ImGui::TextColored( sgame_RankStringColors[ deaths_Rank ], sgame_RankStrings[ deaths_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// style
			//
			ImGui::TableNextColumn();
			ImGui::Text( "STYLE:" );
			ImGui::TableNextColumn();
			ImGui::Text( formatUInt( stylePoints ) );
			ImGui::SameLine();
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
		uint totalScore = 0;

		LevelRank total_Rank = LevelRank::RankS;
		LevelRank time_Rank = LevelRank::RankS;
		LevelRank style_Rank = LevelRank::RankS;
		LevelRank kills_Rank = LevelRank::RankS;
		LevelRank deaths_Rank = LevelRank::RankS;
	};
};
