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

#include "module_public.h"
#include "module_alloc.h"
#include "scriptpreprocessor.h"

const UtlString numbers = "0123456789";
const UtlString identifierStart = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const UtlString identifierBody = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const UtlString hexnumbers = "0123456789abcdefABCDEF";
const UtlString trivials = ",;\n\r\t [{(]})";

const Preprocessor::LexemType trivial_types[] =
{
	Preprocessor::LEX_COMMA, 
	Preprocessor::LEX_SEMICOLON, 
	Preprocessor::LEX_NEWLINE, 
	Preprocessor::LEX_WHITESPACE, 
	Preprocessor::LEX_WHITESPACE, 
	Preprocessor::LEX_WHITESPACE, 
	Preprocessor::LEX_OPEN, 
	Preprocessor::LEX_OPEN, 
	Preprocessor::LEX_OPEN,
	Preprocessor::LEX_CLOSE, 
	Preprocessor::LEX_CLOSE, 
	Preprocessor::LEX_CLOSE
};

static bool searchString(UtlString str, char in)
{
	return (str.find_first_of(in) < str.length());
}

static bool isNumber(char in) { return searchString(numbers,in); }
static bool isIdentifierStart(char in)	{ return searchString(identifierStart,in); }
static bool isIdentifierBody(char in) { return searchString(identifierBody,in); }
static bool isTrivial(char in) { return searchString(trivials,in); }
static bool isHex(char in) { return searchString(hexnumbers,in); }

static char* parseIdentifier(char* start, char* end, Preprocessor::Lexem& out)
{
	out.type = Preprocessor::LEX_IDENTIFIER;
	out.value += *start;
	while(true)
	{
		++start;
		if (start == end) return start;
		if (isIdentifierBody(*start))
		{
			out.value += *start;
		} 
		else
		{
			return start;
		}
	}
};

static char* parseStringLiteral(char* start, char* end, Preprocessor::Lexem& out)
{
	out.type = Preprocessor::LEX_STRING;
	out.value += *start;
	while(true)
	{
		++start;
		if (start == end) return start;
		out.value += *start;
		//End of string literal?
		if (*start == '\"') return ++start;
		//Escape sequence? - Really only need to handle \"
		if (*start == '\\')	
		{
			++start;
			if (start == end) return start;
			out.value += *start;
		}
	}
}

static char* parseCharacterLiteral(char* start, char* end, Preprocessor::Lexem& out)
{
	++start;
	if (start == end) return start;
	out.type = Preprocessor::LEX_NUMBER;
	if (*start == '\\')
	{
		++start;
		if (start == end) return start;
		if (*start == 'n') out.value = '\n';
		if (*start == 't') out.value = '\t';
		if (*start == 'r') out.value = '\r';
		++start; 
		if (start == end) return start;
		++start; return start;
	} 
	else 
	{
		out.value = va( "%i", *start );
		++start;
		if (start == end) return start;
		++start; return start;
	}
}

static char* parseFloatingPoint(char* start, char* end, Preprocessor::Lexem& out)
{
	out.value += *start;
	while (true)
	{
		++start;
		if (start == end) return start;
		if (!isNumber(*start))
		{
			if (*start == 'f')
			{
				out.value += *start;
				++start;
				return start;
			}
			return start;
		} else {
			out.value += *start;
		}
	}
}

static char* parseHexConstant(char* start, char* end, Preprocessor::Lexem& out)
{
	out.value += *start;
	while (true)
	{ 
		++start;
		if (start == end) return start;
		if (isHex(*start))
		{
			out.value += *start;
		} else {
			return start;
		}
	}
}

static char* parseNumber(char* start, char* end, Preprocessor::Lexem& out)
{
	out.type = Preprocessor::LEX_NUMBER;
	out.value += *start;
	while(true)
	{
		++start;
		if (start == end) return start;
		if (isNumber(*start)) {
			out.value += *start;
		} else if (*start == '.') {
			return parseFloatingPoint(start,end,out);
		} else if (*start == 'x') {
			return parseHexConstant(start,end,out);
		} else {
			return start;
		}
	}
}

