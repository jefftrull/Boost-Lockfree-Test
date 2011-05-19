// Test program for Boost proposed Lockfree library: stack container
// Jeff Trull <jetrull@sbcglobal.net> 2011-05-18

#undef NDEBUG

#include <iostream>
#include <stack>

#include <boost/thread.hpp>
#include <boost/lockfree/stack.hpp>

static const int opcount = 50000000;
static boost::lockfree::stack<int> stack;

void producer() {
  for (int i = 0; i < opcount; ++i) {
    stack.push(i);
  }
}

void consumer() {
  int checksum = 0;    // not really a checksum but will allow us to verify without overflowing
  for (int i = 0; i < opcount; ++i) {
    // spin until stack has contents
    while (stack.empty()) {}
    int top;
    stack.pop(&top);
    if (top % 2) {
      checksum += top;
    } else {
      checksum -= top;
    }
  }
  // the sum of 0 + 1 - 2 ... + (opcount - 1) assuming opcount is even:
  assert(checksum == (opcount / 2));
}

int main(int argc, char **argv) {
  // launch threads
  boost::thread consthread(consumer);
  boost::thread prodthread(producer);

  // wait for both to finish
  consthread.join();
  prodthread.join();

  // stack should now be empty
  assert(stack.empty());

  return 0;
}
