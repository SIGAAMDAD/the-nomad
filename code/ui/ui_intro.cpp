#include "ui_lib.h"
#include <functional>

// ui_intro.cpp -- an intro a lot like ultrakill's bootup sequence & titanfall 2's regen screen

#define STATUS_BOOTING 0
#define STATUS_CHECKING_UPDATES 1
#define STATUS_NEEDS_UPDATES 2
#define STATUS_UPDATING 3
#define STATUS_DONE 4

#define BOOT_TIME 2
#define CHECK_TIME 2
#define NEED_TIME 1

#define CALC_TICS(seconds) (TICRATE*(seconds))

typedef struct
{
	CUIMenu menu;
	CUIFont msdosFont;
	
	const stringHash_t *osString;
	const stringHash_t *statusBooting;
	const stringHash_t *statusString0;
	const stringHash_t *statusString1;
	const stringHash_t *statusString2;
	
//	const stringHash_t *audioPrompt;
//	const stringHash_t *inputPrompt;
//	const stringHash_t *videoPrompt;
//	const stringHash_t *mechanicsPrompt;
	
//	const stringHash_t *needString;
	const stringHash_t *checkingString;
//	const stringHash_t *waitingString;
	const stringHash_t *runningString;
	const stringHash_t *doneString;
	
	const stringHash_t *bootString;
	const stringHash_t *finishingString;
	const stringHash_t *completedString;
	
	int ticker;
	int updateStatus;

//	textureFilter_t texture_filter;
//	char detailString[64];
	
//	qboolean doneAudio;
//	qboolean doneVideo;
//	qboolean doneInput;
//	qboolean doneMechanics;
//
	qboolean checking;
	qboolean running;
//	
//	// cvar values
//	float sfxVol;
//	float musicVol;
//	bool sfxOn;
//	bool musicOn;
} intromenu_t;

static const char *detailStrings[] = {
	"MS-DOS VGA",
    "INTEGRATED INTEL GPU",
    "NORMIE",
    "EXPENSIVE SHIT WE'VE GOT HERE",
    "GPU VS GOD"
};

static intromenu_t intro;

static void IntroMenu_Booting( void )
{
	// are we done?
	if (!intro.ticker) {
		ui->DrawString( intro.statusBooting->value );
		intro.ticker = CALC_TICS( CHECK_TIME );
		intro.updateStatus = STATUS_CHECKING_UPDATES;
		return;
	}
	
	// draw it as flickering text
	ui->DrawStringBlink( intro.statusBooting->value, intro.ticker, 2 );
	
	intro.ticker--;
}

static void IntroMenu_CheckingUpdates( void )
{
	// draw everything else
	ui->DrawString( intro.statusString0->value );
	
	// are we done?
	if (!intro.ticker) {
		intro.ticker = CALC_TICS( NEED_TIME );
		intro.updateStatus = STATUS_NEEDS_UPDATES;
		return;
	}
	
	intro.ticker--;
}

static void IntroMenu_NeedsUpdates( void )
{
	// draw everything else
	ui->DrawString( intro.statusString1->value );
	
	// are we done?
	if (!intro.ticker) {
		intro.updateStatus = STATUS_UPDATING;
		intro.ticker = CALC_TICS( 2 );
		intro.running = qtrue;
		intro.checking = qtrue;
		return;
	}
	
	intro.ticker--;
}

static void IntroMenu_Finish( void )
{
//	ui->DrawString(  );
}

static void IntroMenu_Draw( void )
{
	CUIWindow window("IntroMenu");

	window.SetFontScale(2.5f);

	ui->DrawString( intro.osString->value );

	switch (intro.updateStatus) {
	case STATUS_BOOTING:
		IntroMenu_Booting();
		break;
	case STATUS_CHECKING_UPDATES:
		IntroMenu_CheckingUpdates();
		break;
	case STATUS_NEEDS_UPDATES:
		IntroMenu_NeedsUpdates();
		break;
	case STATUS_UPDATING:
		if (intro.ticker) {
			ui->DrawString( intro.statusString2->value );
			intro.ticker--;
		}
		else {
			intro.updateStatus = STATUS_DONE;
		}
		break;
	case STATUS_DONE:
		ui->DrawString( intro.completedString->value );
		ImGui::TextUnformatted( "PRESS ANY KEY TO CONTINUE" );
		if (Key_AnyDown()) {
			ui->SetActiveMenu( UI_MENU_TITLE );
		}
		break;
	};
}

