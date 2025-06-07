#pragma once
#include <stddef.h>
// TODO: better thing that can stream into a buffer or something.
// I don't give a fuck for now so :/
const char* read_entire_file(const char* path, size_t* size);
void remove_carrige_return(char* content);
