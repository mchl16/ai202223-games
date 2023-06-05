#include <bits/stdc++.h>
#include <sys/wait.h>
#include <poll.h>
using namespace std;

//Do zrobienia: program ma po staremu czytać tylko long longa z policzonym ruchem

constexpr int DEPTH=2;
constexpr int DIFF=DEPTH;
constexpr int COUNT=100;
constexpr int CPUCOUNT=4;

constexpr int PACKAGE_LEN=73;
constexpr long long MODULO=5342931457063200LL;

pollfd fds[4];

struct trie{
    struct trie_node{
        union{
            long long val;
            char meta[4];
        } u;
        trie_node *next[3];

        trie_node(){
            memset(this,0,32);
        }
    } root;
    long long& operator[](const string& s);
};

struct allocator{
    static constexpr int SIZE=sizeof(trie::trie_node);
    static constexpr int MAX=32768;
    int state=MAX-1;
    long long allocs=0;
    char *data=nullptr;

    trie::trie_node *allocate(){
        if(state==MAX-1){
            state=-1;
            data=new char[MAX*SIZE];
        }
        ++allocs;
        assert(allocs<250000000);
        return (trie::trie_node*)(data+SIZE*(++state));
    }

} al;

long long& trie::operator[](const string& s){
    trie_node *t=&(this->root);
    for(int i=0;i<s.size();++i){
        // cerr << " " << t << flush;
        if(!(t->next[s[i]-1])){
            auto ptr=al.allocate(); //new trie_node();
            t->next[s[i]-1]=ptr;
        }
        t=t->next[s[i]-1];
    }
    return t->u.val;
}

trie database;

// unordered_map<string,long long> database;

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

// bool import_data(){
// 	ifstream f("database.txt");
// 	string str;
// 	int data;
// 	if(!f.is_open()) return false;
//     int i=0;
// 	while(f >> str >> data){
//         if((i&1048575)==0) cerr << endl <<  i << "???" << endl; ++i;
//         // database[str]=data;
//         database[str]=data;
//     }
//     // assert(database[str]==database2[str]);
// 	f.close();
// 	return true;
// }

// bool export_data(){
//     cerr << al.allocs << endl;
// 	ofstream f("database2.txt",ios::binary);
// 	if(!f.is_open()) return false;
//     int j=0;

//     vector<pair<trie::trie_node*,pair<char,char>>> v;
//     v.push_back({&database.root,{0,0}});
//     while(!v.empty()){
//         if((j&1048575)==0) cerr << endl <<  j << "?!" << endl; ++j;
//         auto n=v[v.size()-1];
//         if(n.second.second==65){
//             long long l=(n.first->val)<<3;
//             bool a=n.first->next[0];
//             bool b=n.first->next[1];
//             bool c=n.first->next[2];
//             if(a) l|=4;
//             if(b) l|=2;
//             if(c) l|=1;
//             f << l;
//             v.pop_back();
//             continue;
//         }
//         if(n.second.first<3) v.push_back({n.first->next[n.second.first++],{0,n.second.second+1}});
//         else v.pop_back();
//     }
// 	f.close();
//     cerr << "Koniec" << endl;
//     system("cp database2.txt database.txt");
// 	return true;
// }

void on_exit(){
    // export_data();
}

void sigterm_handler(int s){
    on_exit();
    exit(s);
}

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
        board[field]=board[64];
        int f1=field>>3,f2=field&7;
        for(const auto& i:gen){
            int g1=f1+i[0],g2=f2+i[1];
            for(;in_bounds(g1,g2) && (board[8*g1+g2]==3-board[64]);g1+=i[0],g2+=i[1]);
            if(!in_bounds(g1,g2) || board[8*g1+g2]!=board[64]) continue;
            for(g1=f1+i[0],g2=f2+i[1];in_bounds(g1,g2) && board[8*g1+g2]==3-board[64];g1+=i[0],g2+=i[1]){
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
    sort(t,t+j,[&ranks](int a,int b){return ranks[a]>ranks[b];});

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

int main(){
    string s;
    reversi g;
    
    new (&rng) rng_gen();
    
    // import_data();
    // exit(0);

    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs,SIGTERM);

    struct sigaction sa;
    sa.sa_handler=sigterm_handler;
    sigaction(SIGTERM,&sa,NULL);

    cout << "RDY\n";
    for(;;){
        cin >> s;
        if(s=="HEDID"){
            float a,b;
            int c,d;
            cin >> a >> b >> c >> d;
            g.move(8*d+c);
            auto x=alphabeta(g).second;
            cout << "IDO " << x%8 << " " << (x>=0 ? x/8 : -1) << "\n";
            g.move(x);
        }
        else if(s=="ONEMORE"){
            g=reversi();
            cout << "RDY\n";
        }
        else if(s=="BYE"){
            on_exit();
            exit(0);
        }
        else if(s=="UGO"){
            auto x=alphabeta(g).second;
            cout << "IDO " << x%8 << " " << (x>=0 ? x/8 : -1) << "\n";
            g.move(x);
        }
    }

    // on_exit();
}