/*
static GDR_INLINE bool IntroMenu_DrawConfigMenu( const char *name, const std::function<void(const CUIWindow& window)>& drawFunc)
{
	ImGui::End();
	
	CUIWindow window(name);
	drawFunc( window );
	
	return ImGui::Button("Done");
}

static const char *TextureFilterString( void )
{
	switch (intro.texture_filter) {
	case TexFilter_Linear:
		return "LINEAR";
	case TexFilter_Nearest:
		return "NEAREST";
	case TexFilter_Bilinear:
		return "BILINEAR";
	case TexFilter_Trilinear:
		return "TRILINEAR";
	default:
		break;
	};
	N_Error(ERR_FATAL, "Invalid texture filtering value: %i", intro.texture_filter);
	return NULL;
}


static void IntroMenu_DrawVideo( void )
{
	bool done;
	int i;

	done = IntroMenu_DrawConfigMenu( "VideoConfig",
	[&]( const CUIWindow& window ) -> void {
		ImGui::BeginTable(" ", 2);
		{
			ImGui::TableNextColumn();
			ImGui::TextUnformatted("TEXTURE FILTERING");
			ImGui::TableNextColumn();
			if (ImGui::BeginMenu(TextureFilterString())) {
				if (ImGui::MenuItem("LINEAR")) {
					intro.texture_filter = TexFilter_Linear;
				}
				if (ImGui::MenuItem("NEAREST")) {
					intro.texture_filter = TexFilter_Nearest;
				}
				if (ImGui::MenuItem("BILINEAR")) {
					intro.texture_filter = TexFilter_Bilinear;
				}
				if (ImGui::MenuItem("TRILINEAR")) {
					intro.texture_filter = TexFilter_Trilinear;
				}
				ImGui::EndMenu();
			}

			ImGui::TableNextRow();
			
			ImGui::TableNextColumn();
			ImGui::TextUnformatted("QUALITY");
			ImGui::TableNextColumn();
			if (ImGui::BeginMenu(intro.detailString)) {
				for (i = 0; i < arraylen(detailStrings); i++) {
					if (ImGui::MenuItem(detailStrings[i])) {
						N_strncpyz(intro.detailString, detailStrings[i], sizeof(intro.detailString));
					}
				}
				ImGui::EndMenu();
			}
		}
		ImGui::EndTable();
	});

	if (done) {
		intro.doneVideo = qtrue;
		
		Cvar_Set("r_textureFiltering", va("%i", (int)intro.texture_filter));

		for (i = 0; i < arraylen(detailStrings); i++) {
			if (!N_stricmp(detailStrings[i], intro.detailString)) {
				break;
			}
		}

		Cvar_Set( "r_textureDetail", va("%i", i) );
		switch (i) {
		case TexDetail_GPUvsGod:
			Cvar_Set( "r_maxPolys", va("%i", 4096*256) );
			Cvar_Set( "r_maxPolyVerts", va("%i", 4096*1024) );
			break;
		case TexDetail_ExpensiveShitWeveGotHere:
			Cvar_Set( "r_maxPolys", va("%i", 4096*128) );
			Cvar_Set( "r_maxPolyVerts", va("%i", 4096*512) );
			break;
		case TexDetail_Normie:
			Cvar_Set( "r_maxPolys", va("%i", 8192) );
			Cvar_Set( "r_maxPolyVerts", va("%i", 4096*128) );
			break;
		case TexDetail_IntegratedGPU:
			Cvar_Set( "r_maxPoly", va("%i", 4096) );
			Cvar_Set( "r_maxPolyVerts", va("%i", 4096*4) );
			break;
		case TexDetail_MSDOS:
			Cvar_Set( "r_maxPolys", va("%i", 256) );
			Cvar_Set( "r_maxPolyVerts", va("%i", 256*4) );
			break;
		};
	}
}

static void IntroMenu_DrawAudio( void )
{
	bool done = false;

	if (!intro.doneAudio) {
		done = IntroMenu_DrawConfigMenu( "AudioConfig",
			[&]( const CUIWindow& window ) -> void {
				ImGui::SeparatorText("SOUND EFFECTS");
				ImGui::Checkbox("ON", &intro.sfxOn);
				if (!intro.sfxOn) {
					ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4( 0.1f, 0.1f, 0.1f, 1.0f ));
				}
				ImGui::SliderFloat("VOLUME", &intro.sfxVol, 0, 100);
				if (!intro.sfxOn) {
					ImGui::PopStyleColor();
				}

				ImGui::SeparatorText("MUSIC");
				ImGui::Checkbox("ON", &intro.musicOn);
				if (!intro.musicOn) {
					ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4( 0.1f, 0.1f, 0.1f, 1.0f ));
				}
				ImGui::SliderFloat("VOLUME", &intro.musicVol, 0, 100);
				if (!intro.musicOn) {
					ImGui::PopStyleColor();
				}
			}
		);

		intro.doneAudio = done;
	}
}

static void IntroMenu_Updating( void )
{
	// draw everything else
	ui->DrawString( intro.statusString2->value );
	
	ImGui::NewLine();
	ImGui::NewLine();

	if (!intro.doneAudio) {
		IntroMenu_DrawAudio();
	} else if (!intro.doneVideo) {
		IntroMenu_DrawVideo();
	} else if (!intro.doneInput) {
		IntroMenu_DrawInput();
	} else if (!intro.doneMechanics) {
		IntroMenu_DrawMechanics();
	} else {
		intro.updateStatus = STATUS_DONE;
	}
}

static void IntroMenu_Draw( void )
{
	// we can't use the window class because we'll need multiple windows
	ImGui::Begin("IntroMenu", NULL,
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse);
	ImGui::SetWindowSize(ImVec2( (float)ui->GetConfig().vidWidth, (float)ui->GetConfig().vidHeight ));
	ImGui::SetWindowPos(ImVec2( 0, 0 ));
	
//	intro.msdosFont.Bind();
	
	ui->DrawString( intro.osString->value );
	ImGui::NewLine();
	
	// draw all the extra stuff
	if (intro.updateStatus > STATUS_NEEDS_UPDATES) {
		ui->DrawString( intro.statusString2->value );
		ImGui::NewLine();
		ImGui::NewLine();
	}

	if (intro.updateStatus >= STATUS_UPDATING) {
		ImGui::BeginTable( " ", 2 );
		{
			ImGui::TableNextColumn();
			ui->DrawString( intro.audioPrompt->value );
			ImGui::TableNextColumn();

			if (intro.running) {
				if (!intro.doneAudio) {
					if (intro.ticker) {
						if (intro.checking) {
							ui->DrawString( intro.checkingString->value );
						}
						else {
							ui->DrawString( intro.needString->value );
						}
						intro.ticker--;
					}
					else {
						if (intro.checking) {
							intro.checking = qfalse;
							intro.ticker = CALC_TICS(2);
						}
						else {
						}
						ui->DrawString( intro.runningString->value );
						intro.ticker = CALC_TICS( 2 );
					}
				}
				else {
					ui->DrawString( intro.doneString->value );
				}
				if (!intro.doneVideo) {
					if (intro.ticker) {
						ui->DrawString(  );
					}
					else {

					}
				}
				else {

				}
				if (!intro.doneInput) {

				}
				else {

				}
				if (!intro.doneMechanics) {

				}
				else {
					intro.running = qfalse;
				}
			}
		}
		ImGui::EndTable();
	}
	
	switch (intro.updateStatus) {
	case STATUS_BOOTING:
		IntroMenu_Booting();
		break;
	case STATUS_CHECKING_UPDATES:
		IntroMenu_CheckingUpdates();
		break;
	case STATUS_NEEDS_UPDATES:
		IntroMenu_NeedsUpdates();
		break;
	case STATUS_UPDATING:
		IntroMenu_Updating();
		break;
	case STATUS_DONE:
		IntroMenu_Finish();
		break;
	default:
		N_Error(ERR_FATAL, "Invalid ui state"); // should never happen
	};
	
//	intro.msdosFont.Unbind();
}
*/

