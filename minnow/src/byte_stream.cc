#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ),close_(0),error_(0),bytes_writed(0),bytes_readed(0),ring_Write(0),ring_Read(capacity_),buf(capacity+1,' '){}

void Writer::push( string data )
{
  // Your code here.
  data=data.substr(0,(data.size()>available_capacity()?available_capacity():data.size()));
  if(ring_Write<ring_Read)
  {
    buf.replace(ring_Write,data.size(),data);
  }
  else{
    if(data.size()<=capacity_+1-ring_Write)
    {
      buf.replace(ring_Write,data.size(),data);
    }
    else{
      buf.replace(ring_Write,data.substr(0,capacity_+1-ring_Write).size(),data.substr(0,capacity_+1-ring_Write));
      buf.replace(0,data.substr(capacity_+1-ring_Write).size(),data.substr(capacity_+1-ring_Write));
    }
  }
  bytes_writed+=data.size();
  ring_Write=(ring_Write+data.size())%(capacity_+1);
}

void Writer::close()
{
  close_=1;
  // Your code here.
}

void Writer::set_error()
{
  // Your code here.
  error_=1;
}

bool Writer::is_closed() const
{
  // Your code here.
  return {close_};
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  uint64_t buffer;
  if(ring_Write>ring_Read)
  {
     buffer=capacity_+1-ring_Write+ring_Read;
  }
  else{
     buffer=ring_Read-ring_Write;
  }
  return {buffer};
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return {bytes_writed};
}
string s;
string_view Reader::peek() const
{
  // Your code here.
  
  uint64_t read_Stander=(ring_Read+1)%(capacity_+1);
  if(read_Stander==ring_Write)
  {
  }
  else if(ring_Write>read_Stander){
    s=buf.substr(read_Stander,ring_Write-read_Stander);
   return {s};
  }else{
    s=buf.substr(read_Stander,1+capacity_-read_Stander);
    return {s};
  }
  return {};
}

bool Reader::is_finished() const
{
  // Your code here.
  return {close_&&(bytes_buffered()==0)};
}
bool Reader::has_error() const
{
  // Your code here.
  return {error_};
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  bytes_readed+=len;
  ring_Read=(ring_Read+len)%(capacity_+1);
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  uint64_t buffer;
  if(ring_Write>ring_Read)
  {
    buffer=ring_Write-ring_Read-1;
  }
  else{
    buffer=capacity_-ring_Read+ring_Write;
  }
  return {buffer};
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return {bytes_readed};
}
