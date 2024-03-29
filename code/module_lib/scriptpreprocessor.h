/*
  ==============================================================================
   
   Preprocessor 0.5
   Copyright (c) 2005 Anthony Casteel

   This software is provided 'as-is', without any express or implied 
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any 
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you 
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and 
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source 
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/
   under addons & utilities or at
   http://www.omnisu.com

   Anthony Casteel
   jm@omnisu.com
  
  ==============================================================================

   This file is part of AngelJuice

   AngelJuice can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   AngelJuice is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with eJUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ==============================================================================
*/

#ifndef __ANGELJUICE_PREPROCESSOR_H_INCLUDED__
#define __ANGELJUICE_PREPROCESSOR_H_INCLUDED__

#include "module_public.h"

class Preprocessor
{
public:
	class VectorOutStream
	{
	private:
		eastl::vector<char> m_data;
	protected:
		virtual void Write(const char* d, unsigned int size)
		{
			m_data.insert(m_data.end(),d,d+size);
		}
        virtual void ReportErrors(const char* msgType,
                                  const char* msgText,
                                  const char* fileName,
                                  unsigned int line)
        {}

	public:
		const char* data() { return &m_data[0]; }
		size_t size() { return m_data.size(); }
	};

	/*
	class NullOutStream: public OutStream
	{
	protected:
		virtual void Write(const char*, unsigned int) {}
        virtual void ReportErrors(const char* msgType,
                                  const char* msgText,
                                  const char* fileName,
                                  unsigned int line) {}
	};
	*/


	//  ==============================================================================
	class LineNumberTranslator
	{
	public:
		class Table;
	private:
		Table* pimple;
	public:
		LineNumberTranslator();
		~LineNumberTranslator();
		eastl::string ResolveOriginalFile(unsigned int linenumber);
		unsigned int ResolveOriginalLine(unsigned int linenumber);
		void SetTable(Table*);
	};

	
	//  ==============================================================================
	class PragmaInstance
	{
	public:
		eastl::string name;
		eastl::string text;
		eastl::string current_file;
		unsigned int current_file_line;
		eastl::string root_file;
		unsigned int global_line;
	};

	class PragmaModel
	{
	public:
		virtual ~PragmaModel() {}
	
		virtual void handlePragma(const PragmaInstance&) = 0;
	};


	//  ==============================================================================
	class FileSource
	{
	public:
		virtual ~FileSource() {}

		virtual bool LoadFile(const eastl::string& filename, eastl::vector<char>& buffer ) {
            fileHandle_t f;

            f = FS_FOpenRead( filename.c_str() );
            if ( f == FS_INVALID_HANDLE ) {
                return false;
            }
            buffer.resize( FS_FileLength( f ) );
            if ( !FS_Read( buffer.data(), buffer.size(), f ) ) {
                FS_FClose( f );
                return false;
            }
            FS_FClose( f );

            return true;
        }
	};
	

public:
    //  ==============================================================================
	enum LexemType
	{
		IDENTIFIER,		//Names which can be expanded.
		COMMA,			//,
		SEMICOLON,
		OPEN,			//{[(
		CLOSE,			//}])
		PREPROCESSOR,	//Begins with #
		NEWLINE,
		WHITESPACE,
		IGNORE,
		COMMENT,
		STRING,
		NUMBER
	}; //End enum LexemType

	class Lexem
	{
	public:
		eastl::string value;
		LexemType type;
	}; //End class Lexem

	
	//  ==============================================================================
	typedef eastl::list<Lexem> LexemList;
	typedef LexemList::iterator LLITR;
	

	//  ==============================================================================
	typedef eastl::map<eastl::string, int32_t> ArgSet;
	
	class DefineEntry
	{
	public:
		LexemList lexems;
		ArgSet arguments;
	}; //end class DefineEntry
	
	typedef eastl::map<eastl::string, DefineEntry> DefineTable;

	//  ==============================================================================
    typedef eastl::map<eastl::string, PragmaModel*> PragmaMap;
	typedef PragmaMap::iterator PragmaIterator;

public:

    //  ==============================================================================
    Preprocessor();
    ~Preprocessor();

    //  ==============================================================================
	int preprocess(
		const eastl::string& filename, 
		const eastl::string& filedata, 
		FileSource& file_source,
		UtlVector<char>& destination, 
		LineNumberTranslator* lnt = nullptr);

	void define(const eastl::string&);
	void registerPragma(const eastl::string&, PragmaModel*);

	
protected:
    void PrintErrorMessage(const eastl::string& errmsg);
    void PrintWarningMessage(const eastl::string& errmsg);
    
    void callPragma(const eastl::string& name, const PragmaInstance& parms);

    LLITR findLexem(LLITR ITR, LLITR END, LexemType type);
    LLITR parseStatement(LLITR ITR, LLITR END, LexemList& dest);
    LLITR parseDefineArguments(LLITR ITR, LLITR END, LexemList& lexems, eastl::vector<LexemList>& args);
    LLITR expandDefine(LLITR ITR, LLITR END, LexemList& lexems, DefineTable& define_table);
    void parseDefine(DefineTable& define_table, LexemList& def_lexems);
    LLITR parseIfDef(LLITR ITR, LLITR END);
    void parseIf(LexemList& directive, eastl::string& name_out);
    void parsePragma(LexemList& args);

    void recursivePreprocess(	
	    eastl::string filename,
	    eastl::string filedata,
	    FileSource& file_source,
	    LexemList& lexems,
	    DefineTable& define_table,
	    const bool fromString);
    
	DefineTable application_specified;

	PragmaMap registered_pragmas;

	LineNumberTranslator::Table* LNT;
	eastl::string root_file;
	eastl::string current_file;
	unsigned int current_line;
	unsigned int lines_this_file;

	unsigned int number_of_errors;
};


#endif