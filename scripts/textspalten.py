def textspalten(textdatei):
    datei1 = open(textdatei, 'r')
    KopfKörper = datei1.read().split("\n\n")
    print(KopfKörper[0])
    informationzeilen = KopfKörper[0].split("\n")
    infomation = []
    for i in informationzeilen:
        infomation += i.split(": ")
    return (infomation)