#pragma once
#include "pch.h"
#include "guy.h"

struct CCRundata {
    GuyGroup guygroup;
    Guy player;
    static CCRundata& get() {
        static CCRundata rd;
        return rd;
    }
};