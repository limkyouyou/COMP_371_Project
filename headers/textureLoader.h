#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <string>

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

#endif