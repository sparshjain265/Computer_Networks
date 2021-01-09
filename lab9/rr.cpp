#include <bits/stdc++.h>

using namespace std;

struct packet {
    float arr;
    int id;
    int qid;
    int len;

    packet(float _arr, int _id, int _qid, int _len)
    : arr{_arr}, id{_id}, qid{_qid}, len{_len} {}
};

// struct buffer {
//     vector<queue<packet>> q;

//     buffer(int num_queues = 4)
//     : q(num_queues), current_served{-1} {}

//     void push(packet p){
//         q[p.qid - 1].push(p);
//     }

//     packet pop
// };

int get_serve_idx( vector <queue<packet>> &q, int prev_served_idx, int curr_time){
    float arr = INT_MAX;
    int serve_idx = 0;

    for (int k = 1; k <= 4; k++) {
        int idx = (prev_served_idx - 1 + k) % 4;
        packet &p = q[idx].front();

        if (curr_time > p.arr) {
            serve_idx = p.qid;
            arr = p.arr;
        }
    }

    // for (auto &itr : q) {
    //     if (itr.empty()) continue;

    //     packet &p = itr.front();
        
    //     if (arr > p.arr and curr_time > p.arr) {
    //         serve_idx = p.qid;
    //         arr = p.arr;
    //     }
    // }

    if (serve_idx) return serve_idx;

    arr = INT_MAX;
    serve_idx = 0;

    for (int k = 1; k <= 4; k++) {
        int idx = (prev_served_idx - 1 + k) % 4;
        packet &p = q[idx].front();

        if (arr > p.arr) {
            serve_idx = p.qid;
            arr = p.arr;
        }
    }

    // for (auto &itr : q) {
    //     if (itr.empty()) continue;

    //     packet &p = itr.front();

    //     if (arr > p.arr) {
    //         serve_idx = p.qid;
    //         arr = p.arr;
    //     }
    // }

    return serve_idx;
}



int main(int argc, char *argv[]) {
    float rate = atof(argv[1]);

    float arr;      // arrival time
    int id;         // packet ID
    int qid;        // queue id
    int len;        // packet length

    vector<packet> plist;

    while(cin >> arr >> id >> qid >> len){
        plist.push_back(packet(arr, id, qid, len));
    }

    vector <queue<packet>> q(4);

    
    for (auto &p : plist) {
        q[p.qid-1].push(p);
    }

    float curr_time = 0.0;
    int serve_idx;
    int prev_served_idx = 0;

    while(serve_idx = get_serve_idx(q, prev_served_idx, curr_time)) {
        packet &p = q[serve_idx - 1].front();

        // service time
        float dt = p.len / rate;

        // waiting time
        // float waiting_time = int(p.arr < curr_time) * (curr_time - p.arr);

        if (p.arr >= curr_time) {
            curr_time = p.arr;
        }

        // transmission time
        curr_time += dt;

        printf("%0.2f %d\n", curr_time, p.id);
        q[serve_idx - 1].pop();
        prev_served_idx = serve_idx;
    }


    return 0;
}