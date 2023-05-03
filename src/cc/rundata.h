#pragma once
#include "pch.h"
#include "guy.h"

struct Rundata {
    GuyGroup guygroup;
    Guy player;
    static Rundata& get() {
        static Rundata rd;
        return rd;
    }
};