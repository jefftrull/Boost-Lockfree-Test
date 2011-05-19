// Test program for Boost proposed Lockfree library: ring buffer container
// traditional locking approach (for comparison)
// Jeff Trull <jetrull@sbcglobal.net> 2011-05-18

#undef NDEBUG

#include <iostream>
#include <algorithm>
#include <boost/lockfree/ringbuffer.hpp>

#include <boost/thread.hpp>

static const int opcount = 100000000;
static const int ringsize = 1000;
static boost::lockfree::ringbuffer<int, ringsize> ring;

// vary amount added/removed each time
static const int chunksize_max = 5;
static int consumer_chunksize = 1;   // intial value different from producer is more interesting IMO
static int producer_chunksize = chunksize_max;

static int produced_count = 0;
static int consumed_count = 0;

void producer() {
  int enqvalues[chunksize_max];

  while (produced_count < opcount) {
    int total_added = 0;
    while (total_added < producer_chunksize) {
      // assemble input data
      for (int j = 0; j < (producer_chunksize - total_added); ++j) {
	enqvalues[j] = produced_count + total_added + j;
      }
      total_added += ring.enqueue(enqvalues, producer_chunksize - total_added);
    }
    produced_count += producer_chunksize--;
    if (producer_chunksize < 1)
      producer_chunksize = chunksize_max;
    producer_chunksize = std::min(producer_chunksize, (opcount - produced_count));  // don't go over on final iteration
  }
}

void consumer() {
  int deqvalues[chunksize_max];

  while (consumed_count < opcount) {
    // spin until we succeed
    int total_removed = 0;
    while (total_removed < consumer_chunksize) {
      int removed_this_call = ring.dequeue(deqvalues, consumer_chunksize - total_removed);
      for (int j = 0; j < removed_this_call; ++j) {
	assert(deqvalues[j] == (consumed_count + total_removed + j));
      }
      total_removed += removed_this_call;
    }
    consumed_count += consumer_chunksize--;
    if (consumer_chunksize < 1)
      consumer_chunksize = chunksize_max;
    consumer_chunksize = std::min(consumer_chunksize, (opcount - consumed_count));
  }
}

int main(int argc, char **argv) {
  // launch threads
  boost::thread consthread(consumer);
  boost::thread prodthread(producer);

  // wait for both to finish
  consthread.join();
  prodthread.join();

  // ring should now be empty
  assert(ring.empty());

  return 0;
}
