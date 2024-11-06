namespace TheNomad::SGame {
	class TutorialPopup {
		TutorialPopup() {
		}

		void Display() {
			ImGui::Begin( "##TutorialPrompt" + m_Title, null, ImGui::MakeWindowFlags() );

			ImGui::Text( m_Text );

			ImGui::End();
		}

		private string m_Title;
		private string m_Text;
	};
};