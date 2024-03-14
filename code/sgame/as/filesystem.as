namespace TheNomad {
	namespace Engine::FileSystem {
		//!
		//!
		//!
		shared class IFileStream {
			~IFileStream() {
				Close();
			}
			
			bool Open( const string& fileName ) {
				GameError( "IFileStream::Open: called" );
			}
			bool IsOpen() const {
				return m_hFile != TheNomad::Engine::Constants.InvalidHandle
			}
			void Close() {
				if ( !IsOpen() ) {
					return;
				}
				CloseFile( m_hFile );
				m_hFile = THeNomad::Engine::Constants.InvalidHandle;
			}
			int GetMode() const {
				return m_OpenMode;
			}
			
			protected int m_hFile;
			protected int m_OpenMode;
		};
		
		//!
		//! @class FileStream
		//! @brief 
		//!
		shared class FileStream : IFileStream {
			FileStream( const string& fileName, int mode ) {
				m_OpenMode = mode;
			}
			
			void WriteChar( const int8 value ) {
				Write( @value, 1 );
			}
			void WriteShort( const int16 value ) {
				Write( @value, 2 );
			}
			void WriteInt( const int32 value ) {
				Write( @value, 4 );
			}
			void WriteLong( const int64 value ) {
				Write( @value, 8 );
			}
			int8 ReadChar();
			int16 ReadShort();
			int32 ReadInt();
			int64 ReadLong();
			
			void Write( const ref@ value, uint64 size ) {
				TheNomad::Engine::FileSystem::Write( value, size, m_hFile );
			}
			
			bool IsOpen() const {
				return m_hFile !is TheNomad::Constants::FS_INVALID_HANDLE;
			}
			
			bool Open( const string& fileName, int mode ) {
				if ( m_hFile !is TheNomad::Constants::FS_INVALID_HANDLE ) {
					Close();
				}
				
				m_OpenMode = mode;
				m_hFile = TheNomad::Constants::FS_INVALID_HANDLE;
				
				switch ( mode ) {
				case TheNomad::Constants::FS_OPEN_READ:
					
					break;
				case TheNomad::Constants::FS_OPEN_WRITE:
					
					break;
				case TheNomad::Constants::FS_OPEN_APPEND:
					
					break;
				};
				
				return true;
			}
			
			private int m_OpenMode;
			private int m_hFile;
		};
		
		class FileManager {
			FileManager() {
				m_GameDir = TheNomad::Engine::CvarVariableString( "fs_gamedir" );
				m_BasePath = TheNomad::Engine::CvarVariableString( "fs_basepath" );
				m_BaseGame = TheNomad::Engine::CvarVariableString( "fs_basegame" );
			}
			
			const string& GetGameDir() const {
				return m_GameDir;
			}
			const string& GetBasePath() const {
				return m_BasePath;
			}
			const string& GetBaseGame() const {
				return m_BaseGame;
			}
			
			// various cvars
			private string m_GameDir;
			private string m_BasePath;
			private string m_BaseGame;
			
			private uint m_nOpenFiles;
		};
	};
};