// Test program for Boost proposed Lockfree library: ring buffer container
// traditional locking approach (for comparison)
// Jeff Trull <jetrull@sbcglobal.net> 2011-05-18

#include <iostream>
#include <algorithm>
#include <boost/circular_buffer.hpp>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

static const int opcount = 100000000;
static const int ringsize = 1000;
static boost::circular_buffer<int> ring(ringsize);
static boost::mutex shared_lock;

// vary amount added/removed each time
static const int chunksize_max = 5;
static int consumer_chunksize = 1;   // intial value different from producer is more interesting IMO
static int producer_chunksize = chunksize_max;

void producer() {
  int i = 0;
  while (i < opcount) {
    // spin until there's room to insert our data
    assert(ring.capacity() >= ring.size());
    while ((ring.capacity() - ring.size()) < producer_chunksize) {
      boost::this_thread::yield();
    }
    // lock
    boost::mutex::scoped_lock sl(shared_lock);
    for (int j = 0; j < producer_chunksize; ++j) {
      ring.push_back(i + j);
    }
    i += producer_chunksize--;
    if (producer_chunksize < 1)
      producer_chunksize = chunksize_max;
    producer_chunksize = std::min(producer_chunksize, (opcount - i));  // don't go over on final iteration
    // unlock
  }
}

void consumer() {
  int i = 0;
  while (i < opcount) {
    // spin until ring contains enough data to supply us
    while (ring.size() < consumer_chunksize) {
      boost::this_thread::yield();
    }
    // lock
    boost::mutex::scoped_lock sl(shared_lock);
    for (int j = 0; j < consumer_chunksize; ++j) {
      ring.pop_front();
    }
    i += consumer_chunksize--;
    if (consumer_chunksize < 1)
      consumer_chunksize = chunksize_max;
    consumer_chunksize = std::min(consumer_chunksize, (opcount - i));
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
