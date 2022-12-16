//
// Created by andyroiiid on 12/15/2022.
//

#include "Files.h"

std::string ReadFile(const std::string &filename) {
    FILE *file = nullptr;

    if (fopen_s(&file, filename.c_str(), "rb")) {
        return {};
    }

    if (fseek(file, 0, SEEK_END)) {
        (void) fclose(file);
        return {};
    }

    const long size = ftell(file);
    if (size < 0) {
        (void) fclose(file);
        return {};
    }

    rewind(file);

    std::string buffer(size, 0);
    if (fread(buffer.data(), sizeof(char), size, file) != size) {
        return {};
    }

    (void) fclose(file);

    return buffer;
}