static char* parseBlockComment(char* start, char* end, Preprocessor::Lexem& out)
{
	out.type = Preprocessor::LEX_COMMENT;
	out.value += "/*";

	int newlines = 0;
	while(true)
	{
		++start;
		if (start == end) break;
		if (*start == '\n')
		{
			newlines++;
		}
		out.value += *start;
		if (*start == '*')
		{
			++start;
			if (start == end) break;
			if (*start == '/') 
			{
				out.value += *start;
				++start;
				break;
			} else 
			{
				--start;
			}
		}
	}
	while (newlines > 0) 
	{
		--start;
		*start = '\n';
		--newlines;
	}
	return start;
}

static char* parseLineComment(char* start, char* end, Preprocessor::Lexem& out)
{
	out.type = Preprocessor::LEX_COMMENT;
	out.value += "//";
	
	while(true)
	{
		++start;
		if (start == end) return start;
		out.value += *start;
		if (*start == '\n') return start;
	}
}

static char* parseLexem(char* start, char* end, Preprocessor::Lexem& out) 
{	
	if (start == end) return start;
	char current_char = *start;
	
	if (isTrivial(current_char)) 
	{
		out.value += current_char;
		out.type = trivial_types[trivials.find_first_of(current_char)];
		return ++start;
	}
	
	if (isIdentifierStart(current_char)) return parseIdentifier(start,end,out);
	
	if (current_char == '#')
	{
		start = parseIdentifier(start,end,out);
		out.type = Preprocessor::LEX_PREPROCESSOR;
		return start;
	}
	
	if (isNumber(current_char)) return parseNumber(start,end,out);
	if (current_char == '\"') return parseStringLiteral(start,end,out);
	if (current_char == '\'') return parseCharacterLiteral(start,end,out);	
	if (current_char == '/')
	{
		//Need to see if it's a comment.
		++start;
		if (start == end) return start;
		if (*start == '*') return parseBlockComment(start,end,out);
		if (*start == '/') return parseLineComment(start,end,out);
		//Not a comment - let default code catch it as MISC
		--start;
	}
				
	out.value = current_char;
	out.type = Preprocessor::LEX_IGNORE;
	return ++start;
}

static int lex(char* begin, char* end, UtlList<Preprocessor::Lexem>& results)
{
	Assert(begin != 0 && end != 0);
	Assert(begin <= end);

	while (true)
	{
		Preprocessor::Lexem current_lexem;
		begin = parseLexem(begin,end,current_lexem);
		if (current_lexem.type != Preprocessor::LEX_WHITESPACE &&
			current_lexem.type != Preprocessor::LEX_COMMENT ) results.push_back(current_lexem);
		if (begin == end) return 0;
	}
};

static void printLexemList( Preprocessor::LexemList& out, UtlVector<char>& destination)
{
	bool need_a_space = false;
	for (Preprocessor::LLITR ITR = out.begin(); ITR != out.end(); ++ITR)
	{
		if (ITR->type == Preprocessor::LEX_IDENTIFIER || ITR->type == Preprocessor::LEX_NUMBER) 
		{
			if (need_a_space) destination.emplace_back( ' ' );
			need_a_space = true;
			destination.insert( destination.end(), ITR->value.begin(), ITR->value.end() );
		}
		else 
		{
			need_a_space = false;
			destination.insert( destination.end(), ITR->value.begin(), ITR->value.end() );
		}
	}
}

static UtlString removeQuotes(const UtlString& in)
{
	return eastl::move( in.substr(1,in.size()-2) );
}

static UtlString addPaths(const UtlString& first, const UtlString& second)
{
	UtlString result;
	size_t slash_pos = first.find_last_of('/');
	if (slash_pos == 0 || slash_pos >= first.size()) return second;
	result = eastl::move( first.substr(0,slash_pos+1) );
	result += second;
	return result;
}

static void setLineMacro(Preprocessor::DefineTable& define_table, unsigned int line)
{
	Preprocessor::DefineEntry def;
	Preprocessor::Lexem l;
	l.type = Preprocessor::LEX_NUMBER;

    l.value.sprintf( "%u", line );
    def.lexems.emplace_back( l );
    define_table["__LINE__"] = def;
}

static void setFileMacro(Preprocessor::DefineTable& define_table, const UtlString& file)
{
	Preprocessor::DefineEntry def;
	Preprocessor::Lexem l;
	l.type = Preprocessor::LEX_STRING;
	l.value = va( "\"%s\"", file.c_str() );
	def.lexems.emplace_back(l);
	define_table["__FILE__"] = def;
}	


