#include "reassembler.hh"

using namespace std;
Reassembler::Reassembler():buffer_(0,' '),flag_(0,false),first_unassemble_index(0),first_unacceptable_index(0),pend_bytes(0),last_turn(0){}
void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  if(!last_turn)
  {
  first_unacceptable_index=first_unassemble_index+output.available_capacity();
  }
  uint64_t last_index=first_index+data.size();
  while(first_index<first_unacceptable_index&&first_index>first_unassemble_index+buffer_.size())
  {
      buffer_.push_back(' ');
      flag_.push_back(false);
  }
  for(uint64_t i=first_index;i<first_unacceptable_index&&i<last_index;i++)
  {
      if(i>=first_unassemble_index+buffer_.size())
      {
        buffer_.push_back(data[i-first_index]);
        flag_.push_back(true);
        pend_bytes++;
      }
      else if(i>=first_unassemble_index&&(!flag_[i-first_unassemble_index])){
        buffer_[i-first_unassemble_index]=data[i-first_index];
        flag_[i-first_unassemble_index]=true;
        pend_bytes++;
      }
  }

  string outcome="";
  while (!flag_.empty()&&flag_.front())
  {
    outcome+=buffer_.front();
    buffer_.pop_front();
    flag_.pop_front();
  }
  if(!outcome.empty())
  {
  output.push(outcome);
  pend_bytes-=outcome.size();
  first_unassemble_index=output.bytes_pushed();
  }
  if(is_last_substring)
  {
   last_turn=true;
   first_unacceptable_index=first_index+data.size();
  }
  if(last_turn&&(first_unassemble_index==first_unacceptable_index))
  {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return {pend_bytes};
}
