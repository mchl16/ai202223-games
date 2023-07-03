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

struct trie{
    struct trie_node{
        union{
            long long val;
            char meta[8];
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

// trie database;