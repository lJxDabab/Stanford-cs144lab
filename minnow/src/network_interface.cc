#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"
#include "parser.hh"
using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  EthernetFrame EthrFrame;
  auto i=ip_Etr_Table.find(next_hop.ipv4_numeric());
  if(i!=ip_Etr_Table.end())
  {
    EthrFrame.header.dst=i->second.first;
    EthrFrame.header.src=ethernet_address_;
    EthrFrame.header.type=EthernetHeader::TYPE_IPv4;
    EthrFrame.payload=serialize(dgram);
    send_list.push_front(EthrFrame);
  }
  else{
      // auto j=find(unknwdst_pack_list.begin(),unknwdst_pack_list.end(),{dgram,next_hop});
      auto j=unknwdst_pack_list.begin();
      for(;j!=unknwdst_pack_list.end();j++)
      {
        if(next_hop.ipv4_numeric()==(*j).second.ipv4_numeric())
        {
          break;
        }
      }
      if(j!=unknwdst_pack_list.end())
      {   
          if(timer-last_time<5000)  //wrong here
          {
            return;
          }
      }
      else{
         unknwdst_pack_list.push_front({dgram,next_hop}); 
      }
      ARPMessage arp;
      arp.sender_ethernet_address=ethernet_address_;
      arp.sender_ip_address=ip_address_.ipv4_numeric();
      arp.target_ethernet_address=ARP_BROADCAST;
      arp.target_ip_address=next_hop.ipv4_numeric();
      arp.opcode=ARPMessage::OPCODE_REQUEST;
      EthrFrame.header.type=EthernetHeader::TYPE_ARP;
      EthrFrame.header.src=ethernet_address_;
      EthrFrame.header.dst=ETHERNET_BROADCAST;
      EthrFrame.payload=serialize(arp);
      send_list.push_front(EthrFrame);
  }
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  if(frame.header.dst!=ETHERNET_BROADCAST&&frame.header.dst!=ethernet_address_)
  {
    return {};
  }
  else{
    if(frame.header.type==EthernetHeader::TYPE_ARP)
    {
      ARPMessage arp;
        if(parse(arp,frame.payload)==true)
        {
          if(arp.target_ip_address==ip_address_.ipv4_numeric())
          {
            pair<uint32_t,pair<EthernetAddress,uint64_t>>both{arp.sender_ip_address,{arp.sender_ethernet_address,30000UL}};
            if(ip_Etr_Table.find(arp.sender_ip_address)==ip_Etr_Table.end())
            {
                ip_Etr_Table.insert(both);
            }
            if(arp.opcode==ARPMessage::OPCODE_REPLY)
            {
               for(auto i=unknwdst_pack_list.begin();i!=unknwdst_pack_list.end();)
               {
                if(i->second.ipv4_numeric()==arp.sender_ip_address)
                {
                  send_datagram(i->first,i->second);
                  unknwdst_pack_list.erase(i++);
                }
               }
            }
            else{
              ARPMessage m;
              EthernetFrame EthrFrame;
              m.sender_ethernet_address=ethernet_address_;
              m.sender_ip_address=ip_address_.ipv4_numeric();
              m.target_ethernet_address=arp.sender_ethernet_address;
              m.target_ip_address=arp.sender_ip_address;
              m.opcode=ARPMessage::OPCODE_REPLY;
              EthrFrame.header.type=EthernetHeader::TYPE_ARP;
              EthrFrame.header.src=ethernet_address_;
              EthrFrame.header.dst=arp.sender_ethernet_address;
              EthrFrame.payload=serialize(m);
              send_list.push_front(EthrFrame);
            }
          }
        }
        else{
            return {};
        }
    }
    else{
      InternetDatagram pack;
        if(parse(pack,frame.payload)==true)
        {
            return {pack};
        }
        else{
          return{};
        }
    }
  }
  return {};
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  timer=timer+ms_since_last_tick;//wrong here
  for(auto i=ip_Etr_Table.begin();i!=ip_Etr_Table.end();)
  {
    if(i->second.second<ms_since_last_tick)
    {
      ip_Etr_Table.erase(i++);
      continue;
    }
    else{
      i->second.second-=ms_since_last_tick;
    }
    i++;
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if(send_list.empty())
  {
    return {};
  }
  else{
    EthernetFrame m=send_list.back();
    send_list.pop_back();
    return {m};
  }
}
