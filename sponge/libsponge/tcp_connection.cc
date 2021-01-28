#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return tnow - tlast; }

size_t TCPConnection::fill_window_and_send() {
    _sender.fill_window();
    size_t res = 0;
    while(!_sender.segments_out().empty()) {
        TCPSegment seg = _sender.segments_out().front();
        if(_receiver.ackno().has_value()) {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
        }
        _segments_out.push(seg);
        _sender.segments_out().pop();
        res++;
    }
    return res;
}

void TCPConnection::segment_received(const TCPSegment &seg) {
    if(seg.header().rst) {
        active2 = false;
    }
    tlast = tnow;
    _receiver.segment_received(seg);
    _sender.ack_received(seg.header().ackno, seg.header().win);
    if(fill_window_and_send() == 0) {
        _sender.send_empty_segment();
        fill_window_and_send();
    }
}

bool TCPConnection::active() const { return active2; }

size_t TCPConnection::write(const string &data) {
    size_t written = _sender.stream_in().write(data);
    fill_window_and_send();
    return written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    tnow += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
}

void TCPConnection::end_input_stream() { 
    _sender.stream_in().end_input();
    fill_window_and_send();
}

void TCPConnection::connect() {
    fill_window_and_send();
    active2 = true; 
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            active2 = false;
            TCPSegment seg;
            seg.header().rst = true;
            _segments_out.push(seg);
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
