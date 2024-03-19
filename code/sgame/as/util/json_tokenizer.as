namespace TheNomad::Util {
	enum JsonTokenType {
		BEGIN_OBJECT,
		END_OBJECT,
		BEGIN_ARRAY,
		END_ARRAY,
		NULL,
		NUMBER,
		STRING,
		BOOLEAN,
		SEP_COLON,
		SEP_COMMA,
		END_DOCUMENT,
		ERROR
	};
	
	class JsonToken {
		JsonToken( JsonTokenType tokenType = NULL, const string& in value = "" ) {
			m_TokenType = tokenType;
			m_Value = value;
		}
		
		JsonTokenType getType() const {
			return m_TokenType;
		}
		
		const string& getValue() const {
			return m_Value;
		}
		string& getValue() {
			return m_Value;
		}
		
		private JsonTokenType m_TokenType;
		private string m_Value;
	};
	
	class JsonTokenizer {
		JsonTokenizer( bool bShouldConvertEscape = true ) {
			shouldConvertEscape = bShouldConvertEscape;
		}
		
		array<JsonToken@>@ Parse( const string& in buffer ) {
			dataStr = buffer;
			Tokenize();
			return @tokens;
		}
		
		array<JsonToken@>@ Parse( const string& in str, bool bShouldConvertEscape ) {
			shouldConvertEscape = bShouldConvertEscape;
			return Parse( str );
		}

		private void Tokenize() {
			JsonToken@ currentToken;
			
			tokens.resize( 0 );
			pos = 0;
			line = 0;
			do {
				@currentToken = Process();
				tokens.insertLast( @currentToken );
			} while ( currentToken.getType() != JsonTokenType::END_DOCUMENT && currentToken.getType() != JsonTokenType::ERROR );
			
			if ( currentToken.getType() == JsonTokenType::ERROR ) {
				GameError( "an error occurred while trying to tokenize JSON string " + currentToken.getValue() + "\n" );
			}
		}
		
		private char getNextChar() {
			buffer = dataStr[pos];
			
			if ( pos < dataStr.size() ) {
				pos++;
			}
			if ( buffer == '\n' ) {
				line++;
				col = 1;
			} else {
				col++;
			}
			return buffer;
		}
		
		private char peekChar( int offset = 0 ) {
			uint relativePos = pos + offset;
			
			if ( relativePos > 0 && relativePos < dataStr.size() ) {
				return dataStr[relativePos];
			} else {
				return buffer;
			}
		}
		
		private JsonToken Process() {
			while ( true ) {
				if ( pos >= dataStr.size() ) {
					return JsonToken( JsonTokenType::END_DOCUMENT, "" );
				}
				getNextChar();
				if ( !IsSpace( buffer ) ) {
					break;
				}
			}
			
			switch ( buffer ) {
			case '{': return JsonToken( JsonTokenType::BEGIN_OBJECT, buffer );
			case '}':
			case '[':
			case ']':
			case ',':
			case ':':
			case 'n':
			case 't':
			case 'f':
			case '\"':
			case '-':
			default:

			};

			if ( buffer == char( "{" ) ) {
				return JsonToken( JsonTokenType::BEGIN_OBJECT, buffer );
			} else if ( buffer == '}' ) {
				return JsonToken( JsonTokenType::END_OBJECT, buffer );
			} else if ( buffer == '[' ) {
				return JsonToken( JsonTokenType::BEGIN_ARRAY, buffer );
			} else if ( buffer == ']' ) {
				return JsonToken( JsonTokenType::END_ARRAY, buffer );
			} else if ( buffer == ',' ) {
				return JsonToken( JsonTokenType::SEP_COMMA, buffer );
			} else if ( buffer == char( ":" ) ) {
				return JsonToken( JsonTokenType::SEP_COLON, buffer );
			} else if ( buffer == char( "n" ) ) {
				return readNull();
			} else if ( buffer == char( "t" ) ) {
				return readBoolean( true );
			} else if ( buffer == char( "f" ) ) {
				return readBoolean( false );
			} else if ( buffer == char( "\"" ) ) {
				return readString( true );
			} else if ( buffer == char( "-" ) || IsDigit( buffer ) ) {
				return readNumber();
			} else {
				// 判断非标准 key-value
				if ( IsAlpha( buffer ) ) {
					return readString( false );
				}
				return JsonToken( JsonTokenType::NULL, "" );
			}
		}
		
		private JsonToken readNull() {
			if ( !( getNextChar() == char( "u" ) && getNextChar() == char( "l" ) && getNextChar() == char( "l" ) ) ) {
				pos -= 3;
				return readString( false );
			} else {
				return JsonToken( JsonTokenType::NULL, "" );
			}
		}
		
		private JsonToken readBoolean( bool expected ) {
			string tmp;
			uint i;
			
			if ( expected ) {
				tmp = "t";
				for ( i = 0; i < 3; i++ ) {
				  tmp += getNextChar();
				}
			} else {
				tmp = "f";
				for ( i = 0; i < 4; i++ ) {
					tmp += getNextChar();
				}
			}
			if ( expected ) {
				if ( tmp == "true" ) {
					return JsonToken( JsonTokenType::BOOLEAN, "true" );
				} else {
					pos -= 3;
					return readString( false );
				}
			} else {
				if ( tmp == "false" ) {
					return JsonToken( JsonTokenType::BOOLEAN, "false" );
				} else {
					pos -= 4;
					return readString( false );
				}
			}
		}
		
		private JsonToken readString( bool isStandard ) {
			array<string> endingChar;
			string strStorage;
			uint i;
			
			int initLine = line;
			int initCol = col;
			
			if ( isStandard ) {
				strStorage = "";
			} else {
				strStorage = peekChar( -1 );
			}
			
			while ( true ) {
				string next = getNextChar();
				
				if ( !isStandard && IsSpace( buffer ) ) {
					continue;
				}
				if ( pos < dataStr.size() ) {
					bool shouldEnd = false;
					bool isEndTypeStandard = isStandard;
					for ( i = 0; i < endingCharStandard.length(); i++ ) {
						if ( endingCharStandard[i] == buffer ) {
							shouldEnd = true;
							isEndTypeStandard = true;
						}
					}
					for ( i = 0; i < endingCharNonStandard.length(); i++ ) {
						if ( endingCharNonStandard[i] == buffer ) {
							shouldEnd = true;
							isEndTypeStandard = false;
						}
					}
					if ( !shouldEnd ) {
						if ( buffer == "\\" && shouldConvertEscape ) { // 处理转义字符
							strStorage += convertEscape( isStandard );
						} else {
							strStorage += buffer;
						}
					} else {
				  		if ( isEndTypeStandard != isStandard ) {
							return JsonToken( JsonTokenType::ERROR, "(" + initLine + ", " + initCol + "): unexpected end of string");
						}
						if ( !isStandard ) {
							pos--;
						}
						break;
					}
				} else {
					return JsonToken( JsonTokenType::ERROR, "(" + initLine + ", " + initCol + "): no end of string" );
				}
			}
			return JsonToken( JsonTokenType::STRING, strStorage );
		}

    	private string convertEscape( bool isStandard ) {
			string storageStr = "";
			uint i;
			int8 next = getNextChar();
	
			// BUG: Following escape characters are currently not available in AngelScript: \f \b
			if ( next == char( "\"" ) ) {
				return "\"";
			} else if ( next == char( "\'" ) ) {
				return "\'";
			} else if ( next == char( "\\" ) ) {
				return "\\";
			} else if ( next == char( "n" ) ) {
				return "\n";
			} else if ( next == char( "r" ) ) {
				return "\r";
			} else if ( next == char( "t" ) ) {
				return "\t";
			} else if ( next == char( "/" ) ) {
				return "/";
			}
			
			if ( next == "u" ) {
				storageStr = "\\u";
				while ( true ) {
					getNextChar();
					if ( pos < dataStr.size() -  1 ) {
						bool shouldEnd = false;
						for ( i = 0; i < endingCharStandard.length(); i++) {
							if ( endingCharStandard[i] == buffer ) {
								shouldEnd = true;
							}
						}
						for ( i = 0; i < endingCharNonStandard.length(); i++ ) {
							if ( endingCharNonStandard[i] == buffer ) {
								shouldEnd = true;
							}
						}
						if ( shouldEnd ) {
							pos--;
							return storageStr;
						}
						storageStr += buffer;
						if ( storageStr.size() >= 6 ) {
							// BUG: storageStr here is already an escape character,
							// but cannot be converted to a unicode character. This
							// is due to the limitation of AngelScript.
							return storageStr;
						}
					} else {
						storageStr += buffer;
						return storageStr;
					}
				}
			}
			pos--;
			return "\\";
		}
		
		private JsonToken readNumber() {
			string num = buffer;
			
			num.reserve( 64 );
			while ( true ) {
				char next = getNextChar();
				if ( pos < dataStr.size() ) {
					if ( IsDigit( next ) || next == char( "." ) ) {
						num += next;
					} else {
						pos--;
						break;
					}
				} else {
					break;
				}
			}
			return JsonToken( JsonTokenType::NUMBER, num );
		}
		
		private uint pos = 0;
		private uint line = 1;
		private uint col = 1;
		private string dataStr = "";
		private char buffer = 0;
		private bool shouldConvertEscape = true;
		private array<string> endingCharStandard = {
			"\""
		};
		private array<string> endingCharNonStandard = {
			":"
		};
		private array<JsonToken@> tokens;
	};
};