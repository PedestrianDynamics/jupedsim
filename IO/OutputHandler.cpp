#include "OutputHandler.h"

void OutputHandler::write(string str) {
}

void STDIOHandler::write(string str) {
    if (this != NULL)
        cout << str << endl;
}

FileHandler::FileHandler(const char *fn) {
    pfp.open(fn);
    if (!fn) {
        char tmp[CLENGTH];
        sprintf(tmp, "Error!!! File [%s] could not be opened!", fn);
        cerr << tmp << endl;
        exit(0);
    }
}

FileHandler::~FileHandler() {
    pfp.close();
}

void FileHandler::write(string str) {
    if (this != NULL) {
        pfp << str << endl;
        pfp.flush();
    }
}

TraVisToHandler::TraVisToHandler() {
    client = new TraVisToClient();
    brokentags.push_back("<trajectoriesDataset>");
    brokentags.push_back("</trajectoriesDataset>");
    brokentags.push_back("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
}

TraVisToHandler::~TraVisToHandler(){
	delete client;
}

void TraVisToHandler::write(string str) {

    vector<string>::iterator str_it;

    //There are a few broken tags which need to be checked for and removed.
    for (str_it = brokentags.begin(); str_it != brokentags.end(); ++str_it) {
        int tagstart = str.find(*str_it);
        if (tagstart != (int) string::npos) {
            str.erase(str.begin() + tagstart, str.begin() + tagstart + (*str_it).size());
        }
    }
    client->sendData(str.c_str());
}
