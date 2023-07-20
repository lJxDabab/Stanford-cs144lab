#pragma once

#include "reassembler.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

class TCPReceiver
{
  private:
   Wrap32 ackno=Wrap32{0};
   uint16_t window_size {0};
   bool syn=false;
public:
  /*
   * The TCPReceiver receives TCPSenderMessages, inserting their payload into the Reassembler
   * at the correct stream index.
   */
  explicit TCPReceiver();
  void receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream );
  /* The TCPReceiver sends TCPReceiverMessages back to the TCPSender. */
  TCPReceiverMessage send( const Writer& inbound_stream ) const;
};
