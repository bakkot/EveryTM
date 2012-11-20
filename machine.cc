#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <cstring> // Sorry...
#include <vector>
#include <cmath>
#include <atomic>
#include <cstdio> // for rename()

#define TICKS_PER_PASS 10
#define MACHINES_PER_FILE 10

using namespace std;

// A tape for a binary Turing machine.
// Provides a constructor taking a string of 0s and 1s and a position therein
// Provides Move, Read, and Write methods
// Also provides repr(), which gives a string representation of the tape.
class Tape { // TODO: alternative implemention (chars?) speed tests
  list<bool> tape_;
  list<bool>::iterator head_;
  long position_;
  
public:
  Tape(string tape_string, long tape_position) {
    long current_position{0};
    for(auto bit: tape_string) {
      tape_.push_back(bit == '1');
      if(current_position == tape_position) {
	head_ = --tape_.end();
      }
      ++current_position;
    }
    
    position_ = tape_position;
  }
  
  Tape() {}
  
  /*
  void MoveLeft() {
    if(head_ == tape_.begin()) {
      tape_.push_front(false); // position remains 0
    }
    else {
      --position_;
    }
    --head_;
  }
  
  void MoveRight() {
    if(head_ == --tape_.end()) {
      tape_.push_back(false);
    }
    ++position_;
    ++head_;
  }*/
  
  void Move(int dir) {
    if(dir == 0) { // left
      if(head_ == tape_.begin()) {
	tape_.push_front(false); // prepend 0; position remains 0
      }
      else {
	--position_;
      }
      --head_;
    }
    else { // right
      if(head_ == --tape_.end()) {
	tape_.push_back(false); // append 0; position still incremented
      }
      ++position_;
      ++head_;
    }
  }
  
  
  bool Read() {
    return *head_;
  }
  
  // TODO maybe combine some or all of these or inline Tape
  void Write(bool bit) {
    *head_ = bit;
  }
  
  string repr() {
    char buf[128]; // must be able to hold a base10 repr of position. 
    sprintf(buf, "%ld\t", position_);
    string out(buf);
    for(auto bit: tape_) {
      out += bit?'1':'0';
    }
    return out;
  }
};


// A TuringMachine description looks like:
/***********************************************
<num_states>\t<state>\t<delta: 'state writebit dir state writebit dir ... (trailing space)'>\t<position>\t<tape: '01001'>\t<ticks>'
*/
class TuringMachine {
  int num_states_; // only used for IO but saved anyway
  long state_;
  vector<long> delta_; // TODO maybe Vector<int>? speedcheck
  Tape tape_;
  long ticks_;
  
public:
  // default constructor
  TuringMachine() {}
  
