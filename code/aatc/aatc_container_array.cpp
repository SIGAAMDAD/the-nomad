#include "../engine/n_shared.h"
#include "../engine/n_common.h"
#include "../module_lib/module_alloc.h"

#include "aatc_container_array.hpp"

#include "aatc_container_listing.hpp"
#include "aatc_container_templated_shared_method.hpp"
#include "aatc_container_templated_shared.hpp"


BEGIN_AS_NAMESPACE
namespace aatc {
	namespace container {
		namespace listing {



			const char* container::listing::tags_of_container::array::scriptname_container = config::scriptname::container::array;



		}//namespace listing
		namespace templated {



			array::array(asITypeInfo* _typeinfo) :
				Containerbase(_typeinfo->GetEngine(), _typeinfo)
			{}
			array::array(const array& other) :
				Containerbase(other.engine, other.typeinfo_container)
			{
				(*this) = other;
			}
			array& array::operator=(const array& other) { Containerbase::operator=(other); return *this; }

			array& array::swap(array& other) {
				shared::method::swap(this, other);
				return *this;
			}

			void* array::back() { return shared::method::native::back(this); }
			void* array::front() { return shared::method::native::front(this); }

			void* array::operator[](config::t::sizetype position) { return shared::method::native::operator_index_position(this, position); }

			void array::sort(bool ascending) { shared::method::genericcc::sort(this, ascending); }
			void array::sort(common::script_Funcpointer* funcptr, bool ascending) { shared::method::genericcc::sort_aatcfuncptr(this,funcptr, ascending); }

            array::Iterator array::find(void* value) { return shared::method::genericcc::find_iterator(this, value); }

			bool array::contains(void* value) { return shared::method::genericcc::contains(this, value); }
			config::t::sizetype array::count(void* value) { return shared::method::genericcc::count(this, value); }



		};//namespace templated
		namespace listing {



			template<> void register_container<CONTAINER::VECTOR>(asIScriptEngine* engine) {
				common::RegistrationState rs(engine);

				{
					using namespace templated::shared;
					typedef templated::array T_container;

					register_containerbase<T_container>(rs);
                    register_container_datalist_base<T_container>( rs );

					register_method::swap<T_container>(rs);

					register_method::native::back<T_container>(rs);
					register_method::native::front<T_container>(rs);

					register_method::native::operator_index_position<T_container>(rs);

					register_method::genericcc::sort<T_container>(rs);

					register_method::genericcc::find_iterator<T_container>(rs);

					register_method::genericcc::contains<T_container>(rs);
					register_method::genericcc::count<T_container>(rs);
				}

				container::shared::autoregister::register_all_tempspec_basics_for_container<tempspec::array>(engine);
			}
			template<> common::container_operations_bitmask_type errorcheck_missing_functions_make_bitfield_for_template<CONTAINER::ARRAY>(enginestorage::template_specific_storage* tss) {
				common::container_operations_bitmask_type mask = 0;
				
				if (!tss->func_cmp) {
					mask |= common::CONTAINER_OPERATION::SORT;
				}
				if (!tss->func_equals) {
					mask |= common::CONTAINER_OPERATION::COUNT;
				}
				
				return mask;
			}



		};//namespace listing



	};//namespace container
};//namespace aatc

END_AS_NAMESPACE