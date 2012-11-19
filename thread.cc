// compile as g++-4.7 --std=c++11 threads.cc -lpthread -o threads
// g++ 4.7.2 will compile fine without the -lpthread, but (at least on my machine) the resulting binary burns and dies. -lpthread does need to occur after the .cc to work, for reasons not clear to me.

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono> // timing, aw yeah
#include <atomic>
#include <cmath> // later irrelevant

#define TICKS_PER_PASS 5000
#define COUNT 1000

using namespace std;
typedef typename chrono::steady_clock mclock;

// OK, so we have one global - but it _really is_ a global property, and must be common to _all_ functions, _everywhere_.
mutex printMutex;


// Version not a member function: takes a control structure reference as a param.
struct Box1 {
    float ks[COUNT];
    atomic<int> pos{0};
    /* Having pos be atomic guarantees that if two threads do x=pos++,
     * one will receive 0 and the other 1 and pos will end with value 2.
     * This makes it great for divvying up work. You can accomplish the
     * same thing (in more generality) using locks, but this is much harder
     * to get right and usually slower. On the other hand, using locks will
     * allow the use of eg iterators or queues.
     */
};

void worker1(Box1& b, int threadNum) {
  int ind;
  while(true) {
    ind = b.pos++; // atomic!
    // sure, the count might go too high - but who cares?
    if(ind >= COUNT) {
      break;
    }
    
    {
      // lock_guard is released when printLock leaves scope
      //lock_guard<mutex> printLock(printMutex);
      //cout << "a" << threadNum << ":" << ind << endl;
    }
    
    // mess with data
    for(int i=0; i<TICKS_PER_PASS; i++) {
	b.ks[ind] = fmod(pow(3.401, b.ks[ind]), 23.1);
    }
  }
}



// Version member function: access data local to the particular control structure; requires no params.
struct Box2 {
    float ks[COUNT];
    atomic<int> pos{0};

    void worker2(int threadNum) {
      int ind;
      while(true) {
	ind = pos++; // atomic!
	// sure, the count might go too high - but who cares?
	if(ind >= COUNT) {
	    break;
	}
	
	{
	  // lock_guard is released when printLock leaves scope
	  //lock_guard<mutex> printLock(printMutex);
	  //cout << "b" << threadNum << ":" << ind << endl;
	}
	
	
	// mess with data	
	for(int i=0; i<TICKS_PER_PASS; i++) {
	    ks[ind] = fmod(pow(3.401, ks[ind]), 23.1);
	}
      }
    }
};





int main() {
  
  Box1 b1;
  Box2 b2;
  
  for(int i=0; i<COUNT; i++) {
     b1.ks[i] = 2.38581;
     b2.ks[i] = 2.38581;
  }
  
  cout << "Run several times and observe that each index in each object is processed by the threads for that object in a different, not necessarily monotonic order, but each index is indeed touched only once." << endl;
  cout << "For example, we might see 'a0:0 a0:2 a0:3 a1:1 a0:4' : index 1 is processed by the second thread after indices 2 and 3 have already been processed by the first." << endl; 
  
  
  //cout << thread::hardware_concurrency() << endl;;
  auto start = mclock::now();
  
  /* t1a MUST use ref(b1), not just b1, because thread() takes constant references
   * and so will attempt to store b1 by value, which you don't want (and which is
   * impossible because b1 contains an atomic, which is not copyable).
   * See: http://stackoverflow.com/a/5116923/1644272
   */
  //thread t1a(worker1, ref(b1), 0);
  //thread t1b(worker1, ref(b1), 1);
  thread t2a(&Box2::worker2, &b2, 0); // alternative thread-control mechanism (member functions)
  thread t2b(&Box2::worker2, &b2, 1);
  //thread t2c(&Box2::worker2, &b2, 2);
  //t1a.join();
  //t1b.join();
  t2a.join();
  t2b.join();
  //t2c.join();
  
  auto end = mclock::now();
  cout << "time taken (ms): " << chrono::duration<double, milli>(end-start).count() << endl;
  
  cout << endl;
  cout << "Proof that each entry of each array was touched exactly once:" << endl;
  for(int i=0; i<COUNT; i++) {
     //cout << b1.ks[i] << endl;
     //cout << b2.ks[i] << endl;
  }

}