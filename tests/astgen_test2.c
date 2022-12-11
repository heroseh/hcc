
typedef int int32;

typedef struct Test Test;
struct Test {
	int a;
	struct {
		int b;
	};
	int c;
};

void func(Test t);

int extern_func();

enum {
	ENUM_ONE,
	ENUM_TWO,
	ENUM_THREE,
};

enum Test {
	TEST_ONE
} another;

struct Test var;

static void static_func() {

}

int extern_func() {
	func(var);

	return 1;
}

