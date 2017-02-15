#pragma once

#define TESTED_SIZE 32
#include "sub_unsigned.h"
#undef TESTED_SIZE

#define TESTED_SIZE 64
#include "sub_unsigned.h"
#undef TESTED_SIZE


#define TESTED_SIZE 32
#include "sub_signed.h"
#undef TESTED_SIZE

#define TESTED_SIZE 64
#include "sub_signed.h"
#undef TESTED_SIZE


#define TESTED_SIZE 32
#include "add_unsigned.h"
#undef TESTED_SIZE


#define TESTED_SIZE 16
#include "and_unsigned.h"
#undef TESTED_SIZE


#define TESTED_FETCH_FIRST

#define TESTED_SIZE 32
#include "add_unsigned.h"
#undef TESTED_SIZE

#define TESTED_SIZE 64
#include "add_unsigned.h"
#undef TESTED_SIZE


#define TESTED_SIZE 16
#include "or_unsigned.h"
#undef TESTED_SIZE

#undef TESTED_FETCH_FIRST
