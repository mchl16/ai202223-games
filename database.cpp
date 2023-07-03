#include <bits/stdc++.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <poll.h>
using namespace std;

constexpr int DEPTH=2;
constexpr int DIFF=DEPTH;
constexpr int COUNT=150;
constexpr int CPUCOUNT=4;

constexpr long long MODULO=5342931457063200LL;

struct rng_gen{
    static constexpr long long MODULO=5342931457063200LL;
    
    random_device rd;
    mt19937_64 generator;
    uniform_int_distribution<long long> distribution;

    rng_gen() : generator(mt19937_64(rd())),distribution(uniform_int_distribution<long long>(0,MODULO-1)){}

    long long get(){
        return distribution(generator);
    }
} rng;

sigjmp_buf env;
bool passed=false;

constexpr int SIZE=3145728;
bool import_data(){
	ifstream f("database.txt",ios::binary);
	if(!f.is_open()) return false;
    cerr << "Importowanie danych..." << endl;

    char buffer[SIZE];
	for(;;){
        f.read(buffer,SIZE);
        int g=f.gcount();
        if((g==0)) break;
        for(int i=0;i<g;i+=24){
            string res(buffer+i,16);
            res+=(buffer[i+16]&3);
            long long l=*(long long*)(buffer+i+16);
            database[res]=l>>3;
        }
    }
    cerr << "Importowanie danych zakonczone" << endl;
	f.close();
	return true;
}

bool export_data(){
    ofstream f("database2.txt");
    if(!f.is_open()) return false;
    cerr << "Eksportowanie danych..." << endl;

    int cnt=0;
    char buffer[SIZE];
	for(const auto& i:database){
        long long l=i.second<<3;
        memcpy(buffer+cnt+16,&l,8);
        memcpy(buffer+cnt,&i.first,16);
        buffer[cnt+16]|=i.first[16];
        cnt+=24;
        if(cnt==SIZE){
            f.write(buffer,SIZE);
            cnt=0;
        }
    }
    f.write(buffer,cnt);

    cerr << "Eksportowanie danych zakonczone" << endl;
	f.close();
	return true;
}


unordered_map<string,long long> database;

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CONNECTIONS 2

constexpr int gen[8][2]={{1,0},{0,1},{-1,0},{0,-1},{1,1},{-1,1},{-1,-1},{1,-1}};

struct rng_gen{
    random_device rd;
    mt19937_64 generator;
    uniform_int_distribution<long long> distribution;

    rng_gen() : generator(mt19937_64(rd())),distribution(uniform_int_distribution<long long>(0,MODULO-1)){}

    long long get(){
        return distribution(generator);
    }
} rng;

struct reversi{
    char board[66];

    reversi(){
        memset(board,3,64);
        board[27]=2;
        board[28]=1;
        board[35]=1;
        board[36]=2;
        board[64]=1;
        board[65]=0;
    }

    static bool in_bounds(int x,int y){
        if(x<0) return false;
        if(x>=8) return false;
        if(y<0) return false;
        return y<8;
    }

    void place_piece(int field){
        assert(board[field]==3);
        assert(field<64);
        board[field]=board[64];
        int f1=field>>3,f2=field&7;
        for(const auto& i:gen){
            int g1=f1+i[0],g2=f2+i[1];
            for(;in_bounds(g1,g2) && (board[8*g1+g2]==3-board[64]);g1+=i[0],g2+=i[1]);
            if(!in_bounds(g1,g2) || board[8*g1+g2]!=board[64]) continue;
            for(g1=f1+i[0],g2=f2+i[1];in_bounds(g1,g2) && board[8*g1+g2]==3-board[64];g1+=i[0],g2+=i[1]){
                assert(8*g1+g2!=64);
                board[8*g1+g2]=board[64];
            }
        }
    }

    bool check_move(int field) const{
        if(field<0) return true;
        if(board[field]!=3) return false;
        int f1=field>>3,f2=field&7;
        for(const auto& i:gen){
            int g1=f1+i[0],g2=f2+i[1];
            for(;in_bounds(g1,g2) && (board[8*g1+g2]==3-board[64]);g1+=i[0],g2+=i[1]);
            if(in_bounds(g1,g2) && board[8*g1+g2]==board[64]){
                if(g1!=f1+i[0] || g2!=f2+i[1]) return true;
            }
        }
        return false;
    }

    void move(int field){
        if(field>=0) place_piece(field);
        board[64]=3-board[64];
    }

    void print(){
        cerr << "Wg reversi4:" << endl;
        for(int i=0;i<8;++i){
            for(int j=0;j<8;++j){
                const char *c=".o#.";
                cerr << c[board[8*i+j]];
            }
            cerr << "\n";
        }
        cerr << endl;
    }
};

