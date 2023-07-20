#include "wrapping_integers.hh"
using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return Wrap32 {static_cast<uint32_t>((static_cast<uint64_t>(zero_point.raw_value_)+n)%(2UL<<32))};
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  uint64_t idx=checkpoint/(1UL<<32);
  uint32_t real_value_=raw_value_-zero_point.raw_value_;
  uint32_t off=static_cast<uint32_t>(checkpoint)%(1UL<<32);
  uint64_t zero_value=static_cast<uint64_t>(real_value_)+idx*(1UL<<32);
  int turn=off>real_value_?(off-real_value_)>(real_value_+(1U<<31)+(1U<<31)-off)?1:0 : (real_value_-off)>(off-real_value_+(1U<<31)+(1U<<31))?-1:0;
  if((idx==0 && (real_value_>off))||(idx==(((1UL<<63)-1+(1UL<<63))/(1UL<<32))&& off>real_value_)||real_value_==off)
  {
    turn=0;
  }
  return {zero_value+turn*(1UL<<32)};
}
