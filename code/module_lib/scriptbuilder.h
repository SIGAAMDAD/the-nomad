#ifndef SCRIPTBUILDER_H
#define SCRIPTBUILDER_H

//---------------------------
// Compilation settings
//

// Set this flag to turn on/off metadata processing
//  0 = off
//  1 = on
#ifndef AS_PROCESS_METADATA
#define AS_PROCESS_METADATA 1
#endif

// TODO: Implement flags for turning on/off include directives and conditional programming



//---------------------------
// Declaration
//

#include "module_public.h"


#if defined(_MSC_VER) && _MSC_VER <= 1200
// disable the annoying warnings on MSVC 6
#pragma warning (disable:4786)
#endif

class CScriptBuilder;

// This callback will be called for each #include directive encountered by the
// builder. The callback should call the AddSectionFromFile or AddSectionFromMemory
// to add the included section to the script. If the include cannot be resolved
// then the function should return a negative value to abort the compilation.
typedef int (*INCLUDECALLBACK_t)(const char *include, const char *from, CScriptBuilder *builder, void *userParam);

// This callback will be called for each #pragma directive encountered by the builder.
// The application can interpret the pragmaText and decide what do to based on that.
// If the callback returns a negative value the builder will report an error and abort the compilation.
typedef int(*PRAGMACALLBACK_t)(const UtlString &pragmaText, CScriptBuilder &builder, void *userParam);

// Helper class for loading and pre-processing script files to
// support include directives and metadata declarations
class CScriptBuilder
{
public:
	CScriptBuilder();

	// Start a new module
	int StartNewModule(asIScriptEngine *engine, const char *moduleName);

	// Load a script section from a file on disk
	// Returns  1 if the file was included
	//          0 if the file had already been included before
	//         <0 on error
	int AddSectionFromFile(const char *filename);

	// Load a script section from memory
	// Returns  1 if the section was included
	//          0 if a section with the same name had already been included before
	//         <0 on error
	int AddSectionFromMemory(const char *sectionName,
							 const char *scriptCode,
							 unsigned int scriptLength = 0,
							 int lineOffset = 0);

	// Build the added script sections
	int BuildModule();

	// Returns the engine
	asIScriptEngine *GetEngine();

	// Returns the current module
	asIScriptModule *GetModule();

	// Register the callback for resolving include directive
	void SetIncludeCallback(INCLUDECALLBACK_t callback, void *userParam);

	// Register the callback for resolving pragma directive
	void SetPragmaCallback(PRAGMACALLBACK_t callback, void *userParam);

	// Add a pre-processor define for conditional compilation
	void DefineWord(const char *word);

	// Enumerate included script sections
	unsigned int GetSectionCount() const;
	const UtlString&  GetSectionName(unsigned int idx) const;

#ifdef AS_PROCESS_METADATA
	// Get metadata declared for classes, interfaces, and enums
	const UtlVector<UtlString>& GetMetadataForType(int typeId);

	// Get metadata declared for functions
	const UtlVector<UtlString>& GetMetadataForFunc(asIScriptFunction *func);

	// Get metadata declared for global variables
	const UtlVector<UtlString>& GetMetadataForVar(int varIdx);

	// Get metadata declared for class variables
	const UtlVector<UtlString>& GetMetadataForTypeProperty(int typeId, int varIdx);

	// Get metadata declared for class methods
	const UtlVector<UtlString>& GetMetadataForTypeMethod(int typeId, asIScriptFunction *method);
#endif

protected:
	void ClearAll();
	int  Build();
	int  ProcessScriptSection(const char *script, unsigned int length, const char *sectionname, int lineOffset);
	int  LoadScriptSection(const char *filename);
	bool IncludeIfNotAlreadyIncluded(const char *filename);

	int  SkipStatement(int pos);

	int  ExcludeCode(int start);
	void OverwriteCode(int start, int len);

	asIScriptEngine           *engine;
	asIScriptModule           *module;
	UtlString                modifiedScript;

	INCLUDECALLBACK_t  includeCallback;
	void              *includeParam;

	PRAGMACALLBACK_t  pragmaCallback;
	void             *pragmaParam;

#ifdef AS_PROCESS_METADATA
	int  ExtractMetadata(int pos, UtlVector<UtlString> &outMetadata);
	int  ExtractDeclaration(int pos, UtlString &outName, UtlString &outDeclaration, int &outType);

	enum METADATATYPE
	{
		MDT_TYPE = 1,
		MDT_FUNC = 2,
		MDT_VAR = 3,
		MDT_VIRTPROP = 4,
		MDT_FUNC_OR_VAR = 5
	};

	// Temporary structure for storing metadata and declaration
	struct SMetadataDecl
	{
		SMetadataDecl( const UtlVector<UtlString>& m, const UtlString& n, const UtlString& d, int t, const UtlString& c, const UtlString& ns )
			: metadata(m), name(n), declaration(d), type(t), parentClass(c), nameSpace(ns) {}
		UtlVector<UtlString> metadata;
		UtlString              name;
		UtlString              declaration;
		int                      type;
		UtlString              parentClass;
		UtlString              nameSpace;
	};
	UtlVector<SMetadataDecl> foundDeclarations;
	UtlString currentClass;
	UtlString currentNamespace;

	// Storage of metadata for global declarations
	UtlHashMap<int, UtlVector<UtlString> > typeMetadataMap;
	UtlHashMap<int, UtlVector<UtlString> > funcMetadataMap;
	UtlHashMap<int, UtlVector<UtlString> > varMetadataMap;

	// Storage of metadata for class member declarations
	struct SClassMetadata
	{
		SClassMetadata(const UtlString& aName) : className(aName) {}
		UtlString className;
		UtlHashMap<int, UtlVector<UtlString> > funcMetadataMap;
		UtlHashMap<int, UtlVector<UtlString> > varMetadataMap;
	};
	UtlHashMap<int, SClassMetadata> classMetadataMap;

#endif

#ifdef _WIN32
	// On Windows the filenames are case insensitive so the comparisons to
	// avoid duplicate includes must also be case insensitive. True case insensitive
	// is not easy as it must be language aware, but a simple implementation such
	// as strcmpi should suffice in almost all cases.
	//
	// ref: http://www.gotw.ca/gotw/029.htm
	// ref: https://msdn.microsoft.com/en-us/library/windows/desktop/dd317761(v=vs.85).aspx
	// ref: http://site.icu-project.org/

	// TODO: Strings by default are treated as UTF8 encoded. If the application choses to
	//       use a different encoding, the comparison algorithm should be adjusted as well

	struct ci_less
	{
		bool operator()(const UtlString &a, const UtlString &b) const
		{
			return _stricmp(a.c_str(), b.c_str()) < 0;
		}
	};
	UtlSet<UtlString, ci_less> includedScripts;
#else
	UtlSet<UtlString>      includedScripts;
#endif

	UtlSet<UtlString>      definedWords;
};

END_AS_NAMESPACE

#endif
