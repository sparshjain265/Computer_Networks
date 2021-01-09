#include <bits/stdc++.h>

using namespace std;
#define MAX_SIZE 10
#define INF 99999
#define T 5

int main() {    
    int N;
    string filename;
    cin >> N >> filename;

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

    cout << "Simulating for " << T << " timesteps.\n\n";

    // for each time step 
    for (int t = 0; t < 5; ++t) {
        cout << "--------------- --------------- T = " << t << " --------------- ---------------" << "\n";

        // for each node 
        for (int x = 0; x < N; x++) {
         cout << "======= Node : " << x << " =========\n";
            //  cout << "Initial Distance vector table... \n";
            //     for (int j = 0; j < N; j++) {
            //         cout << "\t" << D[x][x][j];
            //     } cout << "\n";            

            
                
            for (int y = 0; y < N; y++) {
                if (y == x) continue;
                int min_dist = INF; int d;
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
                            D[x][x][y] = d;
                            R[x][y] = v;    // x --> v --> y : route via v
                        }
                    }
                }

            }







            
           
            cout << "Distance vector table... \n";
                for (int j = 0; j < N; j++) {
                    cout << "\t" << D[x][x][j];
                } cout << "\n";
            // for (int i = 0; i < N; i++) {
            //     for (int j = 0; j < N; j++) {
            //         cout << "\t" << D[x][i][j];
            //     } cout << "\n";
            // }
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

    return 0;
}