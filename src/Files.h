// Working with files

#pragma once

#include <fstream>
#include <filesystem>

#include "Definitions.h"

namespace File {
    namespace fs = std::filesystem;

    struct Data {
        char* data;

        size_t length;

        void destroy() {
            free(data);
        }
    };

    Data Read(cstr path, bool zeroend = false) {
        FILE* file;
        size_t length;
        char* data;
        
        file = fopen(path, "rb");

        if (!file) {
            length = -1;
            goto result;
        }

        // Getting length:
        fseek(file, 0, SEEK_END);
        length = ftell(file)+(zeroend ? 1:0);
        rewind(file);

        data = MALLOC(char, length);
        fread(data, 1, length, file);

        if (zeroend) data[length-1] = '\0';

        fclose(file);

        result:
            Data result;

            result.data = data;
            result.length = length;

            return result;
    }

    Data Read(string path, bool zeroend) {
        return Read(path.c_str(), zeroend);
    }

    bool Write(cstr path, Data data, bool zeroend = false) {
        std::ofstream stream(path);

        if (!stream) return false;

        stream.write(data.data, data.length-(zeroend ? 1:0));

        stream.close();

        return true;
    }

    bool Write(string path, bool zeroend = false) {
        return Write(path.c_str(), zeroend);
    }
}