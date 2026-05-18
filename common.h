#pragma once
#include "tgaimage.h"
#include <unordered_map>

using namespace std;

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void drawLine(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color);

void drawLineRecordRowRange(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color, unordered_map<int, pair<int, int>>& row_range);

std::tuple<int, int> project(float x, float y, float z, int width, int height);

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy);