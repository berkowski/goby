// copyright 2011 t. schneider tes@mit.edu
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this software.  If not, see <http://www.gnu.org/licenses/>.

#include "test.pb.h"
#include "goby/acomms/queue.h"
#include "goby/acomms/connect.h"
#include "goby/util/binary.h"
#include "goby/util/logger.h"

// tests "encode_on_demand" functionality

using goby::acomms::operator<<;

int receive_count = 0;
int encode_on_demand_count = 0;
bool handle_ack_called = false;
int goby_message_qsize = 0;
int last_decoded_count = 0;
std::deque<int> decode_order;
//GobyMessage msg_in1, msg_in2;
goby::acomms::QueueManager* q_manager = goby::acomms::QueueManager::get();
const int MY_MODEM_ID = 1;
bool provide_data = true;


void handle_encode_on_demand(const goby::acomms::protobuf::ModemDataRequest& request_msg,
                             boost::shared_ptr<google::protobuf::Message> data_msg);

void qsize(goby::acomms::protobuf::QueueSize size);

void handle_receive(const google::protobuf::Message &msg);

void request_test(int request_bytes, int expected_encode_requests, int expected_messages_sent);

int main(int argc, char* argv[])
{    
    goby::glog.add_stream(goby::util::Logger::DEBUG3, &std::cerr);
    goby::glog.set_name(argv[0]);

    goby::acomms::DCCLCodec* codec = goby::acomms::DCCLCodec::get();
    
    q_manager->add_queue<GobyMessage>();
    
    goby::acomms::protobuf::QueueManagerConfig cfg;
    cfg.set_modem_id(MY_MODEM_ID);
    q_manager->set_cfg(cfg);

    goby::glog << *q_manager << std::endl;
    
    goby::acomms::connect(&q_manager->signal_queue_size_change, &qsize);
    goby::acomms::connect(&q_manager->signal_data_on_demand, &handle_encode_on_demand);
    goby::acomms::connect(&q_manager->signal_receive, &handle_receive);
    
    // we want to test requesting for a message slightly larger than the expected size
    decode_order.push_back(0);
    request_test(codec->size(GobyMessage()) + 1, 2, 1);
    assert(decode_order.empty());

    // no lag so it should be the *already* encoded message
    decode_order.push_back(1);
    request_test(codec->size(GobyMessage()), 0, 1);
    assert(decode_order.empty());

    // nothing in the queue so we'll try this again
    decode_order.push_back(2);
    decode_order.push_back(3);
    request_test(codec->size(GobyMessage()) * 2 + 1, 3, 2);
    assert(decode_order.empty());

    // 0.2 seconds > 0.1 seconds
    usleep(2e5);
    // 4 is too old so a new request made
    decode_order.push_back(5);
    request_test(codec->size(GobyMessage()), 1, 1);
    assert(decode_order.empty());

    // we won't provide data this time so the old message in the queue (4) should be sent
    provide_data = false;
    decode_order.push_back(4);
    request_test(codec->size(GobyMessage()), 1, 1);
    assert(decode_order.empty());
    
    
    std::cout << "all tests passed" << std::endl;    
}

void request_test(int request_bytes,
                  int expected_encode_requests,
                  int expected_messages_sent)
{
    int starting_qsize = goby_message_qsize;
    int starting_encode_count = encode_on_demand_count;
    
    goby::acomms::protobuf::ModemDataRequest request_msg;
    request_msg.set_max_bytes(request_bytes);
    request_msg.set_frame(0);
    
    goby::acomms::protobuf::ModemDataTransmission data_msg;

    q_manager->handle_modem_data_request(request_msg, &data_msg);
    std::cout << "requesting data for frame 0, got: " << data_msg << std::endl;

    // once for each one that fits, twice for the one that doesn't
    assert(encode_on_demand_count - starting_encode_count == expected_encode_requests);
    
    std::cout << "\tdata as hex: " << goby::util::hex_encode(data_msg.data()) << std::endl;    

    assert(data_msg.base().src() == MY_MODEM_ID);
    assert(data_msg.base().dest() == goby::acomms::BROADCAST_ID);
    assert(data_msg.ack_requested() == false);

    if(provide_data)
        assert(goby_message_qsize == starting_qsize + expected_encode_requests - expected_messages_sent);
    else
        assert(goby_message_qsize == starting_qsize - expected_messages_sent);

    q_manager->handle_modem_receive(data_msg);
}



void handle_encode_on_demand(const goby::acomms::protobuf::ModemDataRequest& request_msg,
                             boost::shared_ptr<google::protobuf::Message> data_msg)
{
    GobyMessage msg;

    if(provide_data)
        msg.set_telegram(encode_on_demand_count);
    
    std::cout << "encoded on demand: " << msg << std::endl;
    
    // put our message into the data_msg for return
    data_msg->CopyFrom(msg);
    ++encode_on_demand_count;
}


void qsize(goby::acomms::protobuf::QueueSize size)
{
    goby_message_qsize = size.size();
}


void handle_receive(const google::protobuf::Message& in_msg)
{
    GobyMessage msg;
    msg.CopyFrom(in_msg);
    
    std::cout << "Received: " << msg << std::endl;
    ++receive_count;
    assert(decode_order.front() == msg.telegram());
    decode_order.pop_front();
}
