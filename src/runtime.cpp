#include <cstdint>
#include <cstdio>

extern "C" {

	void put_int(int32_t x) {
		printf("put_int: %d\n", x);
	}

}
