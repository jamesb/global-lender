#pragma once

// Prints the free and used heap sizes to the app log, with or without an additional c-string argument to append.
#define HEAP_LOG_0()       APP_LOG(APP_LOG_LEVEL_DEBUG, "Free Heap: %zu b, Used Heap: %zu b", heap_bytes_free(), heap_bytes_used()) 
#define HEAP_LOG_1(str)    if (str != NULL) APP_LOG(APP_LOG_LEVEL_DEBUG, "Free Heap: %zu b, Used Heap: %zu b -- %s", heap_bytes_free(), heap_bytes_used(), str) 
#define GET_MACRO(_0, _1, NAME, ...) NAME
#define HEAP_LOG(...) GET_MACRO(_0, ##__VA_ARGS__, HEAP_LOG_1, HEAP_LOG_0)(__VA_ARGS__)


void textLayer_stylize(TextLayer*, const GColor, const GColor, const GTextAlignment, const GFont); 


bool strxcpy(char buffer[], size_t bufsize, const char source[], const char readable[]);
