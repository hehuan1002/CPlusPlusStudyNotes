// mutex C++0X header
#pragma once
#ifndef _MUTEX_
#define _MUTEX_
#ifndef RC_INVOKED

 #ifdef _M_CEE
  #error <mutex> is not supported when compiling with /clr or /clr:pure.
 #endif /* _M_CEE */

#include <thr/xthread>
#include <chrono>
#include <functional>

 #pragma pack(push,_CRT_PACKING)
 #pragma warning(push,3)
 #pragma push_macro("new")
 #undef new
_STD_BEGIN
 #pragma warning(disable: 4800)

	// MUTUAL EXCLUSION
class _Mutex_base
	{	// base class for all mutex types
public:
	_Mutex_base(int _Flags = 0)
		{	// construct with _Flags
		_Mtx_initX(&_Mtx, _Flags | _Mtx_try);
		}

	~_Mutex_base() _NOEXCEPT
		{	// clean up
		_Mtx_destroy(&_Mtx);
		}

private:
	_Mutex_base(const _Mutex_base&);	// not defined
	_Mutex_base& operator=(const _Mutex_base&);	// not defined

public:
	void lock()
		{	// lock the mutex
		_Mtx_lockX(&_Mtx);
		}

	bool try_lock()
		{	// try to lock the mutex
		return (_Mtx_trylockX(&_Mtx) == _Thrd_success);
		}

	void unlock()
		{	// unlock the mutex
		_Mtx_unlockX(&_Mtx);
		}

	typedef void * native_handle_type;

	native_handle_type native_handle()
		{	// return Concurrency::critical_section * as void *
		return (_Mtx_getconcrtcs(&_Mtx));
		}

private:
	friend class _Timed_mutex_base;
	friend class condition_variable;
	_Mtx_t _Mtx;
	};

class _Timed_mutex_base
	: public _Mutex_base
	{	// base class for mutexes with timeouts
public:
	_Timed_mutex_base(int _Flags = 0)
		: _Mutex_base(_Flags | _Mtx_timed)
		{	// construct from _Flags
		}

private:
	_Timed_mutex_base(const _Timed_mutex_base&);	// not defined
	_Timed_mutex_base& operator=(const _Timed_mutex_base&);	// not defined
public:
	template<class _Rep,
		class _Period>
		bool try_lock_for(
			const chrono::duration<_Rep, _Period>& _Rel_time)
		{	// try to lock for duration
		stdext::threads::xtime _Tgt = _To_xtime(_Rel_time);
		return (try_lock_until(&_Tgt));
		}

	template<class _Clock, class _Duration>
		bool try_lock_until(
			const chrono::time_point<_Clock, _Duration>& _Abs_time)
		{	// try to lock until time point
		typename chrono::time_point<_Clock, _Duration>::duration _Rel_time =
			_Abs_time - _Clock::now();
		return (try_lock_for(_Rel_time));
		}

	bool try_lock_until(const xtime *_Abs_time)
		{	// try to lock the mutex with timeout
		return (_Mtx_timedlockX(&_Mtx, _Abs_time) == _Thrd_success);
		}
	};

class mutex
	: public _Mutex_base
	{	// class for mutual exclusion
public:
	mutex() _NOEXCEPT
		: _Mutex_base()
		{	// default construct
		}

private:
	mutex(const mutex&); // not defined
	mutex& operator=(const mutex&); // not defined
	};

class recursive_mutex
	: public _Mutex_base
	{	// class for recursive mutual exclusion
public:
	recursive_mutex()
		: _Mutex_base(_Mtx_recursive)
		{	// default construct
		}

private:
	recursive_mutex(const recursive_mutex&);	// not defined
	recursive_mutex& operator=(const recursive_mutex&); // not defined
	};

