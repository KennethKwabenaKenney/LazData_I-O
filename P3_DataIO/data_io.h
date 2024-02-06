#pragma once
#pragma comment(lib, "laszip.lib")
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#if defined(_MSC_VER) && \
    (_MSC_FULL_VER >= 150000000)
#define LASCopyString _strdup
#else
#define LASCopyString strdup
#endif
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>
#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <laszip/laszip_api.h>

using namespace std;

// The Ascii point could structure structure as asciiPTC
typedef struct asciiPointCloud
{
	double x, y, z;
    unsigned short r, g, b, intensity;
} asciiPTC;

typedef struct objVertices
{
    double Vx, Vy, Vz;
} objVtx;

typedef struct objFacets
{
    int Fx, Fy, Fz;
} objFct;

// The Laz/Las point cloud structure as LasPoint
typedef struct lasPoint
{
    double x, y, z;
    unsigned short intensity;
    unsigned short r, g, b;
} LasPoint;

// Data structure to store LAS/ASCII point cloud statistics
typedef struct PointStatistics {
    size_t numPoints;

    double minX, maxX;
    double minY, maxY;
    double minZ, maxZ;
};

vector<asciiPTC> read_asciiTxt(string file_in);
void Write_asciiTxt(string file_o, vector<asciiPTC>& pts);

vector<LasPoint> read_LasFile(string file_in);
void write_LasFile(string file_o, const vector<LasPoint>& lasPoints);

pair<vector<objVtx>, vector<objFct>> read_obj(string file_in);
void write_obj(string file_o, const pair<vector<objVtx>, vector<objFct>>& result);

//vector<objPTC> read_obj(string file_in);
//void write_obj(string file_o, vector<objPTC>& pts);

string add_extension(string path, string new_ext, bool behind_name_or_ext);
string remove_extension(string path);
string get_extension(string path);

template <typename PointType>
PointStatistics computeStatistics(const vector<PointType>& points);

void printStatistics(const PointStatistics& stats);