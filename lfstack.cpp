// Test program for Boost proposed Lockfree library: stack container
// Jeff Trull <jetrull@sbcglobal.net> 2011-05-18

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
  for (int i = 0; i < opcount; ++i) {
    // spin until stack has contents
    while (stack.empty()) {}
    int popval;
    stack.pop(&popval);
  }
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