class timed_mutex
	: public _Timed_mutex_base
	{	// class for mutual exclusion with timeouts
public:
	timed_mutex()
		: _Timed_mutex_base()
		{	// default construct
		}

private:
	timed_mutex(const timed_mutex&);	// not defined
	timed_mutex& operator=(const timed_mutex&); // not defined
	};

class recursive_timed_mutex
	: public _Timed_mutex_base
	{	// class for recursive mutual exclusion with timeouts
public:
	recursive_timed_mutex()
		: _Timed_mutex_base(_Mtx_recursive)
		{	// default construct
		}

private:
	recursive_timed_mutex(const recursive_timed_mutex&);	// not defined
	recursive_timed_mutex& operator=(
		const recursive_timed_mutex&); // not defined
	};

	// LOCK PROPERTIES
struct adopt_lock_t
	{	// indicates adopt lock
	};

struct defer_lock_t
	{	// indicates defer lock
	};

struct try_to_lock_t
	{	// indicates try to lock
	};

extern _CRTIMP2_PURE const adopt_lock_t adopt_lock;
extern _CRTIMP2_PURE const defer_lock_t defer_lock;
extern _CRTIMP2_PURE const try_to_lock_t try_to_lock;

	// LOCKS
template<class _Mutex>
	class lock_guard
	{	// class with destructor that unlocks mutex
public:
	typedef _Mutex mutex_type;

	explicit lock_guard(_Mutex& _Mtx)
		: _MyMutex(_Mtx)
		{	// construct and lock
		_MyMutex.lock();
		}

	lock_guard(_Mutex& _Mtx, adopt_lock_t)
		: _MyMutex(_Mtx)
		{	// construct but don't lock
		}

	~lock_guard() _NOEXCEPT
		{	// unlock
		_MyMutex.unlock();
		}

private:
	lock_guard(const lock_guard&);  // not defined
	lock_guard& operator=(const lock_guard&);	// not defined

private:
	_Mutex& _MyMutex;
	};

