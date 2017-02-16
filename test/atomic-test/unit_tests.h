#pragma once

#define TESTING_SIZE 32
#include "sub_unsigned.h"
#undef TESTING_SIZE

#define TESTING_SIZE 64
#include "sub_unsigned.h"
#undef TESTING_SIZE


#define TESTING_SIZE 32
#include "sub_signed.h"
#undef TESTING_SIZE

#define TESTING_SIZE 64
#include "sub_signed.h"
#undef TESTING_SIZE


#define TESTING_SIZE 32
#include "add_unsigned.h"
#undef TESTING_SIZE


#define TESTING_SIZE 16
#include "and_unsigned.h"
#undef TESTING_SIZE


#define TESTING_FETCH_AND_X

#define TESTING_SIZE 32
#include "add_unsigned.h"
#undef TESTING_SIZE

#define TESTING_SIZE 64
#include "add_unsigned.h"
#undef TESTING_SIZE


#define TESTING_SIZE 16
#include "or_unsigned.h"
#undef TESTING_SIZE

#undef TESTING_FETCH_AND_X
