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


#ifndef _includedh_aatc_container_mapped_templated_shared_method
#define _includedh_aatc_container_mapped_templated_shared_method



#include "aatc_container_mapped_templated_shared.hpp"



BEGIN_AS_NAMESPACE
namespace aatc {
	namespace container {
		namespace mapped {
			namespace templated {
				namespace shared {



					namespace method {

						template<typename T_container> void swap(T_container* t, T_container& other) {
							t->container.swap(other.container);
							t->safety_iteratorversion_Increment();
							other.safety_iteratorversion_Increment();
						}


						template<typename T_container> void insert(T_container* t, void* key, void* value) {
							#if aatc_CONFIG_ENABLE_ERRORCHECK_RUNTIME
								if (t->need_errorcheck_missing_functions) {
									if (t->missing_functions & common::CONTAINER_OPERATION::INSERT) {
										common::errorprint::container::missingfunctions_operation_missing(t->typeinfo_container->GetName(), t->typeinfo_key->GetName(), "insert");
										return;
									}
								}
							#endif

							t->safety_iteratorversion_Increment();

							common::primunion findkey;

							t->BuildPrimunion(findkey, key, t->datahandlingid_key, t->primitiveid_key);

							typename T_container::T_iterator_native it = t->container.find(findkey);
							if (it == t->container.end()) {
								common::primunion_pair insertpair;

								t->store_Scriptany_to_Primunion(key, insertpair.first, t->datahandlingid_key, t->primitiveid_key, t->typeinfo_key);
								t->store_Scriptany_to_Primunion(value, insertpair.second, t->datahandlingid_value, t->primitiveid_value, t->typeinfo_value);

								t->container.insert(insertpair);
							}
						}



						template<typename T_container> void erase(T_container* t, void* key) {
							#if aatc_CONFIG_ENABLE_ERRORCHECK_RUNTIME
								if (t->need_errorcheck_missing_functions) {
									if (t->missing_functions & common::CONTAINER_OPERATION::ERASE_VALUE) {
										common::errorprint::container::missingfunctions_operation_missing(t->typeinfo_container->GetName(), t->typeinfo_key->GetName(), "erase");
										return;
									}
								}
							#endif

							t->safety_iteratorversion_Increment();

							common::primunion findkey;

							t->BuildPrimunion(findkey, key, t->datahandlingid_key, t->primitiveid_key);

							typename T_container::T_iterator_native it = t->container.find(findkey);

							if (it != t->container.end()) {
								common::primunion old_key;
								common::primunion old_value;

								old_key.ptr = (*it).first.ptr;
								old_value.ptr = (*it).second.ptr;

								t->container.erase(it);

								if (t->datahandlingid_key != common::DATAHANDLINGTYPE::PRIMITIVE) {
									t->engine->ReleaseScriptObject(old_key.ptr, t->typeinfo_key);
								}
								if (t->datahandlingid_value != common::DATAHANDLINGTYPE::PRIMITIVE) {
									t->engine->ReleaseScriptObject(old_value.ptr, t->typeinfo_value);
								}
							}
						}


						template<typename T_container> void* find_value(T_container* t, void* key, bool& success) {
							#if aatc_CONFIG_ENABLE_ERRORCHECK_RUNTIME
								if (t->need_errorcheck_missing_functions) {
									if (t->missing_functions & common::CONTAINER_OPERATION::FIND) {
										common::errorprint::container::missingfunctions_operation_missing(t->typeinfo_container->GetName(), t->typeinfo_key->GetName(), "find");
										return T_container::DefaultPrimunion(t->datahandlingid_value, t->primitiveid_value);
									}
								}
							#endif

							common::primunion findkey;
							t->BuildPrimunion(findkey, key, t->datahandlingid_key, t->primitiveid_key);

							typename T_container::T_iterator_native_const it = t->container.find(findkey);
							if (it == t->container.end()) {
								success = 0;

								return T_container::DefaultPrimunion(t->datahandlingid_value, t->primitiveid_value);
							} else {
								success = 1;

								const common::primunion& found_value_const = it->second;
								common::primunion& found_value = const_cast<common::primunion&>(found_value_const);

								return T_container::Scriptany_ref_from_Primunion(found_value, t->datahandlingid_value, t->primitiveid_value);
							}
						}

