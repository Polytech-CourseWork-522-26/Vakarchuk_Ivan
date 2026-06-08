#ifndef IDGENERATOR_H
#define IDGENERATOR_H
#include <cstring>
#include <string>

class IDGenerator {
public:
    static std::string generate(const std::string& fileName);
};

#endif