import numpy as np

def splitinfo(textdatei):
    datei1 = open(textdatei, 'r')
    totalInfo = datei1.read().split("\n\n")
    datei1.close()
    infolines = totalInfo[0].split("\n")
    info = []
    for i in infolines:
        info += i.split(": ")
    return (info)


testinfo1 = splitinfo("testtabelle.txt")
testinfo2 = splitinfo("testtabelle2.txt")
print(testinfo1)
print(testinfo2)


def compareinfo(info1, info2):
    if info1[7] != info2[7]: return 0  # framerate
    if info1[9] != info2[9]: return 0  # geometry
    return 1


if compareinfo(testinfo1, testinfo2) == 1:
    print("information of the tables are equal")
else:
    print("information of the tables are not equal")

a = np.loadtxt('testtabelle2.txt')
print(a[0][1]) # erster Frame
print(a[-1][1]) # letzter Frame


def testframes(textdatei):  # test if all frames are included
    a = np.loadtxt(textdatei)
    i = 0
    frame = a[0][1]
    while a[i][1] < a[-1][1]:
        if frame == a[i][1]:
            frame = frame + 1
        else:
            if frame < a[i][1]:
                return 0
        i = i + 1
    if frame == a[-1][1]:
        return 1
    return frame


if testframes("testtabelle2.txt") == 1:
    print("all frames are included")
else:
    print("not all frames are included")

infotogether = testinfo1[0] + ": " + testinfo1[1] + "\n" + testinfo1[2] + ":  >=" + max(testinfo1[3], testinfo2[3])
infotogether += "\n" + testinfo1[4] + ": " + testinfo2[5] + "\n" + testinfo1[6] + ": " + testinfo1[7]
i = 8
while i < len(testinfo1) :
    infotogether += "\n" + testinfo1[i] + ": " + testinfo1[i+1]
    i = i+2
infotogether += "\n \n"
print(infotogether)

def addData(trajec1, trajec2):
    a = np.loadtxt(trajec1)
    data = np.array_str(a,precision=5,suppress_small=True)
    data += "\n"
    b = np.loadtxt(trajec2)
    data += np.array_str(b,precision=5,suppress_small=True)
    return data

output_file = open('newtesttabelle.txt','w') # diese Datei wird immer geschrieben, auch wenn die Trajectories nicht zusammengehören
output_file.write(infotogether)
output_file.write(addData("testtabelle.txt","testtabelle2.txt"))
output_file.close()