						template<typename T_container> void* find_value(T_container* t, void* key) {
							bool success;
							return find_value(t, key, success);
						}

						template<typename T_container> bool contains(T_container* t, void* key) {
							bool find_success = 0;
							find_value(t, key, find_success);
							return find_success;
						}






						template<typename T_container> typename T_container::Iterator find_iterator(T_container* t, void* key) {
							#if aatc_CONFIG_ENABLE_ERRORCHECK_RUNTIME
								if (t->need_errorcheck_missing_functions) {
									if (t->missing_functions & common::CONTAINER_OPERATION::FIND) {
										common::errorprint::container::missingfunctions_operation_missing(t->typeinfo_container->GetName(), t->typeinfo_key->GetName(), "find");
										return t->end();
									}
								}
							#endif

							common::primunion findkey;
							t->BuildPrimunion(findkey, key, t->datahandlingid_key, t->primitiveid_key);

							typename T_container::T_iterator_native it = t->container.find(findkey);


							typename T_container::Iterator result(t);
							result.it = it;

							if (it == t->container.end()) {
								result.SetToEnd();
							}

							return result;
						}

						template<typename T_container> bool erase_iterator(T_container* t, const typename T_container::Iterator& aatc_it) {
							#if aatc_CONFIG_ENABLE_ERRORCHECK_ITERATOR_SAFETY_VERSION_NUMBERS
								if (t->safety_iteratorversion != aatc_it.safety_iteratorversion) {
									common::errorprint::container::iterator_invalid();
									return 0;
								}
							#endif

							typename T_container::T_iterator_native it = aatc_it.it;

							if (it == t->container.end()) {
								return 0;
							} else {
								t->safety_iteratorversion_Increment();

								common::primunion old_key;
								common::primunion old_value;

								if (t->datahandlingid_key != common::DATAHANDLINGTYPE::PRIMITIVE) { old_key.ptr = (*it).first.ptr; }
								if (t->datahandlingid_value != common::DATAHANDLINGTYPE::PRIMITIVE) { old_value.ptr = (*it).second.ptr; }

								t->container.erase(it);

								switch (t->datahandlingid_key) {
								case common::DATAHANDLINGTYPE::PRIMITIVE: { break; }
								case common::DATAHANDLINGTYPE::STRING:
								{
									t->engine->ReleaseScriptObject(old_key.ptr, t->typeinfo_key);
									break;
								}
								default:
								{
									t->engine->ReleaseScriptObject(old_key.ptr, t->typeinfo_key);
									break;
								}
								};
								switch (t->datahandlingid_value) {
								case common::DATAHANDLINGTYPE::PRIMITIVE: { break; }
								case common::DATAHANDLINGTYPE::STRING:
								{
									t->engine->ReleaseScriptObject(old_value.ptr, t->typeinfo_value);
									break;
								}
								default:
								{
									t->engine->ReleaseScriptObject(old_value.ptr, t->typeinfo_value);
									break;
								}
								};

								return 1;
							}
						}