//=================================================================================

Preprocessor::Preprocessor()
{
}

Preprocessor::~Preprocessor()
{
    registered_pragmas.clear();
}


void Preprocessor::PrintErrorMessage(const UtlString& errmsg)
{
    g_pModuleLib->GetScriptEngine()->WriteMessage( current_file.c_str(), (int)( lines_this_file + 1 ), 0,
        asMSGTYPE_ERROR, errmsg.c_str() );
    number_of_errors++;
}

void Preprocessor::PrintWarningMessage(const UtlString& errmsg)
{
    g_pModuleLib->GetScriptEngine()->WriteMessage( current_file.c_str(), (int)( lines_this_file + 1 ), 0,
        asMSGTYPE_WARNING, errmsg.c_str() );
}

void Preprocessor::registerPragma(const UtlString& name, Preprocessor::PragmaModel* pc)
{
	if (pc == 0) return;
	PragmaIterator I = registered_pragmas.find(name);
	if (I != registered_pragmas.end()) 
	{
		// I->second = 0;
		registered_pragmas.erase(I);
	}
	registered_pragmas[name] = pc;
}

void Preprocessor::callPragma(const UtlString& name, const Preprocessor::PragmaInstance& parms)
{
	Preprocessor::PragmaIterator I = registered_pragmas.find(name);
	if (I == registered_pragmas.end())
	{
		PrintErrorMessage("Unknown pragma command.");
		return;
	}
	if (I->second) I->second->handlePragma(parms);
}


//=================================================================================

class Preprocessor::LineNumberTranslator::Table
{
public:
	struct Entry
	{
		UtlString file;
		unsigned int start_line;
		unsigned int offset;
	};

	UtlVector<Entry> lines;

	//Assuming blocks were entered in the proper order.
	Entry& search(unsigned int linenumber)
	{
		for (size_t i = 1; i < lines.size(); ++i)
		{
			if (linenumber < lines[i].start_line)
			{
				//Found the first block after our line.
				return lines[i-1];
			}
		}
		return lines[lines.size()-1]; //Line must be in last block.
	}

	void AddLineRange(const UtlString& file, unsigned int start_line, unsigned int offset)
	{
		Entry e;
		e.file = file;
		e.start_line = start_line;
		e.offset = offset;
		lines.push_back(e);
	}
};


//=================================================================================

Preprocessor::LineNumberTranslator::LineNumberTranslator()
  : pimple(0)
{
}

Preprocessor::LineNumberTranslator::~LineNumberTranslator()
{
    if (pimple)
        delete pimple;
    pimple = 0;
}

UtlString Preprocessor::LineNumberTranslator::ResolveOriginalFile(unsigned int linenumber)
{
	if (! pimple) return "ERROR";
	return pimple->search(linenumber).file;
}

unsigned int Preprocessor::LineNumberTranslator::ResolveOriginalLine(unsigned int linenumber)
{
	if (! pimple) return 0;
	return linenumber - pimple->search(linenumber).offset;
}

void Preprocessor::LineNumberTranslator::SetTable(Preprocessor::LineNumberTranslator::Table *t)
{
	if (pimple)
		delete pimple;
	pimple = t;
}


//=================================================================================

Preprocessor::LLITR Preprocessor::findLexem(Preprocessor::LLITR ITR, Preprocessor::LLITR END, Preprocessor::LexemType type)
{
	while(ITR != END && ITR->type != type)
	{
		++ITR;
	}
	return ITR;
}

Preprocessor::LLITR Preprocessor::parseStatement(Preprocessor::LLITR ITR, Preprocessor::LLITR END, Preprocessor::LexemList& dest)
{
	int depth = 0;
	while (ITR != END)
	{
		if (ITR->value == "," && depth == 0) return ITR;
		if (ITR->type == Preprocessor::LEX_CLOSE && depth == 0) return ITR;
		if (ITR->type == Preprocessor::LEX_SEMICOLON && depth == 0) return ITR;
		dest.push_back(*ITR);
		if (ITR->type == Preprocessor::LEX_OPEN) depth++;
		if (ITR->type == Preprocessor::LEX_CLOSE) {
			if (depth == 0) PrintErrorMessage("Mismatched braces while parsing statement.");
			depth--;
		}
		++ITR;
	}
	return ITR;
}

