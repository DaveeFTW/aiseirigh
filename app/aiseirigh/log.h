#pragma once

#include <lk/debug.h>
#include <stdio.h>

#define LOG(...) printf(__VA_ARGS__); printf("\n")
#define FATAL(...) panic(__VA_ARGS__)