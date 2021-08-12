#ifndef _VehComplianceProfileXForm_H_
#define _VehComplianceProfileXForm_H_

namespace eei
{
	enum XF_Type
	{
		XF_Invalid,

		XF_ACCELERATE,
		XF_DECELERATE,
		XF_MAINTAIN,

		XF_Last
	};
}

namespace std
{
	inline string to_string(const eei::XF_Type t)
	{
		switch (t)
		{
		case eei::XF_Type::XF_Invalid: return "invalid";
		case eei::XF_Type::XF_ACCELERATE: return "accelerate";
		case eei::XF_Type::XF_DECELERATE: return "decelerate";
		case eei::XF_Type::XF_MAINTAIN: return "maintain";
		default: break;
		}
		return "<unknown>";
	}
}

namespace eei 
{

	/**
	 *  Interface describing a transform of a vehicle profile speed vector
	 */
	class IVehComplianceProfileXForm
	{
	public:
		typedef std::map<std::string, double> ParamsMap;
		typedef std::vector<double> DoubleVec;
		virtual ~IVehComplianceProfileXForm() = default;
		virtual void apply(DoubleVec &v) = 0;
		virtual void setTitle(const char *title) = 0;
		virtual const char *title() = 0;
		virtual IVehComplianceProfileXForm *owner() = 0;
		virtual void setOwner(IVehComplianceProfileXForm *owner) = 0;
		virtual IVehComplianceProfileXForm &set(const char *paramName, double v) = 0;
		virtual double get(const char *paramName) = 0;

		virtual std::string report() = 0;
		virtual std::string getReports() = 0;
		virtual void setParent(EEIObject *obj) = 0;
		virtual EEIObject *parent() = 0;
	};

	class BaseVehComplianceProfileXForm : public IVehComplianceProfileXForm
	{
	public:
		BaseVehComplianceProfileXForm() : _owner(NULL), _title("NO TITLE"), m_pParent(NULL) { }
		virtual void setTitle(const char *title) { REQUIRE(title != NULL); _title = title; }
		virtual const char *title() { return _title.c_str(); }
		virtual IVehComplianceProfileXForm *owner() { return _owner; }
		virtual void setOwner(IVehComplianceProfileXForm *owner) { _owner = owner; }
		virtual void setParent(EEIObject *obj) { m_pParent = obj; }
		virtual EEIObject *parent() { return m_pParent; }

		virtual IVehComplianceProfileXForm &set(const char *paramName, double v) { REQUIRE("***IVehComplianceprofileXForm: set() call not allowed at this level" == NULL); return *this; }
		virtual double get(const char *paramName) { REQUIRE("***IVehComplianceprofileXForm: get() call not allowed at this level" == NULL);  return 0.0; }

		virtual std::string report() { return "<BASE LEVEL>"; }
		virtual std::string getReports()
		{
			std::string r = report();
			if (owner())
				r = owner()->getReports() + " ==> " + r;
			return r;
		}
	private:
		IVehComplianceProfileXForm *_owner;
		std::string _title;
	protected:
		EEIObject *m_pParent;
	};

	/**
	 *
	 */
	struct _Range
	{
		_Range() : _r0(0), _r1(0) { }
		_Range(double r0, double r1) : _r0(r0), _r1(r1)
		{
			if (r0 > r1) std::swap(_r0, _r1);
		}
		double _r0, _r1;
	};

	struct OPX_ValuesRange : public _Range
	{
		OPX_ValuesRange() { }
		OPX_ValuesRange(double r0, double r1) : _Range(r0, r1) { }
	};
	struct OPX_CriteriaRange : public _Range
	{
		OPX_CriteriaRange() { }
		OPX_CriteriaRange(double r0, double r1) : _Range(r0, r1) { }
	};

	/**
	*	Simple uniform speed profile offset by a random factor selected from the given interval
	*/
	class offsetProfileXform : public BaseVehComplianceProfileXForm
	{
	public:
		offsetProfileXform(const char *title, EEIObject *parent, const OPX_CriteriaRange &cr, const OPX_ValuesRange &vr, IVehComplianceProfileXForm *owner = NULL)
		{
			_values = vr;
			_criteria = cr;
			setTitle(title);
			setOwner(owner);
			setParent(parent);
		}
		
		virtual std::string report() 
		{ 
			return title() + std::string(": CR(") + std::to_string(_criteria._r0) + "," + std::to_string(_criteria._r1) + ") VAL(" + std::to_string(_values._r0) + "," + std::to_string(_values._r1)+")";
		}

		virtual void apply(DoubleVec &v)
		{
			double size = get("val1") - get("val0");	// values
			double f = ((double)rand()) / RAND_MAX;
			double offset = get("val1") - f * size;
			for (DoubleVec::iterator it = v.begin(); it != v.end(); it++)
				*it += offset;

			std::string rep = getReports() + " VALUE=" + std::to_string(offset);
			pflogMsg("%s\n", rep.c_str());
		}

		//	"crt0", "crt1" : _criteria
		//	"val0", "val1" : _values
		virtual IVehComplianceProfileXForm &set(const char *paramName, double v)
		{ 
			REQUIRE(paramName != NULL); 
			if (strcmp(paramName, "crt0") == 0) _criteria._r0 = v;
			else
			if (strcmp(paramName, "crt1") == 0) _criteria._r1 = v;
			else
			if (strcmp(paramName, "val0") == 0) _values._r0 = v;
			else
			if (strcmp(paramName, "val1") == 0) _values._r1 = v;
			else
			{
				pflogErr("***ERROR: set() param %s not recognised\n", paramName);
				pslogErr("***ERROR: set() param %s not recognised\n", paramName);
			}
			return *this;
		}