Preprocessor::LLITR Preprocessor::parseDefineArguments(Preprocessor::LLITR ITR, Preprocessor::LLITR END, Preprocessor::LexemList& lexems,
	UtlVector<Preprocessor::LexemList>& args)
{
	if (ITR == END || ITR->value != "(") 
	{
		PrintErrorMessage("Expected argument list.");
		return ITR;
	}
	Preprocessor::LLITR begin_erase = ITR;
	++ITR;

	while (ITR != END)
	{
		Preprocessor::LexemList argument;
		ITR = parseStatement(ITR,END,argument);
		args.push_back(argument);

		if (ITR == END) {
			PrintErrorMessage("Unexpected end of file.");
			return ITR;
		}
		if (ITR->value == ",")
		{
			++ITR;
			if (ITR == END) {
				PrintErrorMessage("Unexpected end of file.");
				return ITR;
			}
			continue;
		}
		if (ITR->value == ")")
		{
			++ITR;
			break;
		}
	}

	return lexems.erase(begin_erase,ITR);
}

Preprocessor::LLITR Preprocessor::expandDefine(Preprocessor::LLITR ITR, Preprocessor::LLITR END, Preprocessor::LexemList& lexems, Preprocessor::DefineTable& define_table)
{
	Preprocessor::DefineTable::iterator define_entry = define_table.find(ITR->value);
	if (define_entry == define_table.end()) return ++ITR;
	ITR = lexems.erase(ITR);
	if (define_entry->second.arguments.size() == 0) 
	{
		lexems.insert(ITR,
			define_entry->second.lexems.begin(),
			define_entry->second.lexems.end());
		return ITR;
	}

	//define has arguments.
	UtlVector<Preprocessor::LexemList> arguments;
	ITR = parseDefineArguments(ITR,END,lexems,arguments);

	if (define_entry->second.arguments.size() != arguments.size()) 
	{
		PrintErrorMessage("Didn't supply right number of arguments to define.");
		return ITR;
	}

	Preprocessor::LexemList temp_list(define_entry->second.lexems.begin(),define_entry->second.lexems.end());

	Preprocessor::LLITR TLI = temp_list.begin();
	while (TLI != temp_list.end())
	{
		ArgSet::iterator arg = define_entry->second.arguments.find(TLI->value);
		if (arg == define_entry->second.arguments.end()) 
		{
			++TLI;
			continue;
		}

		TLI = temp_list.erase(TLI);
		temp_list.insert(TLI,arguments[arg->second].begin(),arguments[arg->second].end());
	}

	lexems.insert(ITR,temp_list.begin(),temp_list.end());

	return ITR;
}

void Preprocessor::parseDefine(Preprocessor::DefineTable& define_table, Preprocessor::LexemList& def_lexems)
{
	def_lexems.pop_front();	//remove #define directive
	if (def_lexems.empty()) {
		PrintErrorMessage("Define directive without arguments.");
		return;
	}
	Lexem name = *def_lexems.begin();
	if (name.type != Preprocessor::LEX_IDENTIFIER) 
	{
		PrintErrorMessage("Define's name was not an identifier.");
		return;
	}
	def_lexems.pop_front();

	Preprocessor::DefineEntry def;

	if (!def_lexems.empty())
	{
		if (def_lexems.begin()->type == Preprocessor::LEX_PREPROCESSOR && def_lexems.begin()->value == "#")
		{
			//Macro has arguments
			def_lexems.pop_front();
			if (def_lexems.empty()) 
			{
				PrintErrorMessage("Expected arguments.");
				return;
			}
			if (def_lexems.begin()->value != "(")
			{
				PrintErrorMessage("Expected arguments.");
				return;
			}
			def_lexems.pop_front();

			int num_args = 0;
			while(!def_lexems.empty() && def_lexems.begin()->value != ")")
			{
				if (def_lexems.begin()->type != Preprocessor::LEX_IDENTIFIER) 
				{
					PrintErrorMessage("Expected identifier.");
					return;
				}
				def.arguments[def_lexems.begin()->value] = num_args;
				def_lexems.pop_front();
				if (!def_lexems.empty() && def_lexems.begin()->value == ",")
				{
					def_lexems.pop_front();
				}
				num_args++;
			}

			if (!def_lexems.empty()) 
			{
				if (def_lexems.begin()->value != ")")
				{
					PrintErrorMessage("Expected closing parantheses.");
					return;
				}
				def_lexems.pop_front();
			}
			else 
			{
				PrintErrorMessage("Unexpected end of line.");
				return;
			}
		}

		Preprocessor::LLITR DLB = def_lexems.begin();
		while (DLB != def_lexems.end())
		{
			DLB = expandDefine(DLB,def_lexems.end(),def_lexems,define_table);
		}
	}
	
	def.lexems = def_lexems;
	define_table[name.value] = def;
}

