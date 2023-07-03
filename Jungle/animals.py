from copy import deepcopy as cp
from random import randint as rng,choice
from sys import stderr,argv
import os

debug=len(argv)>=2 and argv[1]=='debug'

cnt=0

class animal_chess:
    def __init__(self):
        self.board=[None for i in range(63)]
        self.turn=1
        s="l.....t.d...c.r.j.w.e"
        for i in range(21):
            if s[i]=='.': continue
            self.board[i]=self.piece(self,".rcdwjtle".index(s[i]),2,i%7,i//7)
            self.board[62-i]=self.piece(self,".rcdwjtle".index(s[i]),1,6-i%7,8-i//7)
    
    # def __deepcopy__(self,memo):
    #     result=object.__new__(self.__class__)
    #     result.board=[i for i in self.board]
    #     result.turn=self.turn
    #     return result


    def is_trap(n):
        return n in (2,4,10,52,58,60)
    
    def is_lake(n):
        return n//7 in (3,4,5) and n%7 in (1,2,4,5)
    
    def is_shithole(n,c):
        return (59,3)[c-1]==n
    
    def __getitem__(self,n):
        return self.board[n]
    
    def move(self,x1,y1,x2,y2):
        if self.board[x1+7*y1].can_move(x2,y2):
            self.board[x1+7*y1].move(x2,y2)
            self.board[x2+7*y2]=self.board[x1+7*y1]
            self.board[x1+7*y1]=None
            self.turn=3-self.turn
        if self[3] is not None and self[3].color==1: return 1
        elif self[59] is not None and self[59].color==2: return -1
        return 0

    def get_color(self,x,y):
        return 0 if self[x+7*y] is None else self[x+7*y].color
    
    def print_board(self):
        S=".rcdwjtle"
        for i in range(63):
            cs="\033[1m"
            s=''
            
            if animal_chess.is_trap(i): 
                s='*'
            if animal_chess.is_lake(i):
                cs+="\033[48;2;0;%d;255m" % (128 if i&1==1 else 64)
            else:
                cs+="\033[48;2;0;%d;0m" % (255 if i&1==1 else 192)
            if self[i] is None: 
                if animal_chess.is_trap(i): 
                    s='*'
                    cs+="\033[38;2;0;0;0m"
                elif animal_chess.is_shithole(i,1):
                    s='#'
                    cs+="\033[38;2;255;0;255m"
                elif animal_chess.is_shithole(i,2):
                    s='#'
                    cs+="\033[38;2;0;255;255m"
                else: 
                    s=' '
                    cs+="\033[38;2;255;255;255m"

            else:
                cs+="\033[38;2;0;0;0m"
                s=S[self[i].power]
                if self[i].color==2: s=s.upper()
            print("%s%s" % (cs,s),end='',file=stderr)
            if i%7==6: print("\033[0m",file=stderr)
        print(file=stderr)

    class piece:
        def __init__(self,board,power,c,x,y):
            self.board=board
            self.color=c
            self.x,self.y=x,y
            self.temppower=power
            self.power=power

        def can_move(self,x,y):
            if 0<=x<7 and 0<=y<9: pass
            else: return False

            if self.x==x:
                if self.y-y in (-1,1): pass
                elif self.power not in (6,7): return False
                else:
                    for i in range(min(y,self.y)+1,max(y,self.y)):
                        if self.board.get_color(x,y)==3-self.color: return False

            elif self.y==y:
                if self.x-x in (-1,1): pass
                elif self.power not in (6,7): return False
                else:
                    for i in range(min(x,self.x)+1,max(x,self.x)):
                        if self.board.get_color(x,y)==3-self.color: return False

            else: return False

            if self.board[x+7*y] is not None: 
                if self.board[x+7*y].color==self.color: return False
                if self.board[x+7*y].temppower>self.temppower:
                    if (self.board[x+7*y].power,self.power)!=(8,1): return False

            if animal_chess.is_lake(x+7*y) and self.power!=1: return False
            if animal_chess.is_lake(self.x+7*self.y):
                if not animal_chess.is_lake(x+7*y) and self.board[x+7*y] is not None: return False
            
            return not animal_chess.is_shithole(x+7*y,self.color)

        def move(self,x,y):
            self.x,self.y=x,y
            if animal_chess.is_trap(x+7*y): self.temppower=0
            else: self.temppower=self.power
            return True

def gen_moves(pos):
    for i in range(63):
        if pos[i] is None: continue
        if pos.get_color(i%7,i//7)!=pos.turn: continue

        vx,vy=1,0
        for _ in range(4):
            x,y=i%7,i//7
            x+=vx
            y+=vy
            if 0<=x<7 and 0<=y<9: pass
            else: 
                vx,vy=-vy,vx
                continue
            
            if pos[i].power in [6,7]:
                while animal_chess.is_lake(x+7*y):
                    x+=vx
                    y+=vy

            if pos[i].can_move(x,y): yield(i%7,i//7,x,y)

            vx,vy=-vy,vx


def apply_move(pos,move):
    return pos.move(move[0],move[1],move[2],move[3])

def analyze2(pos):
    while True:
        g=[i for i in gen_moves(pos)]
        if len(g)==0: return 0
        res=apply_move(pos,choice(g))
        if res!=0: return res

def distance(x1,y1,color):
    return abs(x1-3)+(8-y1 if color==2 else y1)

mode=os.cpu_count()

def analyze(pos):
    t=[i for i in gen_moves(pos)]
    u=[]

    L=len(t)
    L//=mode

    def analyze3(pos,t):
        u=[0 for _ in t]
        
        for i,j in enumerate(t):
            pos2=cp(pos)
            apply_move(pos2,j)
            for _ in range(250):
                u[i]+=analyze2(pos2)
        return u

        #pos2.print_board()
    pipes=[os.pipe() for _ in range(mode)]
    pids=[0 for _ in range(mode)]
    
    for k in range(mode):
        P=pipes[k]
        pids[k]=os.fork()
        if pids[k]==0:
            res2=0
            if k==mode-1:
                res2=analyze3(pos,t[k*L::])
            else:
                res2=analyze3(pos,t[k*L:(k+1)*L:])
            os.close(P[0])
            os.write(P[1],str(res2).encode())
            os.close(P[1])
            exit(0)

        os.close(P[1])
        
    for k in range(mode): os.waitpid(pids[k],0)
    for k in range(mode): u+=eval(os.read(pipes[k][0],213769420).decode())
    for k in range(mode): os.close(pipes[k][0])
    print(u,file=stderr)
        
    x,r=0,-1e18
    for i,j in enumerate(u):
        if(debug): print(j,file=stderr)
        if j>r: x,r=i,j
        elif j==r:
            if distance(t[i][2],t[i][3],pos.turn)<distance(t[x][2],t[x][3],pos.turn):
                x,r=i,j

    return t[x]        


class game:
    def __init__(self):
        self.setup()
        
    def setup(self):
        self.t=animal_chess()
        print("RDY")
    
    def play(self):
        while True:
            xd=input().split()
            cmd,args=xd[0],list(map(int,xd[3::]))

            if cmd=="HEDID":
                self.t.move(*args)
                # self.t.print_board()
                x=analyze(self.t)
                print("IDO",*x)
                self.t.move(*x)
                # self.t.print_board()
            elif cmd=="ONEMORE":
                self.setup()
            elif cmd=="BYE":
                return
            elif cmd=="UGO":
                x=analyze(self.t)
                print("IDO",*x)
                self.t.move(*x)
                # self.t.print_board()

game().play()
    
    
    