		//	"crt0", "crt1" : _criteria
		//	"val0", "val1" : _values
		virtual double get(const char *paramName)
		{ 
			REQUIRE(paramName != NULL);
			if (strcmp(paramName, "crt0") == 0) return _criteria._r0;
			else
			if (strcmp(paramName, "crt1") == 0) return _criteria._r1;
			else
			if (strcmp(paramName, "val0") == 0) return _values._r0;
			else
			if (strcmp(paramName, "val1") == 0) return _values._r1;
			pflogErr("***ERROR: %s::get() param %s not recognised\n", title(), paramName);
			pslogErr("***ERROR: %s::get() param %s not recognised\n", title(), paramName);
			return 0.0;
		}

	private:
		OPX_ValuesRange _values;
		OPX_CriteriaRange _criteria;
	};

	/**
	 *
	 */
	class compositeOffsetProfileXForm : public BaseVehComplianceProfileXForm
	{
	public:
		typedef std::map<XF_Type, IVehComplianceProfileXForm *> XForms;
		typedef bool (*F_RULE)(IVehComplianceProfileXForm *xf);

		compositeOffsetProfileXForm(const char *title, EEIObject *parent, IVehComplianceProfileXForm *owner = NULL)
		{
			setTitle(title);
			setOwner( owner );
			setParent(parent);
		}
		virtual ~compositeOffsetProfileXForm()
		{
			for (XForms::iterator it = _xforms.begin(); it != _xforms.end(); it++)
				delete it->second;
		}

		virtual std::string report()
		{
			std::string r = title() + std::string(": USER_PARAMS(");
			for (ParamsMap::iterator it = _user_params.begin(); it != _user_params.end(); it++)
			{
				if (it->first == "xform_type")
					r += it->first + ":" + std::to_string((XF_Type)(int)it->second) + ", ";
				else
				if (it->second > -1E100)
					r += it->first + ":" + std::to_string(it->second) + ", ";
			}
			r += ")";
			return r;
		}

		virtual void apply(DoubleVec &v)
		{
			for (XForms::iterator it = _xforms.begin(); it != _xforms.end(); it++)
			{
				if (_f_rule(it->second))
				{
					it->second->apply(v);
					break;
				}
			}
		}

		void setRule(F_RULE fr)
		{
			REQUIRE(fr != NULL);
			_f_rule = fr;
		}

		void addXForm(const XF_Type type, IVehComplianceProfileXForm *xform)
		{
			REQUIRE(xform != NULL);
			XForms::iterator it = _xforms.find(type);
			if (it != _xforms.end())
				delete it->second;
			_xforms[type] = xform;
			xform->setOwner(this);
		}
		IVehComplianceProfileXForm *getXform(const XF_Type type)
		{
			XForms::iterator it = _xforms.find(type);
			if (it != _xforms.end())
				return it->second;
			return NULL;
		}

		//	Adds a parameter value with name specified by 'paramName'
		virtual IVehComplianceProfileXForm &set(const char *paramName, double v)
		{
			REQUIRE(paramName != NULL);
			_user_params[paramName] = v;
			return *this;
		}

		//	Returns a user-defined parameter by its name
		virtual double get(const char *paramName)
		{
			REQUIRE(paramName != NULL);
			ParamsMap::iterator it = _user_params.find(paramName);
			if (it != _user_params.end())
				return it->second;
			pflogErr("***ERROR: %s::get() param %s not recognised\n", title(), paramName);
			pslogErr("***ERROR: %s::get() param %s not recognised\n", title(), paramName);
			return 0.0;
		}

	private:
		XForms _xforms;
		F_RULE _f_rule;
		ParamsMap _user_params;
	};

	/**
	 *	Simple uniform speed profile scaling by a fixed factor
	 */
	class scaleProfileXform : public IVehComplianceProfileXForm
	{
	public:
		scaleProfileXform() : _scaleFactor(1.0) { }
		scaleProfileXform(double sf) : _scaleFactor(sf) { }
		virtual void apply(DoubleVec &v)
		{
			for (DoubleVec::iterator it = v.begin(); it != v.end(); it++)
				*it *= _scaleFactor;
		}

		void setScaleFactor(double f) { _scaleFactor = f; }
		double getScaleFactor() { return _scaleFactor; }
	private:
		double _scaleFactor;
	};


	/**
	 *	Speed profile scaling by a pseudo-random factor within a given interval
	 */
	class randomScaleProfileXform : public scaleProfileXform
	{
	public:
		randomScaleProfileXform() { }
		randomScaleProfileXform(double l, double u) : _lower(l), _upper(u) { }
		virtual void apply(DoubleVec &v)
		{
			double size = _upper - _lower;
			double f = ((double)rand())/ RAND_MAX;
			setScaleFactor(_upper - f * size);
			scaleProfileXform::apply(v);
		}
		void setBounds(double lower, double upper) { _lower = lower; _upper = upper;}
		double getLower() { return _lower; }
		double getUpper() { return _upper; }
	private:
		double _lower;
		double _upper;
	};

} // namespace eei

#endif