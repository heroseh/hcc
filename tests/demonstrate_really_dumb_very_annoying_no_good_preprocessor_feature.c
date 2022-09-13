
// I ran into this when parsing the handmade hero codebase which I would have
// been happy if I'd gone my whole life not knowing about
#if 1
int function_taking_three_arguments(int a, int b, int c)
{
  return 0;
}

#define dumb_preprocessor_feature(a, b, ...) \
  function_taking_three_arguments(a, b, ## __VA_ARGS__)

int main()
{
  dumb_preprocessor_feature(1, 2, 3);
}

#endif


// Just happened to notice the parser doesn't handle this.  Not sure if this is 
#if 0
void func_with_no_return_value()
{
  return;
}
#endif

// Not sure if the preprocessor is meant to handle this case yet, but it doesn't
#define bug3 0
#if bug3
this doesnt error, oh noes!
#endif
