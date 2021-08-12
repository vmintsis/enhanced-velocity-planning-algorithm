#ifndef _TSptr_H_
#define _TSptr_H_

class CLog;
extern CLog &dbg;

template<class T>
struct TRefCnt
{
	explicit TRefCnt() : _cnt(1) 
	{
//		dbg.fshow("TRefCnt(%p) NEW ref=%d\n", this, _cnt); 
	}
	~TRefCnt() { Assert(_cnt == 0); }

	size_t addref() 
	{ 
//		dbg.fshow("TRefCnt %s (%p) ADDREF ref=%d\n", typeid(this).name(), this, _cnt+1); 
		return ++_cnt; 
	}
	size_t release() 
	{ 
		if (_cnt > 0) --_cnt;
		if (_cnt == 0) 
		{ 
//			dbg.fshow("TRefCnt %s (%p) RELEASE(DEL) ref=%d\n", typeid(this).name(), this, _cnt); 
			delete this; 
		}
//		else
//			dbg.fshow("TRefCnt %s (%p) RELEASE ref=%d\n", typeid(this).name(), this, _cnt);
		return _cnt;
	}

	size_t _cnt;
	T _eei;
};

template<class T>
struct TSptr
{
	TSptr()  : _mref(0) { }
	TSptr(const TSptr<T> &p) : _mref(0) { *this = p; }
	~TSptr() {  if (_mref) _mref->release(); _mref = 0;}

	TSptr<T> &newObj() { if (_mref) _mref->release(); _mref = new TRefCnt<T>; return *this;}

	TSptr<T> &operator=(const TSptr<T> &p)
	{
		if (this != &p)
		{
			if (p._mref) p._mref->addref();
			if (_mref) _mref->release();
			_mref = p._mref;
		}
		return *this;
	}

	bool empty() { return _mref == 0; }

	operator T*() 
	{
		return !empty() ? &_mref->_eei : 0;
	}
	T *operator->() 
	{
		Assert(!empty());
		return &_mref->_eei;
	}

	TRefCnt<T> *_mref;
};

#endif