Preprocessor::LLITR Preprocessor::parseIfDef(Preprocessor::LLITR ITR, Preprocessor::LLITR END)
{
	int depth = 0;
	int newlines = 0;
	bool found_end = false;
	while (ITR != END)
	{
		if (ITR->type == Preprocessor::LEX_NEWLINE) newlines++;
		else if (ITR->type == Preprocessor::LEX_PREPROCESSOR)
		{
			if (ITR->value == "#endif" && depth == 0) 
			{
				++ITR;
				found_end = true;
				break;
			}
			if (ITR->value == "#ifdef" || ITR->value == "#ifndef") depth++;
			if (ITR->value == "#endif" && depth > 0) depth--;
		}
		++ITR;
	}
	if (ITR == END && !found_end) 
	{
		PrintErrorMessage("Unexpected end of file.");
		return ITR;
	}
	while (newlines > 0)
	{
		--ITR;
		ITR->type = Preprocessor::LEX_NEWLINE;
		ITR->value = "\n";
		--newlines;
	}
	return ITR;
}

void Preprocessor::parseIf( Preprocessor::LexemList& directive, UtlString& name_out )
{
	directive.pop_front();
	if (directive.empty()) 
	{
		PrintErrorMessage("Expected argument.");
		return;
	}
	name_out = directive.begin()->value;
	directive.pop_front();
	if (!directive.empty()) PrintErrorMessage("Too many arguments.");
}

void Preprocessor::parsePragma(Preprocessor::LexemList& args)
{
	args.pop_front();
	if (args.empty())
	{
		PrintErrorMessage("Pragmas need arguments.");
		return;
	}
	UtlString p_name = args.begin()->value;
	args.pop_front();
	UtlString p_args;
	if (!args.empty())
	{
		if (args.begin()->type != Preprocessor::LEX_STRING) 
			PrintErrorMessage("Pragma parameter should be a string literal.");
		p_args = removeQuotes(args.begin()->value);
		args.pop_front();
	}
	if (!args.empty()) PrintErrorMessage("Too many parameters to pragma.");

	Preprocessor::PragmaInstance PI;
	PI.name = p_name;
	PI.text = p_args;
	PI.current_file = current_file;
	PI.current_file_line = lines_this_file;
	PI.root_file = root_file;
	PI.global_line = current_line;
	callPragma(p_name,PI);
}

