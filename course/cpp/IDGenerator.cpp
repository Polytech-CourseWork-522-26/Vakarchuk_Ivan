#include "IDGenerator.h"
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

string IDGenerator::generate(const string& fileName) {
    int id = 0;

    ifstream in(fileName);
    if (in) in >> id;
    in.close();

    id++;

    ofstream out(fileName);
    out << id;
    out.close();

    stringstream ss;
    ss << setw(5) << setfill('0') << id;

    return ss.str();
}