#ifndef _includedh_aatc_container_queue
#define _includedh_aatc_container_queue


#include "aatc_common.hpp"
#include "aatc_container_templated_shared.hpp"
#include "aatc_container_tempspec_shared_method.hpp"



BEGIN_AS_NAMESPACE
namespace aatc {
	namespace container {



		namespace templated {



			class queue : public shared::Containerbase <
				aatc_acit_queue<void*>,
				container::listing::CONTAINER::DEQUE,
				container::listing::tags_of_container::queue
			> {
			public:
				queue(asITypeInfo* typeinfo);
				queue(const queue& other);
				queue& operator=(const queue& other);
				queue& swap(queue& other);



				void push_back(void* value);
				void pop_back();

				void push_front(void* value);
				void pop_front();

				void* back();
				void* front();

				void* operator[](config::t::sizetype position);

				void sort(bool ascending = true);
				void sort(common::script_Funcpointer* funcptr, bool ascending = true);

				void erase(config::t::sizetype position);
				void erase(const Iterator& position);
				config::t::sizetype erase(config::t::sizetype range_begin, config::t::sizetype range_end);
				config::t::sizetype erase(const Iterator& range_begin, const Iterator& range_end);

				config::t::sizetype erase_value(void* value, bool all = false);

				Iterator find(void* value);
				void insert(const Iterator& position, void* value);

				bool contains(void* value);
				config::t::sizetype count(void* value);
			};



		};//namespace templated
		namespace tempspec {



			template<typename T_content> class queue : public shared::Containerbase <
				aatc_acit_queue<T_content>,
				T_content,
				container::listing::CONTAINER::DEQUE,
				container::listing::tags_of_container::queue
			> {
			public:
				typedef shared::Containerbase <
					aatc_acit_queue<T_content>,
					T_content,
					container::listing::CONTAINER::DEQUE,
					container::listing::tags_of_container::queue
				> Containerbase;
				typedef typename Containerbase::Iterator Iterator;



				queue() {}
				queue(const queue& other):
					Containerbase(other)
				{}
				queue& operator=(const queue& other) { Containerbase::operator=(other); return *this; }
				queue& swap(queue& other) { shared::method::swap(this, other); return *this; }



				void push_back(const T_content& value) { shared::method::native::push_back(this, value); }
				void pop_back() { shared::method::native::pop_back(this); }

				void push_front(const T_content& value) { shared::method::native::push_front(this, value); }
				void pop_front() { shared::method::native::pop_front(this); }

				T_content& back() { return shared::method::native::back(this); }
				T_content& front() { return shared::method::native::front(this); }

				void insert(config::t::sizetype position, const T_content& value) { shared::method::genericcc::insert_position_before_constant(this, position, value); }
				void insert(const Iterator& position, const T_content& value){ shared::method::native::insert_iterator(this, position, value); }

				void erase(config::t::sizetype position) { shared::method::genericcc::erase_position_constant(this, position); }
				void erase(const Iterator& position) { shared::method::native::erase_iterator(this, position); }
				config::t::sizetype erase(const Iterator& range_begin, const Iterator& range_end) { return shared::method::native::erase_iterator_range(this, range_begin, range_end); }
				config::t::sizetype erase(config::t::sizetype position_range_begin, config::t::sizetype position_range_end) { return shared::method::genericcc::erase_position_range_constant(this, position_range_begin, position_range_end); }

				config::t::sizetype erase_value(const T_content& value, bool all = false) { shared::method::genericcc::erase_value(this, value, all); }

				T_content& operator[](config::t::sizetype position) { return shared::method::native::operator_index_position(this,position); }

				Iterator find(const T_content& value) { return shared::method::genericcc::find_iterator(this, value); }

				void sort(bool ascending = true) { shared::method::genericcc::sort(this, ascending); }
				void sort(common::script_Funcpointer* funcptr, bool ascending = true) { shared::method::genericcc::sort_aatcfuncptr(this, funcptr, ascending); }

				bool contains(const T_content& value) { return shared::method::genericcc::contains(this, value); }
				config::t::sizetype count(const T_content& value) { return shared::method::genericcc::count(this, value); }



				static void Register(common::RegistrationState& rs, const char* n_content) {
					using namespace tempspec::shared;
					typedef queue T_container;

					register_containerbase<T_container>(rs, n_content);
					register_method::swap<T_container>(rs);



					register_method::native::push_back<T_container>(rs);
					register_method::native::pop_back<T_container>(rs);

					register_method::native::back<T_container>(rs);
					register_method::native::front<T_container>(rs);

					register_method::genericcc::insert_position_before_constant<T_container>(rs);
					register_method::native::insert_iterator<T_container>(rs);

					register_method::genericcc::erase_position_constant<T_container>(rs);
					register_method::native::erase_iterator<T_container>(rs);
					register_method::native::erase_iterator_range<T_container>(rs);
					register_method::genericcc::erase_position_range_constant<T_container>(rs);

					register_method::genericcc::erase_value<T_container>(rs);

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
