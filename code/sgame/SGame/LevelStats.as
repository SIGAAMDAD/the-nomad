namespace TheNomad::SGame {
    class LevelStats {
		LevelStats() {
		}
		LevelStats() {
		}

		void CalcLevelStats() {
			LevelInfoData@ data = @LevelManager.GetCurrentData();
			LevelRank rank;

			rank = LevelRank::RankS;
			if ( numKills < data.m_RankS.minKills ) {
				rank = LevelRank::RankA;
			}
			if ( numKills < data.m_RankA.minKills ) {
				rank = LevelRank::RankB;
			}
			if ( numKills < data.m_RankB.minKills ) {
				rank = LevelRank::RankC;
			}
			if ( numKills < data.m_RankC.minKills ) {
				rank = LevelRank::RankD;
			}
			if ( numKills < data.m_RankD.minKills ) {
				rank = LevelRank::RankF;
			}
			if ( numKills < data.m_RankF.minKills ) {
				rank = LevelRank::RankWereUBotting;
			}
			kills_Rank = rank;

			rank = LevelRank::RankS;
			if ( m_TimeMilliseconds > data.m_RankS.minTime ) {
				rank = LevelRank::RankA;
			}
			if ( m_TimeMilliseconds > data.m_RankA.minTime ) {
				rank = LevelRank::RankB;
			}
			if ( m_TimeMilliseconds > data.m_RankB.minTime ) {
				rank = LevelRank::RankC;
			}
			if ( m_TimeMilliseconds > data.m_RankC.minTime ) {
				rank = LevelRank::RankD;
			}
			if ( m_TimeMilliseconds > data.m_RankD.minTime ) {
				rank = LevelRank::RankF;
			}
			if ( m_TimeMilliseconds > data.m_RankF.minTime ) {
				rank = LevelRank::RankWereUBotting;
			}
			time_Rank = rank;

			rank = LevelRank::RankS;
			if ( numDeaths > data.m_RankS.maxDeaths ) {
				rank = LevelRank::RankA;
			}
			if ( numDeaths > data.m_RankA.maxDeaths ) {
				rank = LevelRank::RankB;
			}
			if ( numDeaths > data.m_RankB.maxDeaths ) {
				rank = LevelRank::RankC;
			}
			if ( numDeaths > data.m_RankC.maxDeaths ) {
				rank = LevelRank::RankD;
			}
			if ( numDeaths > data.m_RankD.maxDeaths ) {
				rank = LevelRank::RankF;
			}
			if ( numDeaths > data.m_RankF.maxDeaths ) {
				rank = LevelRank::RankWereUBotting;
			}
			deaths_Rank = rank;
		}

		private bool AllRanksAre( LevelRank rank ) const {
			return ( kills_Rank == rank && deaths_Rank == rank && time_Rank == rank );
		}

		void CalcTotalLevelStats() {
			LevelRank highestRank;
			uint numHighestRanks = 0;

			CalcLevelStats();

			// add bonus
			if ( collateralScore == 0 ) {
				totalScore += 1000;
			} else {
				totalScore -= collateralScore;
			}
			if ( numDeaths == 0 ) {
				totalScore += 5000;
			} else {
				totalScore -= numDeaths * 250;
			}
			isClean = collateralScore == 0;

			if ( TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" ) > TheNomad::GameSystem::GameDifficulty::Normal ) {
				difficulty = TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" );
				totalScore += difficulty * 1000;
			}

			// absolute perfection
			if ( AllRanksAre( LevelRank::RankS ) && collateralScore == 0 ) {
				total_Rank = LevelRank::RankS;
			}

			highestRank = GetHighestRank();
			/*if ( style_Rank == highestRank ) {
				numHighestRanks++;
			}
			*/
			if ( kills_Rank == highestRank ) {
				numHighestRanks++;
				totalScore += 250;
			}
			if ( deaths_Rank == highestRank ) {
				numHighestRanks++;
				totalScore += 250;
			}
			if ( time_Rank == highestRank ) {
				numHighestRanks++;
				totalScore += 250;
			}
			if ( numHighestRanks > 1 ) {
				total_Rank = highestRank;
				totalScore += 500;
			} else {
				total_Rank = LevelRank( uint( highestRank ) - 1 );
			}

			switch ( highestRank ) {
			case LevelRank::RankS:
				totalScore += 1500;
				break;
			case LevelRank::RankA:
				totalScore += 500;
				break;
			case LevelRank::RankB:
				totalScore += 275;
				break;
			case LevelRank::RankC:
				totalScore += 150;
				break;
			case LevelRank::RankD:
				totalScore += 50;
				break;
			case LevelRank::RankF:
				totalScore += 10;
				break;
			};

			// apply collateral damages
			if ( collateralScore > 0 ) {
				ApplyCollateral();
			}

			// TODO: give massive bonus for no bullet time used
		}

		private void DrawEndOfLevelStats() const {
			const float fontScale = ImGui::GetFontScale();
			const float scale = TheNomad::GameSystem::UIScale;
			ImGuiWindowFlags windowFlags = ImGui::MakeWindowFlags( ImGuiWindowFlags::NoTitleBar | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoResize | ImGuiWindowFlags::NoBringToFrontOnFocus );
			
			ImGui::Begin( "##LevelStatsShow", null, windowFlags );

			// ensure we aren't drawing anything over this
			TheNomad::Engine::CvarSet( "g_paused", "0" );

			ImGui::SetWindowPos( vec2( 120 * scale, 64 * scale ) );
			ImGui::SetWindowSize( vec2( 1200 * scale, 500 * scale ) );

			TheNomad::Engine::UserInterface::SetActiveFont( TheNomad::Engine::UserInterface::Font_RobotoMono );
			ImGui::SetWindowFontScale( fontScale * 2.0f );

			ImGui::Text( LevelManager.GetLevelName() );
			
			ImGui::BeginTable( "##LevelStatsNumbersEndOfLevel", 2 );
			{
				ImGui::TableNextColumn();

				ImGui::BeginTable( "##LevelStatsIndividualEndOfLevel", 3 );
				{
					ImGui::SetWindowFontScale( ( fontScale * 2.5f ) );

					ImGui::TableNextColumn();
					ImGui::Text( "TIME" );
					ImGui::TableNextColumn();
					ImGui::Text( " " + m_TimeMinutes + ":" + m_TimeSeconds + "." + m_TimeMilliseconds );
					ImGui::PushStyleColor( ImGuiCol::Text, sgame_RankStringColors[ time_Rank ] );
					ImGui::TableNextColumn();
					ImGui::Text( sgame_RankStrings[ time_Rank ] );
					ImGui::PopStyleColor();

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::Text( "KILLS" );
					ImGui::TableNextColumn();
					ImGui::Text( " " + numKills );
					ImGui::PushStyleColor( ImGuiCol::Text, sgame_RankStringColors[ kills_Rank ] );
					ImGui::TableNextColumn();
					ImGui::Text( sgame_RankStrings[ kills_Rank ] );
					ImGui::PopStyleColor();

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::Text( "DEATHS" );
					ImGui::TableNextColumn();
					ImGui::Text( " " + numDeaths );
					ImGui::PushStyleColor( ImGuiCol::Text, sgame_RankStringColors[ deaths_Rank ] );
					ImGui::TableNextColumn();
					ImGui::Text( sgame_RankStrings[ deaths_Rank ] );
					ImGui::PopStyleColor();

					/*
					ImGui::TableNextColumn();
					ImGui::Text( "STYLE" );
					ImGui::TableNextColumn();
					ImGui::Text( formatUInt( stylePoints ) );
					ImGui::PushStyleColor( ImGuiCol::Text, sgame_RankStringColors[ style_Rank ] );
					ImGui::TableNextColumn();
					ImGui::Text( sgame_RankStrings[ style_Rank ] );
					ImGui::PopStyleColor();
					*/
				}
				ImGui::EndTable();

				ImGui::TableNextColumn();

				ImGui::PushStyleColor( ImGuiCol::Text, sgame_RankStringColors[ total_Rank ] );
				ImGui::SetWindowFontScale( ( fontScale * 4.5f ) );
				ImGui::Text( sgame_RankStrings[ total_Rank ] );
				ImGui::SetWindowFontScale( ( fontScale * 2.5f ) );
				ImGui::PopStyleColor();

				ImGui::TableNextColumn();

				ImGui::Separator();

				ImGui::Text( "- " );
				ImGui::SameLine();
				if ( collateralScore > 0 ) {
					ImGui::PushStyleColor( ImGuiCol::Text, colorRed );
					ImGui::Text( formatInt( collateralScore ) );
					ImGui::PopStyleColor();
					ImGui::SameLine();
					ImGui::Text( " COLLATERAL (PENALTY -" + collateralScore + ")" );
				} else {
					ImGui::PushStyleColor( ImGuiCol::Text, colorGreen );
					ImGui::Text( "NO CLEANUP (+1000)" );
					ImGui::PopStyleColor();
				}

				ImGui::TableNextColumn();

				ImGui::Text( "- " );
				ImGui::SameLine();
				if ( numDeaths > 0 ) {
					ImGui::PushStyleColor( ImGuiCol::Text, colorRed );
					ImGui::Text( formatInt( numDeaths ) );
					ImGui::PopStyleColor();
					ImGui::SameLine();
					ImGui::Text( "DEATHS (PENALTY -" + ( numDeaths * 250 ) + ")" );
				} else {
					ImGui::PushStyleColor( ImGuiCol::Text, colorGreen );
					ImGui::Text( "NO RESTARTS (+5000)" );
					ImGui::PopStyleColor();
				}

				ImGui::TableNextColumn();

				ImGui::Text( "- " );
				ImGui::SameLine();
				ImGui::Text( "DIFFICULTY BONUS " );
				if ( TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" ) > TheNomad::GameSystem::GameDifficulty::Normal ) {
					ImGui::PushStyleColor( ImGuiCol::Text, colorMagenta );
					ImGui::Text( SP_DIFF_STRINGS[ TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" ) ] );
					ImGui::PopStyleColor();
					ImGui::SameLine();
					ImGui::Text( " (BONUS +" + ( difficulty * 1000 ) +  ")" );
				} else {
					ImGui::Text( "NONE" );
				}

				ImGui::TableNextColumn();

				// TODO:
				ImGui::Text( "CHALLENGE: NONE" );

				/*
				if ( isClean ) {
					ImGui::Text( "- " );
					ImGui::SameLine();
					ImGui::PushStyleColor( ImGuiCol::Text, colorGreen );
					ImGui::Text( "CLEAN RUN" );
					ImGui::PopStyleColor();
				}
				*/

				ImGui::TableNextColumn();
				
				ImGui::Text( "TOTAL " + totalScore );
			}
			ImGui::EndTable();
			
			if ( ImGui::Button( "DONE" ) ) {
				TheNomad::Engine::SoundSystem::SoundEffect( "event:/sfx/menu/select_item" ).Play();
				GlobalState = GameState::EndOfLevel;
			}

			ImGui::End();
		}
		
		void Draw( bool endOfLevel, uint64 timer ) {
			ImGuiWindowFlags windowFlags = ImGui::MakeWindowFlags( ImGuiWindowFlags::NoTitleBar | ImGuiWindowFlags::NoMove
				| ImGuiWindowFlags::NoResize );
			const float scale = TheNomad::GameSystem::UIScale;

			if ( GlobalState != GameState::StatsMenu ) {
				if ( !TheNomad::Engine::IsKeyDown( TheNomad::Engine::KeyNum::Tab )
					&& !TheNomad::Engine::IsKeyDown( TheNomad::Engine::KeyNum::GamePad_Back ) )
				{
					return;
				}
			}

			if ( !endOfLevel ) {
				m_TimeMilliseconds = TheNomad::GameSystem::GameTic - timer;
			} else {
				m_TimeMilliseconds = timer;
			}
			m_TimeSeconds = m_TimeMilliseconds / 1000;
			m_TimeMinutes = m_TimeMilliseconds / 60000;
			
			if ( endOfLevel ) {
				DrawEndOfLevelStats();
				return;
			}

			ImGui::Begin( "##LevelStatsShow", null, windowFlags );
			ImGui::SetWindowPos( vec2( 16 * scale, 16 * scale ) );
			ImGui::SetWindowSize( vec2( 320.0f * scale, 128.0f * scale ) );
			
			ImGui::BeginTable( "##LevelStatsNumbers", 2 );
			
			//
			// time
			//
			ImGui::TableNextColumn();
			ImGui::Text( "TIME:" );
			ImGui::TableNextColumn();

			ImGui::Text( " " + m_TimeMinutes + ":" + m_TimeSeconds + "." + m_TimeMilliseconds );
			ImGui::SameLine();
			ImGui::TextColored( sgame_RankStringColors[ time_Rank ], sgame_RankStrings[ time_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// kills
			//
			ImGui::TableNextColumn();
			ImGui::Text( "KILLS:" );
			ImGui::TableNextColumn();
			ImGui::Text( " " + numKills );
			ImGui::SameLine();
			ImGui::TextColored( sgame_RankStringColors[ kills_Rank ], sgame_RankStrings[ kills_Rank ] );
			
			ImGui::TableNextRow();
			
			//
			// deaths
			//
			ImGui::TableNextColumn();
			ImGui::Text( "DEATHS:" );
			ImGui::TableNextColumn();
			ImGui::Text( " " + numDeaths );
			ImGui::SameLine();
			ImGui::TextColored( sgame_RankStringColors[ deaths_Rank ], sgame_RankStrings[ deaths_Rank ] );
			
			ImGui::EndTable();
			
			ImGui::End();
		}

		//
		// LevelStats::ApplyCollateral: collateral damage (killing innocents, unnecessary destruction)
		// will cost the ranking to go down a lot
		//
		private void ApplyCollateral() {
			LevelInfoData@ data = @LevelManager.GetCurrentData();

			if ( total_Rank == LevelRank::RankS && ( collateralScore >= data.m_RankS.maxCollateral || data.m_RankS.requiresClean ) ) {
				total_Rank = LevelRank::RankA;
			}
			if ( total_Rank == LevelRank::RankA && ( collateralScore >= data.m_RankA.maxCollateral || data.m_RankA.requiresClean ) ) {
				total_Rank = LevelRank::RankB;
			}
			if ( total_Rank == LevelRank::RankB && ( collateralScore >= data.m_RankB.maxCollateral || data.m_RankB.requiresClean ) ) {
				total_Rank = LevelRank::RankC;
			}
			if ( total_Rank == LevelRank::RankC && ( collateralScore >= data.m_RankC.maxCollateral || data.m_RankC.requiresClean ) ) {
				total_Rank = LevelRank::RankD;
			}
			if ( total_Rank == LevelRank::RankD && ( collateralScore >= data.m_RankD.maxCollateral || data.m_RankD.requiresClean ) ) {
				total_Rank = LevelRank::RankF;
			}
			if ( total_Rank == LevelRank::RankF && ( collateralScore >= data.m_RankF.maxCollateral || data.m_RankF.requiresClean ) ) {
				total_Rank = LevelRank::RankWereUBotting;
			}
		}
		private LevelRank GetHighestRank() const {
			LevelRank rank = kills_Rank;

			if ( rank < deaths_Rank ) {
				rank = deaths_Rank;
			}
			if ( rank < time_Rank ) {
				rank = time_Rank;
			}
			return rank;
		}
		private LevelRank GetLowestRank() const {
			LevelRank rank = kills_Rank;

			if ( rank > style_Rank ) {
				rank = style_Rank;
			}
			if ( rank > deaths_Rank ) {
				rank = deaths_Rank;
			}
			if ( rank > time_Rank ) {
				return rank;
			}
			return rank;
		}

		int difficulty = 0;

		uint m_TimeMilliseconds = 0;
		int m_TimeSeconds = 0;
		int m_TimeMinutes = 0;

		uint stylePoints = 0;
		uint numKills = 0;
		uint numDeaths = 0;
		uint collateralScore = 0;
		bool isClean = true;
		int totalScore = 0;
		uint numSecrets = 0;

		LevelRank total_Rank = LevelRank::RankS;
		LevelRank time_Rank = LevelRank::RankS;
		LevelRank style_Rank = LevelRank::RankS;
		LevelRank kills_Rank = LevelRank::RankS;
		LevelRank deaths_Rank = LevelRank::RankS;
	};
};