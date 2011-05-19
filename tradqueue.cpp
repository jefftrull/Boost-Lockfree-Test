// Test program for Boost proposed Lockfree library: fifo queue container
// traditional locking approach (for comparison)
// Jeff Trull <jetrull@sbcglobal.net> 2011-05-18

#undef NDEBUG

#include <iostream>
#include <queue>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

static const int opcount = 50000000;
static std::queue<int> queue;
static boost::mutex shared_lock;

void producer() {
  int produced_count = 0;
  while (produced_count < opcount) {
    // lock
    boost::mutex::scoped_lock sl(shared_lock);
    queue.push(produced_count++);
    // unlock
  }
}

void consumer() {
  int consumed_count = 0;
  while (consumed_count < opcount) {
    // lock
    boost::mutex::scoped_lock sl(shared_lock);
    if (!queue.empty()) {
      int front = queue.front();
      assert(front == consumed_count);
      queue.pop();
      consumed_count++;
    }
    // unlock
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
