

typedef int int32;

typedef struct Test Test;

struct Test {
	int a;
	struct {
		int b;
	};
	int c;
};

enum {
	ENUM_ONE,
	ENUM_TWO,
	ENUM_THREE,
};

enum Test {
	TEST_ONE
} another;

struct Test var;

void func();
void func();
void func();

