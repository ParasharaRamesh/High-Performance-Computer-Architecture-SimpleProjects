import csv
with open("/home/rahul/Desktop/final (2)/datasets/networklogs/training.csv","r") as csvfile:
    linesreader = csv.reader(csvfile, delimiter=',')
    lastattrib={}
    total=0
    for row in linesreader:
        print(".")
        if row[-1] not in lastattrib.keys():
            lastattrib[row[-1]]=[1]
            total+=1
        else:
            lastattrib[row[-1]][0]+=1
            total+=1

    for k,v in lastattrib.items():
        print("##")
        lastattrib[k].append(v[0]/float(total))

    for key in sorted(lastattrib.iterkeys()):
        print "%s: %s" % (key, lastattrib[key])


with open("classcounts.txt","w") as f:
    for k,v in lastattrib.items():
        print("&&")
        f.write(k+","+str(v[0])+","+str(v[1])+"," + "\n")
    print("got the class counts!")