						template<typename T_container> config::t::sizetype erase_iterator_range(T_container* t, const typename T_container::Iterator& aatc_it_range_begin, const typename T_container::Iterator& aatc_it_range_end) {
							#if aatc_CONFIG_ENABLE_ERRORCHECK_ITERATOR_SAFETY_VERSION_NUMBERS
								if ((t->safety_iteratorversion != aatc_it_range_begin.safety_iteratorversion) || (t->safety_iteratorversion != aatc_it_range_end.safety_iteratorversion)) {
									common::errorprint::container::iterator_invalid();
									return 0;
								}
							#endif


							typename T_container::T_iterator_native it_range_begin = aatc_it_range_begin.it;
							typename T_container::T_iterator_native it_range_end = aatc_it_range_end.it;

							if (it_range_begin == it_range_end) {
								return 0;
							} else {
								t->safety_iteratorversion_Increment();

								config::t::sizetype delcount = (config::t::sizetype)eastl::distance(it_range_begin, it_range_end);

								eastl::vector<eastl::pair<common::primunion, common::primunion>> old_items;

								int nonprimitives = (t->datahandlingid_key != common::DATAHANDLINGTYPE::PRIMITIVE) + (t->datahandlingid_value != common::DATAHANDLINGTYPE::PRIMITIVE);

								if (nonprimitives) {
									old_items.reserve(delcount);

									if (nonprimitives == 2) {
										for (auto it = it_range_begin; it != it_range_end; it++) {
											eastl::pair<common::primunion, common::primunion> pp;
											pp.first.ptr = (*it).first.ptr;
											pp.second.ptr = (*it).second.ptr;
											old_items.push_back(pp);
										}
									} else {
										if (t->datahandlingid_key != common::DATAHANDLINGTYPE::PRIMITIVE) {
											for (auto it = it_range_begin; it != it_range_end; it++) {
												eastl::pair<common::primunion, common::primunion> pp;
												pp.first.ptr = (*it).first.ptr;
												old_items.push_back(pp);
											}
										} else {
											for (auto it = it_range_begin; it != it_range_end; it++) {
												eastl::pair<common::primunion, common::primunion> pp;
												pp.second.ptr = (*it).second.ptr;
												old_items.push_back(pp);
											}
										}
									}
								}

								t->container.erase(it_range_begin, it_range_end);

								if (nonprimitives) {
									if (nonprimitives == 2) {
										for (auto it = old_items.begin(); it != old_items.end(); it++) {
											t->engine->ReleaseScriptObject((*it).first.ptr, t->typeinfo_key);
											t->engine->ReleaseScriptObject((*it).second.ptr, t->typeinfo_value);
										}
									} else {
										if (t->datahandlingid_key != common::DATAHANDLINGTYPE::PRIMITIVE) {
											for (auto it = old_items.begin(); it != old_items.end(); it++) {
												t->engine->ReleaseScriptObject((*it).first.ptr, t->typeinfo_key);
											}
										} else {
											for (auto it = old_items.begin(); it != old_items.end(); it++) {
												t->engine->ReleaseScriptObject((*it).second.ptr, t->typeinfo_value);
											}
										}
									}
								}

								return delcount;
							}
						}



						namespace cpp_interface {
							template<typename T_container> void* operator_index(T_container* t, void* key) {
								bool find_success = 0;
								void* find_result = find_value(t, key, find_success);

								if (find_success) {
									return find_result;
								} else {
									common::primunion_pair insertpair;

									t->store_Scriptany_to_Primunion(key, insertpair.first, t->datahandlingid_key, t->primitiveid_key, t->typeinfo_key);
									t->DefaultConstructPrimunion(insertpair.second, t->datahandlingid_value, t->primitiveid_value, t->typeinfo_value);

									eastl::pair<typename T_container::T_iterator_native,bool> insert_result = t->container.insert(insertpair);

									return T_container::Scriptany_ref_from_Primunion(insert_result.first->second, t->datahandlingid_value, t->primitiveid_value);
								}
							}
						};



					};//namespace method



					namespace register_method {

						template<typename T_container> inline void swap(common::RegistrationState& rs) {
							rs.Format("%s& %s(%s &inout)", rs.n_container_T, config::scriptname::method::container::swap, rs.n_container_T);
							CheckASCall( rs.engine->RegisterObjectMethod(rs.n_container_T, rs.textbuf, WRAP_MFN(T_container,swap), asCALL_GENERIC ) );
						}

						template<typename T_container> inline void insert(common::RegistrationState& rs) {
							rs.Format("void %s(const T_key&in,const T_value&in)", config::scriptname::method::container::insert);
							CheckASCall( rs.engine->RegisterObjectMethod(rs.n_container_T, rs.textbuf, WRAP_OBJ_FIRST(method::insert<T_container>), asCALL_GENERIC ) );
						}