int mcts(reversi& pos,bool passed=false){
    int res=0;
    int t[64];
    int j=0;
    for(int i=0;i<64;++i) if(pos.check_move(i)) t[j++]=i;

    if(j==0){
        if(passed){
            constexpr int T2[]={0,1,-1,0};
            res=0;
            for(int i=0;i<64;++i) res+=T2[pos.board[i]];
            if(res>0) res=1;
            else if(res<0) res=-1;

            database[pos.board]+=res;
            return res;
        }
        reversi pos2=pos;
        pos2.move(-1);
        res=mcts(pos2,true);

        database[pos.board]+=res;
        return res;
    }
        
    reversi pos2=pos;
    pos2.move(t[rng.get()%j]);
    res=mcts(pos2);

    database[pos.board]+=res;
    return res;
}

void mcts_driver(reversi& pos){
    long long res=0;
    for(int i=0;i<COUNT;++i){
        reversi pos2=pos;
        mcts(pos2);
    }
}

pair<long long,int> alphabeta(reversi& pos,long long a=-1e18,long long b=1e18,int depth=DEPTH,int last=-1,bool passed=false){
    bool black=(pos.board[64]==1);
    long long val=0;
    int res=last;
    if(depth==0){
        mcts_driver(pos);
        return {database[pos.board],res};
    } 

    int t[64];
    int j=0;
    for(int i=0;i<64;++i) if(pos.check_move(i)) t[j++]=i;

    if(j==0){
        if(passed){
            constexpr int T2[]={0,1,-1,0};
            long long res2=0;
            for(int i=0;i<64;++i) res2+=T2[pos.board[i]];
            if(res2>0) res2=1;
            else if(res2<0) res2=-1;
            return {1e18*res2,-1};
        }
        pos.move(-1);
        return alphabeta(pos,a,b,depth-1,-1,true);
    }

    reversi T[64];
    long long ranks[64];
    for(int i=0;i<j;++i){
        T[t[i]]=pos;
        T[t[i]].move(t[i]);
        ranks[t[i]]=database[T[t[i]].board];
    }
    
    if(black) sort(t,t+j,[&ranks](int a,int b){return ranks[a]>ranks[b];});
    else sort(t,t+j,[&ranks](int a,int b){return ranks[a]<ranks[b];});

    for(int i=0;i<j;++i){
        auto ab=alphabeta(T[t[i]],a,b,depth-1,t[i],false);

        if(black){
            if(ab.first>a){
                a=ab.first;
                val=a;
                res=t[i];
            }
        }
        else{
            if(ab.first<b){
                b=ab.first;
                val=b;
                res=t[i];
            }
        }
        if(a>b) break;
    }
    return {val,res};
}

void make_move(reversi& pos,int usecs){
    itimerval T;
    memset(&T,0,sizeof(T));
    T.it_value.tv_usec=usecs;
    
    setitimer(ITIMER_REAL,&T,nullptr);
    sigsetjmp(env,1);
    
    int x=-1;
    if(passed){
        cerr << "Nie dali mi :C" << endl;
        passed=false;
        bool black=pos.board[64]==1;
        long long res=1e18*(black ? -1 : 1);
        for(int i=0;i<64;++i) {
            if(!pos.check_move(i)) continue;
            reversi pos2=pos;
            pos2.move(i);
            long long d=database[pos2.board];
            if(black && d>res){
                res=d;
                x=i;
            }
            else if(!black && d<res){
                res=d;
                x=i;
            }
        }
    }
    else x=alphabeta(pos).second;

    memset(&T,0,sizeof(T));
    //setitimer(ITIMER_REAL,&T,nullptr);
    cout << "IDO " << x%8 << " " << (x>=0 ? x/8 : -1) << "\n";
    pos.move(x);
}

int main(){
    /* ChatGPT */
    int server_fd, new_socket[MAX_CONNECTIONS] = {0}, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *message = "Server received your message: ";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CONNECTIONS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for client connections...\n");

    // Accepting up to two client connections
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if ((new_socket[i] = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Client %d connected.\n", i + 1);

        valread = read(new_socket[i], buffer, BUFFER_SIZE);
        printf("Client %d message: %s\n", i + 1, buffer);

        // Echoing the received message back to the client
        send(new_socket[i], message, strlen(message), 0);
        send(new_socket[i], buffer, strlen(buffer), 0);
        printf("Message echoed back to Client %d.\n", i + 1);
    }

    /* /ChatGPT */

    for(;;){

    }
}