void IntroMenu_Cache( void )
{
	memset(&intro, 0, sizeof(intro));
	
	intro.osString = strManager->ValueForKey("INTRO_OS_STRING");
	intro.statusBooting = strManager->ValueForKey("INTRO_STATUS_BOOTING");
	intro.statusString0 = strManager->ValueForKey("INTRO_STATUS_STRING0");
	intro.statusString1 = strManager->ValueForKey("INTRO_STATUS_STRING1");
	intro.statusString2 = strManager->ValueForKey("INTRO_STATUS_STRING2");
	
//	intro.audioPrompt = strManager->ValueForKey("INTRO_AUDIO_STRING");
//	intro.inputPrompt = strManager->ValueForKey("INTRO_INPUT_STRING");
//	intro.videoPrompt = strManager->ValueForKey("INTRO_VIDEO_STRING");
//	intro.mechanicsPrompt = strManager->ValueForKey("INTRO_MECHANICS_STRING");
//	
//	intro.needString = strManager->ValueForKey("INTRO_NEED_STRING");
//	intro.checkingString = strManager->ValueForKey("INTRO_CHECKING_STRING");
//	intro.waitingString = strManager->ValueForKey("INTRO_WAITING_STRING");
//	intro.runningString = strManager->ValueForKey("INTRO_RUNNING_STRING");
//	intro.doneString = strManager->ValueForKey("INTRO_DONE_STRING");
	
	intro.bootString = strManager->ValueForKey("INTRO_BOOT_STRING");
	intro.finishingString = strManager->ValueForKey("INTRO_FINISHING");
	intro.completedString = strManager->ValueForKey("INTRO_COMPLETED");
	
	intro.ticker = CALC_TICS( BOOT_TIME );
	intro.updateStatus = STATUS_BOOTING;
}

void UI_IntroMenu( void )
{	
	IntroMenu_Cache();

	intro.menu.Draw = IntroMenu_Draw;

	ui->PushMenu( &intro.menu );
}
