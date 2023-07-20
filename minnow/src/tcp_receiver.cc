#include "tcp_receiver.hh"
#include <algorithm>
using namespace std;
TCPReceiver::TCPReceiver(){};
void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  if(message.SYN)
  {
    ackno=message.seqno;
    syn=true;
  }
  if(syn)
  {
  reassembler.insert(message.seqno.unwrap(ackno+(syn-message.SYN),inbound_stream.bytes_pushed()),message.payload,message.FIN,inbound_stream);
  }
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  TCPReceiverMessage Smessage;
  if(syn)
  {
  Smessage.ackno=ackno+syn+inbound_stream.bytes_pushed()+inbound_stream.is_closed();
  }

  Smessage.window_size=min(65535UL,inbound_stream.available_capacity());
  return {Smessage};
}
