
typedef int int32;

typedef struct Test Test;
struct Test {
	int a;
	struct {
		int b;
	};
	int c;
};

int extern_func();

void func(Test t);

void func(Test t) {
}

enum {
	ENUM_ONE,
	ENUM_TWO,
	ENUM_THREE,
};

extern enum Test {
	TEST_ONE
} another;

extern struct Test var;

static void static_func() {

}

int func2() {
	int test = 0;

	if (test) {
		test = 3;
	} else {
		test += 3;
	}

	while (test) {
		test--;
	}

	switch (test) {
		case 0:
			test = 1;
			break;
	}

	for (int i = 2; i < 3; ++i) {
		test *= i + test * 12;
		continue;
	}

	int a = 2, b = 3;

	extern_func();

	a += i1 + i2 + i3 + i4 + i5;
	return a ? test : b;
}

