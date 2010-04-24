////////////////////////////////////////////////////////////////////////////////
//						Fast Delegates, part 3:
//
//				Wrapper classes to ensure type safety
//
////////////////////////////////////////////////////////////////////////////////


// Once we have the member function conversion templates, it's easy to make the
// wrapper classes. So that they will work with as many compilers as possible, 
// the classes are of the form
//   delegate3<int, char *, double>
// They can cope with any combination of parameters. The max number of parameters
// allowed is 8, but it is trivial to increase this limit.
// Note that we need to treat const member functions seperately.
// All this class does is to enforce type safety, and invoke the delegate with
// the correct list of parameters.

// Because of the weird rule about the class of derived member function pointers,
// you sometimes need to apply a downcast to the 'this' pointer.
// This is the reason for the use of "implicit_cast<X*>(pthis)" in the code below. 
// If CDerivedClass is derived from CBaseClass, but doesn't override SimpleVirtualFunction,
// without this trick you'd need to write:
//		MyDelegate(static_cast<CBaseClass *>(&d), &CDerivedClass::SimpleVirtualFunction);
// but with the trick you can write
//		MyDelegate(&d, &CDerivedClass::SimpleVirtualFunction);

// RetType is the type the compiler uses in compiling the template. For VC6,
// it cannot be void. RetType is the real type which is returned from
// all of the functions. It can be void.

// Implicit conversion to "bool" is achieved using the safe_bool idiom,
// using member data pointers (MDP). This allows "if (dg)..." syntax
// Because some compilers (eg codeplay) don't have a unique value for a zero
// MDP, an extra padding member is added to the SafeBool struct.
// Some compilers (eg VC6) won't implicitly convert from 0 to an MDP, so
// in that case the static function constructor is not made explicit; this
// allows "if (dg==0) ..." to compile.

namespace detail
{
	template<class RetType>
	struct deleg_traits0
	{
		typedef RetType(*StaticFunctionPtr)();
		typedef RetType (detail::GenericClass::*GenericMemFn)();
		typedef detail::closure_ptr<GenericMemFn, StaticFunctionPtr> ClosureType;
	};

	template<class P1, class RetType>
	struct deleg_traits1
	{
		typedef RetType (*StaticFunctionPtr)(P1 p1);
		typedef RetType (detail::GenericClass::*GenericMemFn)(P1 p1);
		typedef detail::closure_ptr<GenericMemFn, StaticFunctionPtr> ClosureType;
	};

	template<class P1, class P2, class RetType>
	struct deleg_traits2
	{
		typedef RetType (*StaticFunctionPtr)(P1 p1, P2 p2);
		typedef RetType (detail::GenericClass::*GenericMemFn)(P1 p1, P2 p2);
		typedef detail::closure_ptr<GenericMemFn, StaticFunctionPtr> ClosureType;
	};

	template<class P1, class P2, class P3, class RetType>
	struct deleg_traits3
	{
		typedef RetType (*StaticFunctionPtr)(P1 p1, P2 p2, P3 p3);
		typedef RetType (detail::GenericClass::*GenericMemFn)(P1 p1, P2 p2, P3 p3);
		typedef detail::closure_ptr<GenericMemFn, StaticFunctionPtr> ClosureType;
	};

	template<class P1, class P2, class P3, class P4, class RetType>
	struct deleg_traits4
	{
		typedef RetType (*StaticFunctionPtr)(P1 p1, P2 p2, P3 p3, P4 p4);
		typedef RetType (detail::GenericClass::*GenericMemFn)(P1 p1, P2 p2, P3 p3, P4 p4);
		typedef detail::closure_ptr<GenericMemFn, StaticFunctionPtr> ClosureType;
	};