template<class _Mutex>
	class unique_lock
	{	// whizzy class with destructor that unlocks mutex
public:
	typedef unique_lock<_Mutex> _Myt;
	typedef _Mutex mutex_type;

	// CONSTRUCT, ASSIGN, AND DESTROY
	unique_lock() _NOEXCEPT
		: _Pmtx(0), _Owns(false)
		{	// default construct
		}

	explicit unique_lock(_Mutex& _Mtx)
		: _Pmtx(&_Mtx), _Owns(false)
		{	// construct and lock
		_Pmtx->lock();
		_Owns = true;
		}

	unique_lock(_Mutex& _Mtx, adopt_lock_t)
		: _Pmtx(&_Mtx), _Owns(true)
		{	// construct and assume already locked
		}

	unique_lock(_Mutex& _Mtx, defer_lock_t) _NOEXCEPT
		: _Pmtx(&_Mtx), _Owns(false)
		{	// construct but don't lock
		}

	unique_lock(_Mutex& _Mtx, try_to_lock_t)
		: _Pmtx(&_Mtx), _Owns(_Pmtx->try_lock())
		{	// construct and try to lock
		}

	template<class _Rep, class _Period>
		unique_lock(_Mutex& _Mtx,
			const chrono::duration<_Rep, _Period>& _Rel_time)
		: _Pmtx(&_Mtx), _Owns(_Pmtx->try_lock_for(_Rel_time))
		{	// construct and lock with timeout
		}

	template<class _Clock, class _Duration>
		unique_lock(_Mutex& _Mtx,
			const chrono::time_point<_Clock, _Duration>& _Abs_time)
		: _Pmtx(&_Mtx), _Owns(_Pmtx->try_lock_until(_Abs_time))
		{	// construct and lock with timeout
		}

	unique_lock(_Mutex& _Mtx, const xtime *_Abs_time)
		: _Pmtx(&_Mtx), _Owns(false)
		{	// try to lock until _Abs_time
		_Owns = _Pmtx->try_lock_until(_Abs_time);
		}

	unique_lock(unique_lock&& _Other) _NOEXCEPT
		: _Pmtx(_Other._Pmtx), _Owns(_Other._Owns)
		{	// destructive copy
		_Other._Pmtx = 0;
		_Other._Owns = false;
		}

	unique_lock& operator=(unique_lock&& _Other) _NOEXCEPT
		{	// destructive copy
		if (this != &_Other)
			{	// different, move contents
			if (_Owns)
				_Pmtx->unlock();
			_Pmtx = _Other._Pmtx;
			_Owns = _Other._Owns;
			_Other._Pmtx = 0;
			_Other._Owns = false;
			}
		return (*this);
		}

	~unique_lock() _NOEXCEPT
		{	// clean up
		if (_Owns)
			_Pmtx->unlock();
		}

	unique_lock(const unique_lock&);	// not defined
	unique_lock& operator=(const unique_lock&);	// not defined

	// LOCK AND UNLOCK
	void lock()
		{	// lock the mutex
		_Pmtx->lock();
		_Owns = true;
		}

	bool try_lock() _NOEXCEPT
		{	// try to lock the mutex
		_Owns = _Pmtx->try_lock();
		return (_Owns);
		}

	template<class _Rep,
		class _Period>
		bool try_lock_for(const chrono::duration<_Rep, _Period>& _Rel_time)
		{	// try to lock mutex with timeout
		_Owns = _Pmtx->try_lock_for(_Rel_time);
		return (_Owns);
		}

	template<class _Clock,
		class _Duration>
		bool try_lock_until(
			const chrono::time_point<_Clock, _Duration>& _Abs_time)
		{	// try to lock mutex with timeout
		_Owns = _Pmtx->try_lock_until(_Abs_time);
		return (_Owns);
		}

	bool try_lock_until(const xtime *_Abs_time)
		{	// try to lock the mutex until _Abs_time
		_Owns = _Pmtx->try_lock_until(_Abs_time);
		return (_Owns);
		}

	void unlock()
		{	// unlock the mutex
		_Pmtx->unlock();
		_Owns = false;
		}

	// MUTATE
	void swap(unique_lock& _Other) _NOEXCEPT
		{	// swap with _Other
		_STD swap(_Pmtx, _Other._Pmtx);
		_STD swap(_Owns, _Other._Owns);
		}

	_Mutex *release() _NOEXCEPT
		{	// disconnect
		_Mutex *_Res = _Pmtx;
		_Pmtx = 0;
		_Owns = false;
		return (_Res);
		}

	// OBSERVE
	bool owns_lock() const _NOEXCEPT
		{	// return true if this object owns the lock
		return (_Owns);
		}

	_TYPEDEF_BOOL_TYPE;

	_OPERATOR_BOOL() const _NOEXCEPT
		{	// return true if this object owns the lock
		return (_Owns ? _CONVERTIBLE_TO_TRUE : 0);
		}

	_Mutex *mutex() const _NOEXCEPT
		{	// return pointer to managed mutex
		return (_Pmtx);
		}

private:
	_Mutex *_Pmtx;
	bool _Owns;
	};

	// SWAP
template<class _Mutex>
	void swap(unique_lock<_Mutex>& _Left,
		unique_lock<_Mutex>& _Right) _NOEXCEPT
	{	// swap _Left and _Right
	_Left.swap(_Right);
	}

	// MULTIPLE LOCKS
template<class _Lock0> inline
	int _Try_lock(_Lock0& _Lk0)
	{	// try to lock one mutex
	if (!_Lk0.try_lock())
		return (0);
	else
		return (-1);
	}

#define _TRY_LOCK_LOCK( \
	TEMPLATE_LIST, PADDING_LIST, LIST, COMMA, X1, X2, X3, X4) \
