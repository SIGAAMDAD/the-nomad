namespace TheNomad::Engine::Renderer {
	class Shader {
		Shader() {
		}
		Shader( int hShader ) {
			m_hShader = hShader;
		}
		Shader( const string& in fileName ) {
			Set( fileName );
		}

		void Set( int hShader ) {
			m_hShader = hShader;
		}
		void Set( const string& in fileName ) {
			m_hShader = RegisterShader( fileName );
			if ( m_hShader == FS_INVALID_HANDLE ) {
				ConsoleWarning( "failed to load shader handle '" + fileName + "'\n" );
			}
		}

		int opConv() const {
			return m_hShader;
		}
		int opImplConv() const {
			return m_hShader;
		}

		private int m_hShader = FS_INVALID_HANDLE;
	};
};