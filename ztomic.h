#ifndef __ZATOMIC_HPP__
#define __ZATOMIC_HPP__

#include <stdint.h>

namespace atomic_impl
{
	typedef enum memory_order
	{
		memory_order_relaxed,
		memory_order_consume,
		memory_order_acquire,
		memory_order_release,
		memory_order_acq_rel,
		memory_order_seq_cst
	} memory_order;

#if defined( _MSC_VER )
	namespace detail
	{
		template<int param> struct msvc_fetch_add { };
		template<> struct msvc_fetch_add<4>
		{
			template<typename T, typename V>
			static  T call(T* storage, V v)
			{
				return _InterlockedAdd((volatile long*)storage, (long)v);
			}
		};


		template<> struct msvc_fetch_add<8>
		{
			template<typename T, typename V>
			static  T call(T* storage, V v)
			{
				return InterlockedAdd64((volatile LONGLONG*)storage, (LONGLONG)v);
			}
		};

		template<int param> struct msvc_cas { };
		template<> struct msvc_cas<4>
		{
			template<typename T, typename V>
			static  bool call(T* storage, T* exp, V v)
			{
				unsigned long rc=InterlockedCompareExchange((volatile unsigned long *)storage, v, *exp);
				
				if (rc == *exp)
					return true;

				*exp = rc;
				return false;
			}
		};


		template<> struct msvc_cas<8>
		{
			template<typename T, typename V>
			static  bool call(T* storage, T* exp, V v)
			{
				__int64 rc = InterlockedCompareExchange64((__int64 volatile *)storage, v, *exp);

				if (rc == *exp)
					return true;

				*exp = rc;
				return false;
			}
		};
		
	}

    template<typename T,typename V>
    static  T fetch_add(T* storage, V v)
    {
		return atomic_impl::detail::msvc_fetch_add<sizeof(T)>::call(storage,v);
    }

	template<typename T, typename V>
	static  bool cas(T* storage, T* exp, V v)
	{
		return atomic_impl::detail::msvc_cas<sizeof(T)>::call(storage, exp, v);
	}

#elif defined( __IBMCPP__ ) && defined( __powerpc )

#include<stdint.h>

extern "builtin" void __lwsync(void);
extern "builtin" void __isync(void);
extern "builtin" int __fetch_and_add(volatile int32_t* addr, int val);
extern "builtin" int64_t __fetch_and_addlp(volatile int64_t* addr, int64_t val);

struct local_sync
{
	local_sync()
	{
		__lwsync();
	}

	~local_sync()
	{
		__isync();
	}
};

template<int param> struct xlc_fetch_add { };
template<> struct xlc_fetch_add<4>
{
	template<typename T,typename V>
		static  T call(T* storage, V v)
		{
			local_sync _1;
			return __fetch_and_add (storage, v);
		}
};
template<> struct xlc_fetch_add<8>
{
	template<typename T,typename V>
		static  T call(T* storage, V v)
		{
			local_sync _1;
			return __fetch_and_addlp (storage, v);
		}
};

    template<typename T,typename V>
    static  T fetch_add(T* storage, V v)
    {
		return atomic_impl::xlc_fetch_add<sizeof(T)>::call(storage,v);
    }

#elif defined( __GNUC__ )
    template<typename T,typename V>
    static  void store(T* storage,V v)
    {
        __atomic_store_n(storage,v,memory_order_release);
    }

    template<typename T>
    static  T load(T* storage)
    {
		return __atomic_load_n(storage, memory_order_acquire);
    }

    template<typename T,typename V>
    static  T fetch_add(T* storage, V v)
    {
        return __sync_fetch_and_add (storage, v);
    }

    template<typename T,typename V>
    static  bool cas(T* storage, T* exp, V v)
	{
		T ov=__sync_val_compare_and_swap(storage, *exp, v);
		if(ov==*exp)
			return true;

		*exp=ov;
		return false;
	}
#else
#error No ztomic operations implemented for this platform, sorry!
#endif

template<typename T> struct type_map {};
template<> struct type_map<uint32_t> { typedef int32_t type; };
template<> struct type_map<int32_t>  { typedef int32_t type; };
template<> struct type_map<uint64_t> { typedef int64_t type;};
template<> struct type_map<int64_t>  { typedef int64_t type;};
}

struct ztomic
{
	template<typename V>
	static typename atomic_impl::type_map<V>::type cast(V v)
	{
		return static_cast<typename atomic_impl::type_map<V>::type>(v);
	}

    template<typename T,typename V>
    static  T fetch_add(T *storage, V v)
    {
        return atomic_impl::fetch_add (storage, cast(v));
    }

    template<typename T,typename V>
    static  T fetch_sub(T* storage, V v)
    {
        return ztomic::fetch_add(storage,-v);
    }

    template<typename T,typename V>
    static  T add_fetch(T* storage, V v)
    {
        return ztomic::fetch_add(storage,v)+cast(v);
    }

    template<typename T,typename V>
    static  T sub_fetch(T* storage, V v)
    {
        return ztomic::fetch_add(storage,-v)-cast(v);
    }

    template<typename T,typename V>
    static  void store(T* storage,V v)
    {
        return atomic_impl::store(storage,v);
    }

    template<typename T>
    static  T load(T* storage)
    {
        return atomic_impl::load(storage);
    }

    template<typename T,typename V>
    static  bool cas(T* storage, T* exp, V v)
    {
        return atomic_impl::cas(storage,exp,v);
    }
};

#endif
