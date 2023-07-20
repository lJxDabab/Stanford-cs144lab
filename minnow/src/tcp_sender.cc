#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
Timer::Timer(uint64_t RTO):RTO_(RTO){}
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms ),timer(initial_RTO_ms)
{}
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return {send_syn-rec_ack};
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return {cnt};
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
TCPSenderMessage m;
    if(timer.get_TimerTurn())
    {
      m=unack_deq.back();
     timer.set_TimerTurn(false);
      return {m};
    }
    else if(!read_deq.empty()){
    m=read_deq.back();

    send_syn=m.seqno.unwrap(isn_,send_syn)+m.payload.size()+m.SYN+m.FIN;
    unack_deq.push_front(m);
    read_deq.pop_back();
    if(!timer.started())
    {
    timer.start();
    }
    return {m};
    }
    return {};
}
void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  string buf;
  TCPSenderMessage n;
  if(!window_size_)
  {
    if(send_syn==rec_ack)
    {
      window_size_=1;
      if(!syn && !outbound_stream.is_finished())
      {
        zero_turn=true;
      }
    }
  }
    if(outbound_stream.bytes_buffered() && !fin)
    {
      buf=outbound_stream.peek();
      while(window_size_>=TCPConfig::MAX_PAYLOAD_SIZE&&buf.size()>=TCPConfig::MAX_PAYLOAD_SIZE)
      {
        TCPSenderMessage m;
        if(syn)
        {
          m.SYN=true;
          syn=false;
          window_size_=window_size_-1;
        }
        m.payload=Buffer(buf.substr(0,TCPConfig::MAX_PAYLOAD_SIZE));
        m.seqno=isn_.wrap((outbound_stream.bytes_popped()+!m.SYN),isn_);
        outbound_stream.pop(m.payload.size());
          if(outbound_stream.is_finished())
        {
          m.FIN=true;
          fin=true;
          window_size_=window_size_-1;
        }
        read_deq.push_front(m);
        window_size_-=TCPConfig::MAX_PAYLOAD_SIZE;
        buf=buf.substr(TCPConfig::MAX_PAYLOAD_SIZE);
      }
      string p;
      if(!buf.empty() && window_size_)
      {
      if(outbound_stream.is_finished())
        {
          n.FIN=true;
          fin=true;
          window_size_=window_size_-1;
        }
        if(syn)
        {
          n.SYN=true;
          syn=false;
          window_size_=window_size_-1;
        }
      p=buf.substr(0,buf.size()>window_size_?window_size_:buf.size());
      n.seqno=isn_.wrap((!syn+outbound_stream.bytes_popped()),isn_);
      n.payload=Buffer(p);
      send_syn+=n.payload.size();
      outbound_stream.pop(p.size());
      window_size_=window_size_-static_cast<uint16_t>(p.size());
      }
      uint64_t len;
      if(window_size_>0&&outbound_stream.bytes_buffered())
        {
          buf=outbound_stream.peek();
          len=buf.size()>TCPConfig::MAX_PAYLOAD_SIZE?TCPConfig::MAX_PAYLOAD_SIZE-p.size():buf.size()-p.size();
          p.append(buf,len);
          n.payload=Buffer(p);
          buf=buf.substr(len);
          outbound_stream.pop(len);
          read_deq.push_front(n);
      while(buf.size()>=TCPConfig::MAX_PAYLOAD_SIZE)
      {
        TCPSenderMessage m;
        if(outbound_stream.is_finished()&&window_size_>0)
        {
          m.FIN=true;
          fin=true;
          window_size_=window_size_-1;
        }
        m.payload=Buffer(buf.substr(0,TCPConfig::MAX_PAYLOAD_SIZE));
        m.seqno=isn_.wrap((syn+outbound_stream.bytes_popped()-m.SYN),isn_);
        read_deq.push_front(m);
        buf=buf.substr(TCPConfig::MAX_PAYLOAD_SIZE);
        outbound_stream.pop(m.payload.size());
      }
       if(!buf.empty())
      {
        TCPSenderMessage q;
      if(outbound_stream.is_finished() && window_size_>0)
        {
          q.FIN=true;
          fin=true;
          window_size_=window_size_-1;
        }
      q.payload=Buffer(buf);
      q.seqno=isn_.wrap((syn+outbound_stream.bytes_popped()-n.SYN),isn_);
      outbound_stream.pop(q.payload.size());
      read_deq.push_front(q);
      }
        }
      else if(!n.payload.empty()){
        if(outbound_stream.is_finished() && window_size_>0)
        {
          n.FIN=true;
          fin=true;
          window_size_=window_size_-1;
        }
        read_deq.push_front(n);
      }
    }
    else{
      if(syn)
      {
        n.SYN=syn;
        syn=false;
        send_syn=isn_.unwrap(isn_,send_syn);
        n.seqno=Wrap32(isn_);
        read_deq.push_front(n);
        window_size_=window_size_-1;
      }
      if(!fin && outbound_stream.is_finished()&& window_size_>0)
      {
        if(!read_deq.empty())
        {
          fin=true;
          n=read_deq.back();
          n.FIN=fin;
          read_deq.pop_back();
          read_deq.push_back(n);
          window_size_=window_size_-1;
        }
        else{
          fin=true;
          n.SYN=syn;
          n.FIN=fin;
          n.seqno=Wrap32(isn_+send_syn);
          send_syn=send_syn+1;
          read_deq.push_front(n);
          window_size_=window_size_-1;
         }
      }
    }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  TCPSenderMessage m;
  m.SYN=syn;
  m.seqno=Wrap32(isn_+send_syn);
  return {m};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  uint64_t ex_cet=rec_ack;
  if(msg.ackno)
  {
    ex_cet=msg.ackno->unwrap(isn_,rec_ack);
  }
  if(send_syn<ex_cet)
  {
    return;
  }
  else if(ex_cet>rec_ack){
  rec_ack=ex_cet;
  }
  if(rec_ack+msg.window_size>send_syn)
  {
      window_size_=rec_ack+msg.window_size-send_syn;
      zero_turn=false;
  }
  while(!unack_deq.empty()&&rec_ack>=(unack_deq.back().seqno.unwrap(isn_,rec_ack)+unack_deq.back().FIN+unack_deq.back().payload.size()+unack_deq.back().SYN))
    {
      unack_deq.pop_back();
      timer.set_RTO(initial_RTO_ms_);
      if(unack_deq.empty())
      {
      timer.stop();
      }
      else{
        timer.start();
      }
      cnt=0;
    }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  timer.set_ticks(ms_since_last_tick);
   if(timer.expired(timer.get_RTO()))
    {
      cnt++;
      if(!zero_turn)
      {
      timer.set_RTO(2*timer.get_RTO());
      }
      timer.set_TimerTurn(true);
    }
}
