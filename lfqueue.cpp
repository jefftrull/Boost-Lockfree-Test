// Test program for Boost proposed Lockfree library: fifo queue container
// traditional locking approach (for comparison)
// Jeff Trull <jetrull@sbcglobal.net> 2011-05-18

// we want asserts, always
#undef NDEBUG

#include <iostream>
#include <queue>

#include <boost/thread.hpp>
#include <boost/lockfree/fifo.hpp>

static const int opcount = 50000000;
static boost::lockfree::fifo<int> queue;

void producer() {
  for (int i = 0; i < opcount; ++i) {
    queue.enqueue(i);
  }
}

void consumer() {
  for (int i = 0; i < opcount; ++i) {
    // spin until queue has contents
    while (queue.empty()) {}
    int popval;
    queue.dequeue(&popval);
    assert(popval == i);
  }
}

int main(int argc, char **argv) {
  // launch threads
  boost::thread consthread(consumer);
  boost::thread prodthread(producer);

  // wait for both to finish
  consthread.join();
  prodthread.join();

  // queue should now be empty
  assert(queue.empty());

  return 0;
}
