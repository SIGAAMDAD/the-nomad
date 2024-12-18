#include "Engine/UserInterface/ConfigVarBase.as"

namespace TheNomad::Engine::UserInterface {
	//
	// ConfigVar: the bridge between ImGui and the modder
	// only primitives will work here, strings are the exception
	//
	class ConfigVar : ConfigVarBase {
		ConfigVar() {
		}
		ConfigVar( const string& in name, const string& in id ) {
			m_Id = id;
			m_Name = name;
		}

		void Draw() {
		}
		void Save() {
		}

		protected string m_Id;
		protected string m_Name;
		protected bool m_bModified = false;
	};
};