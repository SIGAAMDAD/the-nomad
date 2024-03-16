namespace TheNomad {
	namespace Engine::FileSystem {
		//!
		//!
		//!
		shared class IFileStream {
			IFileStream( const string& in fileName ) {
				Open( fileName );
			}
			IFileStream() {
			}
			~IFileStream() {
				Close();
			}
			
			bool Open( const string& in fileName ) {
				GameError( "IFileStream::Open: called" );
				return false;
			}
			bool IsOpen() const {
				return m_hFile != FS_INVALID_HANDLE;
			}
			void Close() {
				if ( !IsOpen() ) {
					return;
				}
				CloseFile( m_hFile );
				m_hFile = FS_INVALID_HANDLE;
			}
			int GetMode() const {
				return m_OpenMode;
			}
			
			protected int m_hFile;
			protected int m_OpenMode;
		};
		
		//!
		//! @shared class FileStream
		//! @brief 
		//!
		shared class FileStream : IFileStream {
			FileStream( const string& in fileName, int mode ) {
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
				Write( value, size, m_hFile );
			}
			
			bool IsOpen() const {
				return m_hFile != FS_INVALID_HANDLE;
			}
			
			bool Open( const string& in fileName, int mode ) {
				if ( m_hFile != FS_INVALID_HANDLE ) {
					Close();
				}
				
				m_OpenMode = mode;
				m_hFile = FS_INVALID_HANDLE;
				
				if ( mode == FS_OPEN_READ ) {
					m_hFile = OpenFileRead( fileName );
				} else if ( mods == FS_OPEN_WRITE ) {
					m_hFile = OpenFileWrite( fileName );
				} else if ( mode == FS_OPEN_APPEND ) {
					m_hFile = OpenFileAppend( fileName );
				} else {
					GameError( "FileStream::Open: invalid mode " + formatInt( mode ) );
				}
				
				return true;
			}
		};
		
		shared class InputStream : IFileStream {
			InputStream( const string& in fileName ) {
				Open( fileName );
				m_OpenMode = FS_OPEN_READ;
			}
			InputStream() {
				m_hFile = FS_INVALID_HANDLE;
				m_OpenMode = FS_OPEN_READ;
			}
			~InputStream() {
				Close();
			}

			void Close() {
				CloseFile( m_hFile );
				m_hFile = FS_INVALID_HANDLE;
			}

			bool Open( const string& in fileName ) {
				m_hFile = OpenFileRead( fileName );
				if ( m_hFile == FS_INVALID_HANDLE ) {
					ConsoleWarning( "OutputStream::Open: failed to create read-only file '" + fileName + "'\n" );
					return false;
				}

				return true;
			}

			void ReadByte( uint8& out v ) {
				Write( @v, 1, m_hFile );
			}
			void ReadUShort( uint16& out v ) {
				Write( @v, 2, m_hFile );
			}
			void ReadUInt( uint32& out v ) {
				Write( @v, 4, m_hFile );
			}
			void ReadULong( uint64& out v ) {
				Write( @v, 8, m_hFile );
			}
			void ReadChar( int8& out v ) {
				Write( @v, 1, m_hFile );
			}
			void ReadShort( int16& out v ) {
				Write( @v, 2, m_hFile );
			}
			void ReadInt( int32& out v ) {
				Write( @v, 4, m_hFile );
			}
			void ReadLong( int64& out v ) {
				Write( @v, 8, m_hFile );
			}
		};
		
		shared class OutputStream : IFileStream {
			OutputStream( const string& in fileName ) {
				Open( fileName );
			}
			OutputStream() {
				m_hFile = FS_INVALID_HANDLE;
				m_OpenMode = FS_OPEN_WRITE;
			}
			~OutputStream() {
				Close();
			}

			void Close() {
				CloseFile( m_hFile );
				m_hFile = FS_INVALID_HANDLE;
			}

			bool Open( const string& in fileName ) {
				m_hFile = OpenFileWrite( fileName );
				if ( m_hFile == FS_INVALID_HANDLE ) {
					GameError( "OutputStream::Open: failed to create write-only file '" + fileName + "'" );
				}

				return true;
			}

			void WriteByte( uint8 v ) {
				Write( @v, 1, m_hFile );
			}
			void WriteUShort( uint16 v ) {
				Write( @v, 2, m_hFile );
			}
			void WriteUInt( uint32 v ) {
				Write( @v, 4, m_hFile );
			}
			void WriteULong( uint64 v ) {
				Write( @v, 8, m_hFile );
			}
			void WriteChar( int8 v ) {
				Write( @v, 1, m_hFile );
			}
			void WriteShort( int16 v ) {
				Write( @v, 2, m_hFile );
			}
			void WriteInt( int32 v ) {
				Write( @v, 4, m_hFile );
			}
			void WriteLong( int64 v ) {
				Write( @v, 8, m_hFile );
			}
		};

		shared class FileManager {
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