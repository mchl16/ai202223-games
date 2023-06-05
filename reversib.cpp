#include <bits/stdc++.h>
#include <sys/wait.h>
using namespace std;

constexpr int DEPTH=4;
constexpr int DIFF=DEPTH;

constexpr int gen[8][2]={{1,0},{0,1},{-1,0},{0,-1},{1,1},{-1,1},{-1,-1},{1,-1}};

struct revtable{
    long long data[2];
    
    struct d{
        revtable& r; int i;
        d(revtable r,int i) : r(r),i(i){}

        operator int(){
            return r.get(i);
        }

        void operator=(int val){
            r.set(i,val);    
        }
    };

    int get(int i){
        int a=i>>5,b=(i&31)<<1;
        return (data[a]>>b)&3;
    }

    void set(int i,int val){
        int b=(i&31)<<1,a=i>>5;
        int kaku=3<<b;
        data[a]&=~kaku;
        val<<=b;
        data[a]|=val;
    }

    d operator[](int i){
        return d(*this,i);
    }

    revtable(){
        memset(data,0,2);
    }
};

struct reversi{
    revtable board;
    int color; 

    reversi() : color(1){
        board[27]=2;
        board[28]=1;
        board[35]=1;
        board[36]=2;
    }

    static bool in_bounds(int x,int y){
        if(x<0) return false;
        if(x>=8) return false;
        if(y<0) return false;
        return y<8;
    }

    void place_piece(int field){
        board[field]=color;
        int f1=field>>3,f2=field&7;
        for(const auto& i:gen){
            int g1=f1+i[0],g2=f2+i[1];
            for(;in_bounds(g1,g2) && (board[8*g1+g2]==3-color);g1+=i[0],g2+=i[1]);
            if(!in_bounds(g1,g2) || board[8*g1+g2]!=color) continue;
            for(g1=f1+i[0],g2=f2+i[1];in_bounds(g1,g2) && board[8*g1+g2]==3-color;g1+=i[0],g2+=i[1]){
                board[8*g1+g2]=color;
            }
        }
    }

    bool check_move(int field){
        if(field<0) return true;
        if(board[field]!=0) return false;
        int f1=field>>3,f2=field&7;
        for(const auto& i:gen){
            int g1=f1+i[0],g2=f2+i[1];
            for(;in_bounds(g1,g2) && (board[8*g1+g2]==3-color);g1+=i[0],g2+=i[1]);
            if(in_bounds(g1,g2) && board[8*g1+g2]==color){
                if(g1!=f1+i[0] || g2!=f2+i[1]) return true;
            }
        }
        return false;
    }

    void move(int field){
        if(field>=0) place_piece(field);
        color=3-color;
    }
};

long long rankpos(reversi& pos){
    constexpr int T[]={100,-20,10,5,5,10,-20,100,\
                -20,-50,-2,-2,-2,-2,-50,-20,\
                10,-2,-1,-1,-1,-1,-2,10,\
                5,-2,-1,-1,-1,-1,-2,5,\
                5,-2,-1,-1,-1,-1,-2,5,\
                10,-2,-1,-1,-1,-1,-2,10,\
                -20,-50,-2,-2,-2,-2,-50,-20,\
                100,-20,10,5,5,10,-20,100};
    constexpr int T2[]={0,1,-1};
    long long res=0;
    for(int i=0;i<64;++i) res+=T2[pos.board[i]]*T[i];
    return res;
}



random_device rd;
mt19937 generator(rd());

pair<long long,int> alphabeta(reversi& pos,long long a=-1e18,long long b=1e18,int depth=DEPTH,int last=-1,bool passed=false){
    bool black=(pos.color==1);
    long long val=rankpos(pos);
    int res=last;
    if(depth==0) return {val,res};

    int t[64];
    int j=0;
    for(int i=0;i<64;++i) if(pos.check_move(i)) t[j++]=i;

    if(j==0){
        if(passed){
            constexpr int T2[]={0,1,-1};
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
    int ranks[64];
    for(int i=0;i<j;++i){
        T[t[i]]=pos;
        T[t[i]].move(t[i]);
        ranks[t[i]]=rankpos(T[t[i]]);
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
    cout << "RDY\n";
    for(;;){
        for(int i=0;i<64;++i) cerr << i << ", " << g.board[i] << " "; cerr << "\n";
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
            exit(0);
        }
        else if(s=="UGO"){
            auto x=alphabeta(g).second;
            cout << "IDO " << x%8 << " " << (x>=0 ? x/8 : -1) << "\n";
            g.move(x);
        }
    }
}