template<class _Lock0, \
	class _Lock1 COMMA LIST(_CLASS_TYPE)> inline \
	int _Try_lock(_Lock0& _Lk0, _Lock1& _Lk1 COMMA LIST(_TYPE_REFREF_ARG)) \
	{	/* try to lock n-1 mutexes */ \
	int _Res; \
	if (!_Lk0.try_lock()) \
		return (0); \
	_TRY_BEGIN \
		/* handle exceptions from tail lock */ \
		if ((_Res = try_lock(_Lk1 COMMA LIST(_FORWARD_ARG))) != -1) \
			{	/* tail lock failed */ \
			_Lk0.unlock(); \
			++_Res; \
			} \
	_CATCH_ALL \
		/* tail lock threw exception */ \
		_Lk0.unlock(); \
		throw; \
	_CATCH_END \
	return (_Res); \
	} \
template<class _Lock0 COMMA LIST(_CLASS_TYPE)> inline \
	int try_lock(_Lock0& _Lk0 COMMA LIST(_TYPE_REFREF_ARG)) \
	{	/* try to lock n-1 mutexes */ \
	return (_Try_lock(_Lk0 COMMA LIST(_FORWARD_ARG))); \
	} \
template<class _Lock0, \
	class _Lock1 COMMA LIST(_CLASS_TYPE)> inline \
	void lock(_Lock0& _Lk0, _Lock1& _Lk1 COMMA LIST(_TYPE_REFREF_ARG)) \
	{	/* lock N mutexes */ \
	int _Res = 0; \
	while (_Res != -1) \
		_Res = _Try_lock(_Lk0, _Lk1 COMMA LIST(_FORWARD_ARG)); \
	}

_VARIADIC_EXPAND_0X(_TRY_LOCK_LOCK, , , , )
#undef _TRY_LOCK_LOCK

	// CALL ONCE
class _Once_pad
	{	// base class for call once
public:
	virtual void _Call() const = 0;
	};

template<class _Target>
	class _Once_target
		: public _Once_pad
	{	// template class for call once
public:
	_Once_target(_Target& _Tgt)
		: _MyTgt(&_Tgt)
		{	// construct from target
		}

	void _Call() const
		{	// do it
		(*_MyTgt)();
		}

private:
	_Target *_MyTgt;
	};

_EXTERN_C
_CRTIMP2_PURE void __CLRCALL_PURE_OR_CDECL _Do_call(void *_Tgt);
_END_EXTERN_C

struct once_flag
	{	// class to hold data for call once
	once_flag() _NOEXCEPT
		: _Flag(_ONCE_FLAG_CPP_INIT)
		{	// default construct
		}

private:
	once_flag(const once_flag&);	// not defined
	once_flag& operator=(const once_flag&); // not defined
public:
	_Once_flag_cpp _Flag;
	};

template<class _Target> inline
	void _Call_it(once_flag& _Flag, _Target _Tgt)
	{	// create call data object and make protected call
	_Once_target<_Target> _Once_data(_Tgt);
	_Call_onceEx(&_Flag._Flag, _Do_call,
		(void *)(&_Once_data));
	}

#define _CALL_ONCE( \
	TEMPLATE_LIST, PADDING_LIST, LIST, COMMA, X1, X2, X3, X4) \
template<class _Fn COMMA LIST(_CLASS_TYPE)> inline \
	void call_once(once_flag& _Flag, _Fn&& _Fx COMMA LIST(_TYPE_REFREF_ARG)) \
	{	/* call _Fx(_Ax...) once */ \
	_Call_it(_Flag, \
		_STD bind(_Decay_copy(_STD forward<_Fn>(_Fx)) \
			COMMA LIST(_DECAY_COPY_FORWARD_ARG))); \
	}

_VARIADIC_EXPAND_0X(_CALL_ONCE, , , , )
#undef _CALL_ONCE
_STD_END

 #pragma pop_macro("new")
 #pragma warning(pop)
 #pragma pack(pop)
#endif /* RC_INVOKED */
#endif /* _MUTEX_ */

/*
 * Copyright (c) 1992-2012 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V6.00:0009 */
