with open("database.txt",encoding='iso8859-1') as f:
    zero,total=0,0
    for i in f:
        a,b=i.split()
        b=int(b)
        assert len(a)==65
        total+=1
        if b==0: zero+=1
    print(zero,"/",total)

        