	template<class P1, class P2, class P3, class P4, class P5, class RetType>
	struct deleg_traits5
	{
		typedef RetType (*StaticFunctionPtr)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5);
		typedef RetType (detail::GenericClass::*GenericMemFn)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5);
		typedef detail::closure_ptr<GenericMemFn, StaticFunctionPtr> ClosureType;
	};

	//////////////////////////////////////////////////////////////////////////

	template<class Traits>
	class delegate_n
	{
	protected:
		typedef delegate_n<Traits> base;
		typedef Traits traits;
		typedef typename traits::StaticFunctionPtr StaticFunctionPtr;
		typedef typename traits::GenericMemFn GenericMemFn;
		typedef typename traits::ClosureType ClosureType;
		ClosureType m_Closure;

		delegate_n() { clear(); }
		delegate_n(const delegate_n &x) { m_Closure.CopyFrom(this, x.m_Closure); }
		void operator =(const delegate_n &x)  { m_Closure.CopyFrom(this, x.m_Closure); }
		bool operator ==(const delegate_n &x) const { return m_Closure.IsEqual(x.m_Closure);	}
		bool operator !=(const delegate_n &x) const { return !m_Closure.IsEqual(x.m_Closure); }
		bool operator <(const delegate_n &x) const { return m_Closure.IsLess(x.m_Closure);	}
		bool operator >(const delegate_n &x) const { return x.m_Closure.IsLess(m_Closure);	}

	private:
		// Implicit conversion to "bool" using the safe_bool idiom
		typedef struct SafeBoolStruct 
		{
			int a_data_pointer_to_this_is_0_on_buggy_compilers;
			StaticFunctionPtr m_nonzero;
		} UselessTypedef;
		typedef StaticFunctionPtr SafeBoolStruct::*unspecified_bool_type;
	public:
		operator unspecified_bool_type() const { return empty()? 0: &SafeBoolStruct::m_nonzero; }
		// necessary to allow ==0 to work despite the safe_bool idiom
		inline bool operator==(StaticFunctionPtr funcptr) { return m_Closure.IsEqualToStaticFuncPtr(funcptr); }
		inline bool operator!=(StaticFunctionPtr funcptr) { return !m_Closure.IsEqualToStaticFuncPtr(funcptr); }
		inline bool operator !() const	{ return !m_Closure; }
		inline bool empty() const { return !m_Closure; }
		void clear() { m_Closure.clear();}
		// Conversion to and from the function_data storage class
		const function_data & Getfunction_data() { return m_Closure; }
		void Setfunction_data(const function_data &any) { m_Closure.CopyFrom(this, any); }
	};
}

//////////////////////////////////////////////////////////////////////////

//N=0
template<class RetType = void>
class delegate0 : public detail::delegate_n< detail::deleg_traits0<RetType> > 
{
public:
	// Typedefs to aid generic programming
	typedef delegate0 type;

