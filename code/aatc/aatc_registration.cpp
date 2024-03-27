/*
The zlib/libpng License
http://opensource.org/licenses/zlib-license.php


Angelscript addon Template Containers
Copyright (c) 2014 Sami Vuorela

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1.	The origin of this software must not be misrepresented;
You must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source distribution.


Sami Vuorela
samivuorela@gmail.com
*/


#include "../engine/n_shared.h"
#include "../engine/n_common.h"
//#include "../module_lib/module_alloc.h"

//#include "aatc_internal_lists.hpp"
#include "aatc.hpp"
#include "aatc_common.hpp"
#include "aatc_hash.hpp"
#include "aatc_enginestorage.hpp"
#include "aatc_container_listing.hpp"
#include "aatc_templatemagic.hpp"
#include "../module_lib/aswrappedcall.h"




BEGIN_AS_NAMESPACE
namespace aatc {



	//used with templatemagic::iterator_functor_1arg to iterate in runtime, over a list of all container types defined in compile time
	template<int i> class staticiterate_initializer_register_all_containers {
	public:
		void operator()(Initializer* initializer, asIScriptEngine* engine) {
			if (initializer->include_container[i]) {
				enginestorage::engine_level_storage* els = enginestorage::Get_ELS(engine);
				enginestorage::containertype_specific_storage& ctss = els->containertype_specific_storages[i];

				ctss.els = els;
				ctss.container_id = i;
				ctss.func_errorcheck_missing_functions_make_bitfield_for_template = container::listing::errorcheck_missing_functions_make_bitfield_for_template<i>;

				container::listing::register_container<i>(engine);
			}
		}
	};



	void Initializer::Go() {
		common::primunion_defaultvalue.ui64 = 0;
		common::primunion_defaultvalue.ptr = nullptr;

		engine->SetUserData(new enginestorage::engine_level_storage(engine), config::detail::engine_userdata_id);
		engine->SetEngineUserDataCleanupCallback(enginestorage::engine_cleanup, config::detail::engine_userdata_id);

		#if aatc_ENABLE_REGISTER_TYPEDEF_HASH_TYPE
			engine->RegisterTypedef(config::scriptname::t::hash, config::scriptname::t::hash_actual);
		#endif

		{
			char textbuf[common::RegistrationState::bufsize];

			{//register script_Funcpointer
				using common::script_Funcpointer;

				const char* n_funcpointer = config::scriptname::funcpointer;

				CheckASCall( engine->RegisterObjectType(n_funcpointer, 0, asOBJ_REF ) );

				common::RegistrationState::Format_static(textbuf, common::RegistrationState::bufsize, "%s@ f()", n_funcpointer);

				CheckASCall( engine->RegisterObjectBehaviour( n_funcpointer, asBEHAVE_FACTORY, textbuf, WRAP_FN_PR( script_Funcpointer::Factory, (), script_Funcpointer * ), asCALL_GENERIC ) );

				CheckASCall( engine->RegisterObjectBehaviour(n_funcpointer, asBEHAVE_ADDREF, "void f()", WRAP_MFN_PR( script_Funcpointer::basetype_refcounted, refcount_Add, (), void), asCALL_GENERIC ) );
				CheckASCall( engine->RegisterObjectBehaviour(n_funcpointer, asBEHAVE_RELEASE, "void f()", WRAP_MFN_PR( script_Funcpointer::basetype_refcounted, refcount_Release, (), void), asCALL_GENERIC ) );

				CheckASCall( engine->RegisterObjectProperty(n_funcpointer, "bool ready", asOFFSET(script_Funcpointer, ready)) );
				CheckASCall( engine->RegisterObjectProperty(n_funcpointer, "bool is_thiscall", asOFFSET(script_Funcpointer, is_thiscall)) );
				CheckASCall( engine->RegisterObjectProperty(n_funcpointer, "string funcname", asOFFSET(script_Funcpointer, funcname)) );

				CheckASCall( engine->RegisterObjectMethod(n_funcpointer, "bool Set(string)", WRAP_MFN_PR(script_Funcpointer, Set, (config::t::string), bool), asCALL_GENERIC ) );
				CheckASCall( engine->RegisterObjectMethod(n_funcpointer, "bool Set(string,?&in)", WRAP_MFN_PR(script_Funcpointer, Set, (config::t::string, void*, int), bool), asCALL_GENERIC ) );
				CheckASCall( engine->RegisterObjectMethod(n_funcpointer, "void Call()", WRAP_MFN_PR(script_Funcpointer, scriptsidecall_CallVoid, (), void), asCALL_GENERIC ) );
			}

			{//register hash functions
				common::RegistrationState::Format_static(textbuf, 1000, "%s aatc_Hashfunc_djb2(string &in)", config::scriptname::t::hash_actual);
				CheckASCall( engine->RegisterGlobalFunction(textbuf, WRAP_FN(hash::hashfunc::djb2), asCALL_GENERIC ) );
			}
		}

		templatemagic::staticiterate_2arg<
			0,
			container::listing::CONTAINER::_COUNT - 1,
			staticiterate_initializer_register_all_containers,

			Initializer*,
			asIScriptEngine*
		> f; 
		f(this, engine);
	}


};//namespace aatc
END_AS_NAMESPACE