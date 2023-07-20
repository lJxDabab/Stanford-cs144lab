#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

class Timer {
    //! total elapsed time
    size_t ticks_{0};
    uint64_t RTO_{0};
    uint64_t last_time_out{0};
    //! timer is started or not
    bool _started{false};
    bool timer_turn{false};
  public:
    Timer(uint64_t RTO);
    bool expired(const unsigned timeout) {
        // The timer will only expire if it is started
        if(_started && (ticks_>= timeout+last_time_out))
          {
            last_time_out=last_time_out+timeout;
            return 1;
          }
          return 0;
    }

    //! \return true if timer is started
    bool started() const { return _started; }
    void set_RTO(const uint64_t rto){ RTO_=rto; }
    void set_ticks(const uint64_t ms){ ticks_=ticks_+ms;}
    //! stop the timer
    uint64_t get_RTO() const {return RTO_;}
    uint64_t get_ticks() const {return ticks_;}
    bool get_TimerTurn() const {return timer_turn;}
    void set_TimerTurn(bool turn) {timer_turn=turn;}
     void stop() { _started = false; }

    //! start the timer
    void start() {
        ticks_ = 0;
        _started = true;
        last_time_out=0;
    }
};

class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  deque<TCPSenderMessage> read_deq=deque<TCPSenderMessage>{0,TCPSenderMessage()};
  deque<TCPSenderMessage> unack_deq=deque<TCPSenderMessage>{0,TCPSenderMessage()};
  bool syn=true;
  bool fin=false;
  bool zero_turn{false};
  uint16_t window_size_{0};
  uint64_t retransmissions_nums_{0};
  uint64_t rec_ack{0};
  uint64_t send_syn{0};
  uint64_t cnt{0};
  Timer timer;
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