  // constructor from description
  TuringMachine(string description) {
    char* c_description;
    c_description = new char[description.length()+1];
    strcpy(c_description, description.c_str());
    char* end_p;
    num_states_ = strtol(c_description, &end_p, 10);
    state_ = strtol(end_p, &end_p, 10);
    for(long i=0; i<2*(num_states_-1); ++i) {
      delta_.push_back( strtol(end_p, &end_p, 10) );
      delta_.push_back( strtol(end_p, &end_p, 2) );
      delta_.push_back( strtol(end_p, &end_p, 2) );
    }
    end_p += 2; // trailing space and a tab
    long position = strtol(end_p, &end_p, 10);
    end_p += 1; // tab
    char* tab_p = strchr(end_p, '\t');
    string tape_string = string(end_p, static_cast<string::size_type>(tab_p-end_p));
    tape_ = Tape(tape_string, position);
    ticks_ = strtol(tab_p, NULL, 10);
  }
  
  
  // constructor from machine number
  TuringMachine(long machine_number) {
    /* first determine the number of states.
     * since all n-state machines are enumerated before any (n+1)-state
     * machines, this is easy: just count the number of machines machines
     * with fewer than 3, 4, etc states until we find the interval in which
     * the given machine_number lies.
     * 
     * There are (4n)^(2(n-1)) machines with n states, since an n-state
     * machine is completely characterized by it's delta function, which
     * is a function from {1..n-1}x{0,1} to {0..n-1}x{0,1}x{L,R} (no
     * transitions from the halting state need be defined). The first set
     * has cardinality 2(n-1), the second 4n, so there are 4n*4n*...*4n =
     * (4n)^(2(n-1)) n-state machines.
     * 
     * As such, machines 0..63 are the 2-state machines, 64..(64+20736-1) the
     * 3-state machines, and so on.
     */
    long next_num = 0;
    long num_below = 0;
    num_states_ = 1;
    while(machine_number >= next_num) {
      num_states_ += 1;
      num_below = next_num;
      next_num += pow(4*num_states_, 2*(num_states_-1));
    }
    
    state_ = 1; // all machines start in state 1
    
    
    long delta_number = machine_number - num_below;
    
    
    
    for(int i=0; i<2*(num_states_-1); ++i) {
      delta_.push_back( delta_number % num_states_ );
      delta_number = delta_number / num_states_;
      delta_.push_back( delta_number & 1 );
      delta_number = delta_number / 2;
      delta_.push_back( delta_number & 1 );
      delta_number = delta_number / 2;      
    }
    
    tape_ = Tape("0", 0); // all machines start with blank tape
    
    ticks_ = 0;
  }
  
  string repr() {
    char buf[256]; // must be able to hold num_states_ and also state_
    sprintf(buf, "%d\t%ld\t", num_states_, state_);
    string out(buf);
    for(auto d: delta_) {
      sprintf(buf, "%ld ", d);
      out += buf;
    }
    out += '\t';
    out += tape_.repr();
    sprintf(buf, "\t%ld", ticks_);
    out += buf;
    return out;
  }
  
  void Advance(long ticks) {
    for(long i=0; i<ticks; ++i) {
      if(state_ == 0) break; // halting state
      ++ticks_;
      long base_index = (tape_.Read()?2:1)*(state_-1);
      state_ = delta_[base_index];
      tape_.Write(delta_[base_index+1] == 1);
      tape_.Move(delta_[base_index+2]);
    }
  }
};

struct TMBoxen {
  TuringMachine boxen[MACHINES_PER_FILE];
  atomic<int> next_machine{0};
};

// todo variable declarations outside of loops( speedcheck)

int main() {
  long next_file_num = 0;
  char file_name_buf[1024];
  char tfile_name_buf[1024];
  
  TMBoxen box;
  for(int i=0; i<10; ++i) { // TODO while(true)

    // Load or create machines
    sprintf(file_name_buf, "%ld.txt", next_file_num);
    ifstream in_file(file_name_buf);
    if(!in_file.good()) {
      // whoops better generate some machines!      
      // don't forget to use ~ file
      for(int j=0; j<MACHINES_PER_FILE; j++) {
	box.boxen[j] = TuringMachine(next_file_num + j);
      }
      
      next_file_num = 0; // hit the end, so start over
    }
    else {
      string line;
      for(int loaded = 0; loaded < MACHINES_PER_FILE; ++loaded) {
	if(!getline(in_file, line)) {
	  cerr << file_name_buf << " contains too few machines; aborting!" << endl;
	  exit(1);
	}
	// skip lines not beginning with a digit
	if(line.length() == 0 || !('0' <= line[0] && line[0] <= '9')) { // or isdigit, if you like
	  --loaded;
	  continue; 
	}
	
	box.boxen[loaded] = TuringMachine(line);
      }
      
      next_file_num += MACHINES_PER_FILE;
    }
    in_file.close(); // close manually because we'll soon be overwriting
    
    // Run machines (TODO)
    
    // Write machines
    sprintf(tfile_name_buf, "%ld.txt~", next_file_num);
    ofstream out_file(tfile_name_buf);
    for(auto &m : box.boxen) {
      out_file << m.repr() << endl;
    }
    remove(file_name_buf);
    rename(tfile_name_buf, file_name_buf);
  }
}