						template<typename T_container> inline void erase(common::RegistrationState& rs) {
							rs.Format("void %s(const T_key&in)", config::scriptname::method::container::erase);
							CheckASCall( rs.engine->RegisterObjectMethod(rs.n_container_T, rs.textbuf, WRAP_OBJ_FIRST(method::erase<T_container>), asCALL_GENERIC ) );
						}

						template<typename T_container> inline void find(common::RegistrationState& rs) {
							rs.Format("T_value& %s(const T_key &in)", config::scriptname::method::container::find);
							CheckASCall( rs.engine->RegisterObjectMethod(rs.n_container_T, rs.textbuf, WRAP_OBJ_FIRST_PR(method::find_value<T_container>,(T_container*, void*),void*), asCALL_GENERIC ) );

							rs.Format("T_value& %s(const T_key &in,bool &out)", config::scriptname::method::container::find);
							CheckASCall( rs.engine->RegisterObjectMethod(rs.n_container_T, rs.textbuf, WRAP_OBJ_FIRST_PR(method::find_value<T_container>, (T_container*, void*, bool&), void*), asCALL_GENERIC ) );

							rs.Format("bool %s(const T_key&in) const", config::scriptname::method::container::contains);
							CheckASCall( rs.engine->RegisterObjectMethod(rs.n_container_T, rs.textbuf, WRAP_OBJ_FIRST(method::contains<T_container>), asCALL_GENERIC ) );
						}

						template<typename T_container> inline void find_iterator(common::RegistrationState& rs) {
							rs.Format("%s %s(const T_key &in)", rs.n_iterator_T, config::scriptname::method::container::find_iterator);
							CheckASCall( rs.engine->RegisterObjectMethod(rs.n_container_T, rs.textbuf, WRAP_OBJ_FIRST(method::find_iterator<T_container>), asCALL_GENERIC ) );

							rs.Format("%s %s(const T_key &in) const", rs.n_iterator_T, config::scriptname::method::container::find_iterator);
							CheckASCall( rs.engine->RegisterObjectMethod(rs.n_container_T, rs.textbuf, WRAP_OBJ_FIRST(method::find_iterator<T_container>), asCALL_GENERIC ) );
						}

						template<typename T_container> inline void erase_iterator(common::RegistrationState& rs) {
							rs.Format("bool %s(const %s &in)", config::scriptname::method::container::erase_iterator, rs.n_iterator_T);
							CheckASCall( rs.engine->RegisterObjectMethod(rs.n_container_T, rs.textbuf, WRAP_OBJ_FIRST(method::erase_iterator<T_container>), asCALL_GENERIC ) );
						}
						template<typename T_container> inline void erase_iterator_range(common::RegistrationState& rs) {
							rs.Format("%s %s(const %s &in,const %s &in)", config::scriptname::t::size, config::scriptname::method::container::erase_iterator, rs.n_iterator_T, rs.n_iterator_T);
							CheckASCall( rs.engine->RegisterObjectMethod(rs.n_container_T, rs.textbuf, WRAP_OBJ_FIRST(method::erase_iterator_range<T_container>), asCALL_GENERIC ) );
						}

						template<typename T_container> inline void operator_index(common::RegistrationState& rs) {
							CheckASCall( rs.engine->RegisterObjectMethod(rs.n_container_T, "const T_value& get_opIndex(const T_key &in) const", WRAP_OBJ_FIRST_PR(method::find_value<T_container>, (T_container*, void*), void*), asCALL_GENERIC ) );
							CheckASCall( rs.engine->RegisterObjectMethod(rs.n_container_T, "void set_opIndex(const T_key&in,const T_value&in)", WRAP_OBJ_FIRST(method::insert<T_container>), asCALL_GENERIC ) );
						}



					};//namespace register_method



				};//namespace shared
			};//namespace templated
		};//namespace mapped
	};//namespace container
};//namespace aatc
END_AS_NAMESPACE



#endif