	// Construction and comparison functions
	delegate0() { }
	delegate0(const delegate0 &x) : base(x) { }
	void operator = (const delegate0 &x) { base::operator=(x); }
	// Binding to non-const member functions
	template < class X, class Y >
	delegate0(Y *pthis, RetType (X::* function_to_bind)() ) {
		m_Closure.bindmemfunc(detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, RetType (X::* function_to_bind)()) {
		m_Closure.bindmemfunc(detail::implicit_cast<X*>(pthis), function_to_bind);	}
	// Binding to const member functions.
	template < class X, class Y >
	delegate0(const Y *pthis, RetType (X::* function_to_bind)() const) {
		m_Closure.bindconstmemfunc(detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, RetType (X::* function_to_bind)() const) {
		m_Closure.bindconstmemfunc(detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	// Static functions. We convert them into a member function call.
	// This constructor also provides implicit conversion
	delegate0(RetType (*function_to_bind)() ) {
		bind(function_to_bind);	}
	// for efficiency, prevent creation of a temporary
	void operator = (RetType (*function_to_bind)() ) {
		bind(function_to_bind);	}
	inline void bind(RetType (*function_to_bind)()) {
		m_Closure.bindstaticfunc(this, &delegate0::InvokeStaticFunction, 
			function_to_bind); }
	// Invoke the delegate
	RetType operator() () const { return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(); }

private:	// Invoker for static functions
	RetType InvokeStaticFunction() const {
		return (*(m_Closure.GetStaticFunction()))(); }
};

//N=1
template<class P1, class RetType = void>
class delegate1 : public detail::delegate_n< detail::deleg_traits1<P1, RetType> > 
{
public:
	typedef delegate1 type;
	delegate1() { }
	delegate1(const delegate1 &x) : base(x) { }

	template < class X, class Y >
	delegate1(Y *pthis, RetType (X::* function_to_bind)(P1 p1) ) {
		m_Closure.bindmemfunc(detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, RetType (X::* function_to_bind)(P1 p1)) {
		m_Closure.bindmemfunc(detail::implicit_cast<X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	delegate1(const Y *pthis, RetType (X::* function_to_bind)(P1 p1) const) {
		m_Closure.bindconstmemfunc(detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, RetType (X::* function_to_bind)(P1 p1) const) {
		m_Closure.bindconstmemfunc(detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	delegate1(RetType (*function_to_bind)(P1 p1) ) {
		bind(function_to_bind);	}
	void operator = (RetType (*function_to_bind)(P1 p1) ) {
		bind(function_to_bind);	}
	inline void bind(RetType (*function_to_bind)(P1 p1)) {
		m_Closure.bindstaticfunc(this, &delegate1::InvokeStaticFunction, 
			function_to_bind); }
	RetType operator() (P1 p1) const { return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1); }

private:	// Invoker for static functions
	RetType InvokeStaticFunction(P1 p1) const {
		return (*(m_Closure.GetStaticFunction()))(p1); }
};

//N=2
template<class P1, class P2, class RetType = void>
class delegate2 : public detail::delegate_n< detail::deleg_traits2<P1, P2, RetType> >
{
public:
	typedef delegate2 type;

	delegate2() { }
	delegate2(const delegate2 &x) : base(x) { }
	void operator = (const delegate2 &x)  { base::operator=(x); }

	template < class X, class Y >
	delegate2(Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2) ) {
		m_Closure.bindmemfunc(detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2)) {
		m_Closure.bindmemfunc(detail::implicit_cast<X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	delegate2(const Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2) const) {
		m_Closure.bindconstmemfunc(detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2) const) {
		m_Closure.bindconstmemfunc(detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	delegate2(RetType (*function_to_bind)(P1 p1, P2 p2) ) {
		bind(function_to_bind);	}
	void operator = (RetType (*function_to_bind)(P1 p1, P2 p2) ) {
		bind(function_to_bind);	}
	inline void bind(RetType (*function_to_bind)(P1 p1, P2 p2)) {
		m_Closure.bindstaticfunc(this, &delegate2::InvokeStaticFunction, 
			function_to_bind); }
	RetType operator() (P1 p1, P2 p2) const { return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1, p2); }

private:
	RetType InvokeStaticFunction(P1 p1, P2 p2) const {
		return (*(m_Closure.GetStaticFunction()))(p1, p2); }
};

//N=3
template<class P1, class P2, class P3, class RetType = void>
class delegate3 : public detail::delegate_n< detail::deleg_traits3<P1, P2, P3, RetType> >
{
public:
	typedef delegate3 type;

	delegate3() { }
	delegate3(const delegate3 &x) : base(x) { }
	void operator = (const delegate3 &x)  { base::operator=(x); }
	template < class X, class Y >
	delegate3(Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2, P3 p3) ) {
		m_Closure.bindmemfunc(detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2, P3 p3)) {
		m_Closure.bindmemfunc(detail::implicit_cast<X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	delegate3(const Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2, P3 p3) const) {
		m_Closure.bindconstmemfunc(detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2, P3 p3) const) {
		m_Closure.bindconstmemfunc(detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	delegate3(RetType (*function_to_bind)(P1 p1, P2 p2, P3 p3) ) {
		bind(function_to_bind);	}
	void operator = (RetType (*function_to_bind)(P1 p1, P2 p2, P3 p3) ) {
		bind(function_to_bind);	}
	inline void bind(RetType (*function_to_bind)(P1 p1, P2 p2, P3 p3)) {
		m_Closure.bindstaticfunc(this, &delegate3::InvokeStaticFunction, 
			function_to_bind); }
	RetType operator() (P1 p1, P2 p2, P3 p3) const {
		return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1, p2, p3); }

private:
	RetType InvokeStaticFunction(P1 p1, P2 p2, P3 p3) const {
		return (*(m_Closure.GetStaticFunction()))(p1, p2, p3); }
};

//N=4
template<class P1, class P2, class P3, class P4, class RetType = void>
class delegate4 : public detail::delegate_n< detail::deleg_traits4<P1, P2, P3, P4, RetType> >
{
public:
	typedef delegate4 type;

	delegate4() { }
	delegate4(const delegate4 &x) : base(x) { }
	void operator = (const delegate4 &x)  { base::operator=(x); }
	template < class X, class Y >
	delegate4(Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4) ) {
		m_Closure.bindmemfunc(detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4)) {
		m_Closure.bindmemfunc(detail::implicit_cast<X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	delegate4(const Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4) const) {
		m_Closure.bindconstmemfunc(detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4) const) {
		m_Closure.bindconstmemfunc(detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	delegate4(RetType (*function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4) ) {
		bind(function_to_bind);	}
	void operator = (RetType (*function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4) ) {
		bind(function_to_bind);	}
	inline void bind(RetType (*function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4)) {
		m_Closure.bindstaticfunc(this, &delegate4::InvokeStaticFunction, 
			function_to_bind); }
	RetType operator() (P1 p1, P2 p2, P3 p3, P4 p4) const {
		return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1, p2, p3, p4); }

private:
	RetType InvokeStaticFunction(P1 p1, P2 p2, P3 p3, P4 p4) const {
		return (*(m_Closure.GetStaticFunction()))(p1, p2, p3, p4); }
};

//N=5
template<class P1, class P2, class P3, class P4, class P5, class RetType = void>
class delegate5 : public detail::delegate_n< detail::deleg_traits5<P1, P2, P3, P4, P5, RetType> >
{
public:
	typedef delegate5 type;

	delegate5() { }
	delegate5(const delegate5 &x) : base(x) { }
	void operator = (const delegate5 &x)  { base::operator=(x); }
	template < class X, class Y >
	delegate5(Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) ) {
		m_Closure.bindmemfunc(detail::implicit_cast<X*>(pthis), function_to_bind); }
	template < class X, class Y >
	inline void bind(Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)) {
		m_Closure.bindmemfunc(detail::implicit_cast<X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	delegate5(const Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const) {
		m_Closure.bindconstmemfunc(detail::implicit_cast<const X*>(pthis), function_to_bind);	}
	template < class X, class Y >
	inline void bind(const Y *pthis, RetType (X::* function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const) {
		m_Closure.bindconstmemfunc(detail::implicit_cast<const X *>(pthis), function_to_bind);	}
	delegate5(RetType (*function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) ) {
		bind(function_to_bind);	}
	void operator = (RetType (*function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) ) {
		bind(function_to_bind);	}
	inline void bind(RetType (*function_to_bind)(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)) {
		m_Closure.bindstaticfunc(this, &delegate5::InvokeStaticFunction, 
			function_to_bind); }
	RetType operator() (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const {
		return (m_Closure.GetClosureThis()->*(m_Closure.GetClosureMemPtr()))(p1, p2, p3, p4, p5); }

private:
	RetType InvokeStaticFunction(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const {
		return (*(m_Closure.GetStaticFunction()))(p1, p2, p3, p4, p5); }
};


//////////////////////////////////////////////////////////////////////////
// Dynamic delegates
//////////////////////////////////////////////////////////////////////////

// Declares pure virtual function for dynamic invokation of function
class delegate_dynamic_base
{
public:
	virtual void invoke(void ** args, void * ret) const = 0;
};

//////////////////////////////////////////////////////////////////////////

#define UP_ARG(T, N)		(*(T*)args[N])
#define UP_RET()			(*(R*)rt)

template<class Deleg, class R>
struct rt_invoker0 {
	inline static void invoke(void ** args, void * rt, const Deleg& dlg) { if(rt) UP_RET() = dlg(); else dlg(); }
};
template<class Deleg>
struct rt_invoker0<Deleg, void> {
	inline static void invoke(void ** args, void * rt, const Deleg& dlg) { dlg(); }
};

template<class Deleg, class P1, class R>
struct rt_invoker1 {
	inline static void invoke(void ** args, void * rt, const Deleg& dlg) { if(rt) UP_RET() = dlg(UP_ARG(P1, 0)); else dlg(UP_ARG(P1, 0)); }
};
template<class Deleg, class P1>
struct rt_invoker1<Deleg, P1, void> {
	inline static void invoke(void ** args, void * rt, const Deleg& dlg) { dlg(UP_ARG(P1, 0)); }
};

template<class Deleg, class P1, class P2, class R>
struct rt_invoker2 {
	inline static void invoke(void ** args, void * rt, const Deleg& dlg) { if(rt) UP_RET() = dlg(UP_ARG(P1, 0), UP_ARG(P2, 1)); else dlg(UP_ARG(P1, 0), UP_ARG(P2, 1)); }
};
template<class Deleg, class P1, class P2>
struct rt_invoker2<Deleg, P1, P2, void> {
	inline static void invoke(void ** args, void * rt, const Deleg& dlg) { dlg(UP_ARG(P1, 0), UP_ARG(P2, 1)); }
};

template<class Deleg, class P1, class P2, class P3, class R>
struct rt_invoker3 {
	inline static void invoke(void ** args, void * rt, const Deleg& dlg) { if(rt) UP_RET() = dlg(UP_ARG(P1, 0), UP_ARG(P2, 1), UP_ARG(P3, 2)); else dlg(UP_ARG(P1, 0), UP_ARG(P2, 1), UP_ARG(P3, 2)); }
};
template<class Deleg, class P1, class P2, class P3>
struct rt_invoker3<Deleg, P1, P2, P3, void> {
	inline static void invoke(void ** args, void * rt, const Deleg& dlg) { dlg(UP_ARG(P1, 0), UP_ARG(P2, 1), UP_ARG(P3, 2)); }
};

template<class Deleg, class P1, class P2, class P3, class P4, class R>
struct rt_invoker4 {
	inline static void invoke(void ** args, void * rt, const Deleg& dlg) { if(rt) UP_RET() = dlg(UP_ARG(P1, 0), UP_ARG(P2, 1), UP_ARG(P3, 2), UP_ARG(P4, 3)); else dlg(UP_ARG(P1, 0), UP_ARG(P2, 1), UP_ARG(P3, 2), UP_ARG(P4, 3)); }
};
template<class Deleg, class P1, class P2, class P3, class P4>
struct rt_invoker4<Deleg, P1, P2, P3, P4, void> {
	inline static void invoke(void ** args, void * rt, const Deleg& dlg) { dlg(UP_ARG(P1, 0), UP_ARG(P2, 1), UP_ARG(P3, 2), UP_ARG(P4, 3)); }
};

template<class Deleg, class P1, class P2, class P3, class P4, class P5, class R>
struct rt_invoker5 {
	inline static void invoke(void ** args, void * rt, const Deleg& dlg) { if(rt) UP_RET() = dlg(UP_ARG(P1, 0), UP_ARG(P2, 1), UP_ARG(P3, 2), UP_ARG(P4, 3), UP_ARG(P5, 4)); else dlg(UP_ARG(P1, 0), UP_ARG(P2, 1), UP_ARG(P3, 2), UP_ARG(P4, 3), UP_ARG(P5, 4)); }
};
template<class Deleg, class P1, class P2, class P3, class P4, class P5>
struct rt_invoker5<Deleg, P1, P2, P3, P4, P5, void> {
	inline static void invoke(void ** args, void * rt, const Deleg& dlg) { dlg(UP_ARG(P1, 0), UP_ARG(P2, 1), UP_ARG(P3, 2), UP_ARG(P4, 3), UP_ARG(P5, 4)); }
};

#undef UP_ARG
#undef UP_RET

//////////////////////////////////////////////////////////////////////////

//N=0
template<typename R = void>
class delegate_dynamic0 : public delegate0 < R >, public delegate_dynamic_base
{
public:
	typedef delegate0 < R > base_type;
	typedef delegate_dynamic0 this_type;

	delegate_dynamic0() : base_type() { }

	template < class X, class Y >
	delegate_dynamic0(Y * pthis, R (X::* function_to_bind)(  ))
		: base_type(pthis, function_to_bind)  
	{ }

	template < class X, class Y >
	delegate_dynamic0(const Y *pthis, R (X::* function_to_bind)(  ) const)
		: base_type(pthis, function_to_bind)
	{ }

	delegate_dynamic0(R (*function_to_bind)(  ))
		: base_type(function_to_bind) 
	{ }

	void operator = (const base_type &x)  { *static_cast<base_type*>(this) = x; }

	virtual void invoke(void ** args, void * ret) const
	{
		rt_invoker0<this_type, R>::invoke(args, ret, *this);
	}
};

//N=1
template<class P1, typename R = void>
class delegate_dynamic1 : public delegate1 < P1, R >, public delegate_dynamic_base
{
public:
	typedef delegate1 < P1, R > base_type;
	typedef delegate_dynamic1 this_type;

	delegate_dynamic1() : base_type() { }

	template < class X, class Y >
	delegate_dynamic1(Y * pthis, R (X::* function_to_bind)( P1 p1 ))
		: base_type(pthis, function_to_bind)  
	{ }

	template < class X, class Y >
	delegate_dynamic1(const Y *pthis, R (X::* function_to_bind)( P1 p1 ) const)
		: base_type(pthis, function_to_bind)
	{ }

	delegate_dynamic1(R (*function_to_bind)( P1 p1 ))
		: base_type(function_to_bind)  
	{ }

	void operator = (const base_type &x)  { *static_cast<base_type*>(this) = x; }

	virtual void invoke(void ** args, void * ret) const
	{
		rt_invoker1<this_type, P1, R>::invoke(args, ret, *this);
	}
};

//N=2
template<class P1, class P2, typename R = void>
class delegate_dynamic2 : public delegate2 < P1, P2, R >, public delegate_dynamic_base
{
public:
	typedef delegate2 < P1, P2, R > base_type;
	typedef delegate_dynamic2 this_type;

	delegate_dynamic2() : base_type() { }

	template < class X, class Y >
	delegate_dynamic2(Y * pthis, R (X::* function_to_bind)( P1 p1, P2 p2 ))
		: base_type(pthis, function_to_bind) 
	{ }

	template < class X, class Y >
	delegate_dynamic2(const Y *pthis, R (X::* function_to_bind)( P1 p1, P2 p2 ) const)
		: base_type(pthis, function_to_bind)
	{ }

	delegate_dynamic2(R (*function_to_bind)( P1 p1, P2 p2 ))
		: base_type(function_to_bind)  
	{ }

	void operator = (const base_type &x)  { *static_cast<base_type*>(this) = x; }

	virtual void invoke(void ** args, void * ret) const
	{
		rt_invoker2<this_type, P1, P2, R>::invoke(args, ret, *this);
	}
};

//N=3
template<class P1, class P2, class P3, typename R = void>
class delegate_dynamic3 : public delegate3 < P1, P2, P3, R >, public delegate_dynamic_base
{
public:
	typedef delegate3 < P1, P2, P3, R > base_type;
	typedef delegate_dynamic3 this_type;

	delegate_dynamic3() : base_type() { }

	template < class X, class Y >
	delegate_dynamic3(Y * pthis, R (X::* function_to_bind)( P1 p1, P2 p2, P3 p3 ))
		: base_type(pthis, function_to_bind)  
	{ }

	template < class X, class Y >
	delegate_dynamic3(const Y *pthis, R (X::* function_to_bind)( P1 p1, P2 p2, P3 p3 ) const)
		: base_type(pthis, function_to_bind)
	{ }

	delegate_dynamic3(R (*function_to_bind)( P1 p1, P2 p2, P3 p3 ))
		: base_type(function_to_bind)  
	{ }

	void operator = (const base_type &x)  { *static_cast<base_type*>(this) = x; }

	virtual void invoke(void ** args, void * ret) const
	{
		rt_invoker3<this_type, P1, P2, P3, R>::invoke(args, ret, *this);
	}
};

//N=4
template<class P1, class P2, class P3, class P4, typename R = void>
class delegate_dynamic4 : public delegate4 < P1, P2, P3, P4, R >, public delegate_dynamic_base
{
public:
	typedef delegate4 < P1, P2, P3, P4, R > base_type;
	typedef delegate_dynamic4 this_type;

	delegate_dynamic4() : base_type() { }

	template < class X, class Y >
	delegate_dynamic4(Y * pthis, R (X::* function_to_bind)( P1 p1, P2 p2, P3 p3, P4 p4 ))
		: base_type(pthis, function_to_bind)  
	{ }

	template < class X, class Y >
	delegate_dynamic4(const Y *pthis, R (X::* function_to_bind)( P1 p1, P2 p2, P3 p3, P4 p4 ) const)
		: base_type(pthis, function_to_bind)
	{ }

	delegate_dynamic4(R (*function_to_bind)( P1 p1, P2 p2, P3 p3, P4 p4 ))
		: base_type(function_to_bind)  
	{ }

	void operator = (const base_type &x)  { *static_cast<base_type*>(this) = x; }

	virtual void invoke(void ** args, void * ret) const
	{
		rt_invoker4<this_type, P1, P2, P3, P4, R>::invoke(args, ret, *this);
	}
};

//N=5
template<class P1, class P2, class P3, class P4, class P5, typename R = void>
class delegate_dynamic5 : public delegate5 < P1, P2, P3, P4, P5, R >, public delegate_dynamic_base
{
public:
	typedef delegate5 < P1, P2, P3, P4, P5, R > base_type;
	typedef delegate_dynamic5 this_type;

	delegate_dynamic5() : base_type() { }

	template < class X, class Y >
	delegate_dynamic5(Y * pthis, R (X::* function_to_bind)( P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 ))
		: base_type(pthis, function_to_bind)  
	{ }

	template < class X, class Y >
	delegate_dynamic5(const Y *pthis, R (X::* function_to_bind)( P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 ) const)
		: base_type(pthis, function_to_bind)
	{ }

	delegate_dynamic5(R (*function_to_bind)( P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 ))
		: base_type(function_to_bind)  
	{ }

	void operator = (const base_type &x)  { *static_cast<base_type*>(this) = x; }

	virtual void invoke(void ** args, void * ret) const
	{
		rt_invoker5<this_type, P1, P2, P3, P4, P5, R>::invoke(args, ret, *this);
	}
};

//////////////////////////////////////////////////////////////////////////
