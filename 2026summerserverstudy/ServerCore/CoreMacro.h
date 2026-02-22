#pragma once

/*-----------------------------------------
					crash
------------------------------------------*/

//강제로 crash발생
#define CRASH(cause){						\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}

//조건부 crash
#define ASSERT_CRASH(expr){			\
	if (!(expr)){					\
		CRASH("ASSERT_CRASH");		\
		__analysis_assume(expr);	\
	}								\
}