// Test program for Boost proposed Lockfree library: stack container
// traditional locking approach (for comparison)
// Jeff Trull <jetrull@sbcglobal.net> 2011-05-18

#undef NDEBUG

#include <iostream>
#include <stack>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

static const int opcount = 50000000;
static std::stack<int> stack;
static boost::mutex shared_lock;

void producer() {
  int produced_count = 0;
  while (produced_count < opcount) {
    // lock
    boost::mutex::scoped_lock sl(shared_lock);
    stack.push(produced_count++);
    // unlock
  }    
}

void consumer() {
  int consumed_count = 0;
  int checksum = 0;    // not really a checksum but will allow us to verify without overflowing
  while (consumed_count < opcount) {
    // lock
    boost::mutex::scoped_lock sl(shared_lock);
    if (!stack.empty()) {
      int top = stack.top();
      if (top % 2) {
	checksum += top;
      } else {
	checksum -= top;
      }
      stack.pop();
      consumed_count++;
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
