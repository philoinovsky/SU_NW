#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

#define fi first
#define se second
using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : 
    _output(capacity), _capacity(capacity), szUnasb(0), unread(0), unasb(0), eof2(-1), t({}) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    int idx = max(unasb, static_cast<int>(index));
    int endidx = min(index + data.length(), unread + _capacity);
    if(endidx <= idx) {
        return;
    }
    vector<pair<pair<int,int>, string*>> toInsert;
    for(auto c = t.begin(); c != t.end(); c++) {
        if(idx < c->fi.fi) {
            int start = idx, end = min(c->fi.fi, endidx);
            int len = end - start;
            string str = data.substr(idx-index,len);
            toInsert.push_back({{start,end},&str});
        }
        idx = c->fi.se;
        if(c->fi.se >= endidx) {
            break;
        }
    }
    if(idx < endidx) {
        string str = data.substr(idx-index, endidx-idx);
        toInsert.push_back({{idx, endidx}, &str});
    }
    for(auto i: toInsert){
        t[i.fi] = i.se;
        szUnasb += i.se->length();
    }
    for(auto c = t.begin(); c != t.end() && c->fi.fi == unasb; t.erase(c++)) {
        _output.write(*c->se);
        unasb += c->se->length();
        szUnasb -= c->se->length();
    }
    if(eof) {
        eof2 = static_cast<int>(index + data.length());
    }
    if(eof2 != -1 && unasb == eof2) {
        eof2 = -1;
        _output.end_input();
    }
    // printf("the end: %d %d %d\n", eof, eof2, unasb);
}

size_t StreamReassembler::unassembled_bytes() const { return szUnasb; }

bool StreamReassembler::empty() const { return t.empty(); }