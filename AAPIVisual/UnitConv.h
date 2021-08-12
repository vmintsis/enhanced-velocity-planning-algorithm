#ifndef _UnitConv_H_
#define _UnitConv_H_

class UnitConv
{
public:
	UnitConv() { }

	double f2m(double f) { return f/3.28084; }
	double m2f(double f) { return f*3.28084; }

	double kmph2mps(double v) { return v/3.6; }
	double mps2kmph(double v) { return v*3.6; }
};
#endif