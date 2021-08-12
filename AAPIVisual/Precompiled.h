#ifndef _Precompiled_H_
#define _Precompiled_H_

#include "windows.h"
#include <varargs.h>
#include <assert.h>

#include <stdio.h>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <math.h>

#define REQUIRE(x) assert(x)
#define Assert(x) assert(x)

#include "UnitConv.h"
const double PI= 3.14159265359;
#define ACCCEL_FACTOR 0.05
#define INFLUENCE_ZONE_CALIBRATION_PARAM 4.0
#define MIN_SPEED 20.0

#include "AKIProxie.h"
#include "CIProxie.h"
#include "ANGConProxie.h"
#include "Section_Str.h"

#include "CLog.h"
#include "ServiceProvider.h"

#endif