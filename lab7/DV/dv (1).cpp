#include <bits/stdc++.h>

using namespace std;
#define MAX_SIZE 10
#define INF 99999
#define T 5

struct edge {
    int from;
    int to;
    int wt;
    edge(){}
    edge(int a, int b, int c) : from{a}, to{b}, wt{c} {}
};


int main() {
    int N;
    string filename;
    cin >> N >> filename;

    vector<edge> edge_vec;

    int C[N][N];
    int D[N][N][N];
    int R[N][N];

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            R[i][j] = -1;
            for (int k = 0; k < N; k++) {
                D[i][j][k] = INF;
            }
        }
    }


    // reading the cost matrix
    ifstream fin(filename, ifstream::in);
    int cost;
    for (int x = 0; x < N; x++) {
        for (int y = 0; y < N; y++) {
            fin >> cost;
            
            if (cost >= 0) {
                C[x][y] = cost;
            } else {
                C[x][y] = INF;
            }

            if (x!=y) edge_vec.push_back(edge(x,y,C[x][y]));
        }
    }

    // initializing each node form 0 to N-1
    for (int n = 0; n < N; n++) {
        // for neighbors
        int x = n;
        for (int y = 0; y < N; y++) {
            D[n][x][y] = C[x][y];
        }        
    }

    bool convrg = false;
    while(not convrg) {
        convrg = true;

        // INFORMATION EXCHANGE BETWEEN RANDOM EDGE
        random_shuffle(edge_vec.begin(), edge_vec.end());

        for (auto &e : edge_vec) {
            int x = e.from;
            int y = e.to;
            cout << "Relaxing edge: " << x <<" --> " << y << endl;
            int min_dist = INF; int d; int route_through;
            for (int v = 0; v < N; v++) {
                if (C[x][v] > 0 && C[x][v] < INF && v!=x) { // if v is neighbor of x
                    // receive the distance vector of neighbor v
                    for (int i = 0; i < N; i++) {
                        D[x][v][i] = D[v][v][i];
                    }

                    // update x's distance vector
                    d = C[x][v] + D[x][v][y];
                    if (min_dist > d){
                        min_dist = d;
                        route_through = v;    // x --> v --> y : route via v
                    }
                }
            }

            if (min_dist != D[x][x][y]){
                D[x][x][y] = min_dist;
                R[x][y] = route_through;
                convrg = false;
            }
            
            cout << "Distance vector table... \n";
            for (int j = 0; j < N; j++) {
                cout << "\t" << D[x][x][j];
            } cout << "\n";

            cout << endl;

            cout << "Routing table:\n";
            cout << "From node " << x << " to " << "\n";

            for (int y = 0; y < N; y++) {
                cout << "............... node " << y ;
                if (R[x][y] != -1) {
                    cout << " via node " << R[x][y] << "\n";
                } else {
                    cout << " by direct connection." << "\n";
                }
            } cout << endl;

        }
    }

    cout << "=============== FINAL DISTANCE VECTOR TABLE ==============\n";

    for (int x = 0; x < N; x++) {
        cout << "Distance vector table for node : "<< x <<"... \n";
            for (int j = 0; j < N; j++) {
                cout << "\t" << D[x][x][j];
            } cout << "\n";

            cout << endl;

            cout << "Routing table:\n";
            cout << "From node " << x << " to " << "\n";

            for (int y = 0; y < N; y++) {
                cout << "............... node " << y ;
                if (R[x][y] != -1) {
                    cout << " via node " << R[x][y] << "\n";
                } else {
                    cout << " by direct connection." << "\n";
                }
            } cout << endl;

    }
    return 0;
}