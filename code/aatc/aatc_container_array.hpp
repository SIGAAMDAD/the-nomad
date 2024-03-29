#ifndef _includedh_aatc_container_array
#define _includedh_aatc_container_array


#include "aatc_common.hpp"
#include "aatc_container_templated_shared.hpp"
#include "aatc_container_tempspec_shared_method.hpp"


BEGIN_AS_NAMESPACE
namespace aatc {
	namespace container {



		namespace templated {



			class array : public shared::Containerbase <
				aatc_acit_array<void*>,
				container::listing::CONTAINER::ARRAY,
				container::listing::tags_of_container::array
			> {
			public:
				array(asITypeInfo* typeinfo);
				array(const array& other);
				array& operator=(const array& other);
				array& swap(array& other);

				void* back();
				void* front();

				void* operator[](config::t::sizetype position);

				void sort(bool ascending = true);
				void sort(common::script_Funcpointer* funcptr, bool ascending = true);

				Iterator find(void* value);

				bool contains(void* value);
				config::t::sizetype count(void* value);
			};



		};//namespace templated
		namespace tempspec {



			template<typename T_content> class array : public shared::Containerbase <
				aatc_acit_array<T_content>,
				T_content,
				container::listing::CONTAINER::ARRAY,
				container::listing::tags_of_container::array
			> {
			public:
				typedef shared::Containerbase <
					aatc_acit_array<T_content>,
					T_content,
					container::listing::CONTAINER::ARRAY,
					container::listing::tags_of_container::array
				> Containerbase;
				typedef typename Containerbase::Iterator Iterator;



				array() {}
				array(const array& other):
					Containerbase(other)
				{}
				array& operator=(const array& other) { Containerbase::operator=(other); return *this; }
				array& swap(array& other) { shared::method::swap(this, other); return *this; }

				T_content& back() { return shared::method::native::back(this); }
				T_content& front() { return shared::method::native::front(this); }
				T_content& operator[](config::t::sizetype position) { return shared::method::native::operator_index_position(this,position); }

				Iterator find(const T_content& value) { return shared::method::genericcc::find_iterator(this, value); }

				void sort(bool ascending = true) { shared::method::genericcc::sort(this, ascending); }
				void sort(common::script_Funcpointer* funcptr, bool ascending = true) { shared::method::genericcc::sort_aatcfuncptr(this, funcptr, ascending); }

				bool contains(const T_content& value) { return shared::method::genericcc::contains(this, value); }
				config::t::sizetype count(const T_content& value) { return shared::method::genericcc::count(this, value); }


				static void Register(common::RegistrationState& rs, const char* n_content) {
					using namespace tempspec::shared;
					typedef array T_container;

					register_containerbase<T_container>(rs, n_content);
					register_method::swap<T_container>(rs);

					register_method::native::back<T_container>(rs);
					register_method::native::front<T_container>(rs);

					register_method::native::operator_index_position<T_container>(rs);

					register_method::genericcc::sort<T_container>(rs);

					register_method::genericcc::find_iterator<T_container>(rs);

					register_method::genericcc::contains<T_container>(rs);
					register_method::genericcc::count<T_container>(rs);
				}
				static void Register(asIScriptEngine* engine, const char* n_content) {
					common::RegistrationState rs(engine);
					Register(rs, n_content);
				}
			};



		};//namespace tempspec
	};//namespace container
};//namespace aatc
END_AS_NAMESPACE

#endif