void Preprocessor::recursivePreprocess(	
	const UtlString& filename,
	const UtlString& filedata,
	Preprocessor::FileSource& file_source,
	Preprocessor::LexemList& lexems,
	Preprocessor::DefineTable& define_table,
	const bool fromString)
{
	unsigned int start_line = current_line;
	lines_this_file = 0;
	current_file = filename;
	setFileMacro(define_table,current_file);
	setLineMacro(define_table,lines_this_file);

	UtlVector<char> data;

	if ( fromString ) {
		data.resize( filedata.length() );
		memcpy( data.data(), filedata.data(), data.size() );
	}
	else {
		const bool loaded = file_source.LoadFile( filename, data );
		if ( !loaded ) {
			PrintErrorMessage( va( "Could not open file %s", filename.c_str() ) );
			return;
		}
	}

	if ( data.size() == 0 ) {
		return;
	}

	data.push_back( '\n' );
	char *d_end = &data[ data.size() - 1 ];
	++d_end;
	lex( &data[0], d_end, lexems );

	Preprocessor::LexemList::iterator ITR = lexems.begin();
	Preprocessor::LexemList::iterator END = lexems.end();
	while ( ITR != END ) {
		if ( ITR->type == Preprocessor::LEX_NEWLINE ) {
			current_line++;
			lines_this_file++;
			++ITR;
			setLineMacro( define_table, lines_this_file );
		}
		else if ( ITR->type == Preprocessor::LEX_PREPROCESSOR ) {
			Preprocessor::LLITR start_of_line = ITR;
			Preprocessor::LLITR end_of_line = findLexem(ITR, END, LEX_NEWLINE);
			Preprocessor::LexemList directive(start_of_line, end_of_line);

			Con_Printf( "Processing macro of \"%s\"...\n", directive.begin()->value.c_str() );
			if ( directive.begin()->value.find( "#include" ) != UtlString::npos ) {
				ITR = end_of_line;
				continue;
			} else {
				ITR = lexems.erase(start_of_line, end_of_line);
			}

			UtlString value = directive.begin()->value;
			if ( value.find( "#define" ) != UtlString::npos ) {
				parseDefine(define_table, directive);
			}
			else if ( value.find( "#ifdef" ) != UtlString::npos ) {
				UtlString def_name;
				parseIf(directive, def_name);
				Preprocessor::DefineTable::iterator DTI = define_table.find(def_name);
				if (DTI == define_table.end())
				{
					Preprocessor::LLITR splice_to = parseIfDef(ITR,END);
					ITR = lexems.erase(ITR, splice_to);
				}
			} 
			else if ( value.find( "#if" ) != UtlString::npos )
			{
				UtlString def_name;
				parseIf(directive,def_name);
				Preprocessor::DefineTable::iterator DTI = define_table.find(def_name);
				if (DTI != define_table.end())
				{
					Preprocessor::LLITR splice_to = parseIfDef(ITR,END);
					ITR = lexems.erase(ITR, splice_to);
				}
			}
			else if ( value.find( "#endif" ) != UtlString::npos )
			{
				//ignore
			}
			else if (value == "#include")
			{
				/*
				if ( LNT ) {
					LNT->AddLineRange(filename, start_line, current_line - lines_this_file);
				}
				const unsigned int save_lines_this_file = lines_this_file;
				UtlString file_name;
				parseIf(directive, file_name);
				Preprocessor::LexemList next_file;
				recursivePreprocess(
					addPaths(filename,removeQuotes(file_name)),
					"",
					file_source,
					next_file,
					define_table,
					false);
				lexems.splice(ITR, next_file);
				start_line = current_line;
				lines_this_file = save_lines_this_file;
				current_file = filename;
				setFileMacro(define_table, current_file);
				setLineMacro(define_table, lines_this_file);
				*/
			}
			else if ( value.find( "#pragma" ) != UtlString::npos )
			{
				parsePragma(directive);
			}
			else if ( value.find( "#warning" ) != UtlString::npos )
			{
				UtlString msg;
				parseIf(directive, msg);
				PrintWarningMessage(msg);
			}
			else 
			{
				PrintErrorMessage("Unknown directive.");
			}
		} 
		else if (ITR->type == LEX_IDENTIFIER)
		{ 
			ITR = expandDefine(ITR, END, lexems, define_table);
		} 
		else { ++ITR; }
	}

	if (LNT) LNT->AddLineRange(filename, start_line, current_line - lines_this_file);
}

int Preprocessor::preprocess(
	const UtlString& source_file,
	const UtlString& source_data,
	Preprocessor::FileSource& file_source,
	UtlVector<char>& destination,
	Preprocessor::LineNumberTranslator* trans)
{
	if ( trans ) {
		LNT = new Preprocessor::LineNumberTranslator::Table;
	} else {
		LNT = 0;
	}

	current_file = source_file;
	current_line = 0;
	Preprocessor::DefineTable define_table = application_specified;
	Preprocessor::LexemList lexems;
	number_of_errors = 0;
	root_file = source_file;
	
	recursivePreprocess( (const char *)source_file.c_str(),
	                    (const char *)source_data.c_str(),
	                    file_source,
	                    lexems,
	                    define_table,
	                    true );
	printLexemList(lexems, destination);
	
	if (trans) {
		trans->SetTable(LNT);
		LNT = 0;
	}
	
	return number_of_errors;
}

void Preprocessor::define( const char *str )
{
	if ( !str || !strlen( str ) ) {
        return;
    }
	UtlString data;

    data.sprintf( "#define %s", str );
	
    char *d_end = &data[ data.length() - 1 ];
	++d_end;
	Preprocessor::LexemList lexems;
	lex(&data[0],d_end,lexems);

	parseDefine(application_specified,lexems);
}