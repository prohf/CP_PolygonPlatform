#pragma once
#ifndef CP_TRIAGLE_H
#define CP_TRIAGLE_H
#include <iostream>
using namespace std;
#include <vector>
#include "CP_Polygon.h"

class CP_Triagle {
 public:
  int m_vertices[3];

 public:
  CP_Triagle() {
    m_vertices[0] = 0;
    m_vertices[1] = 0;
    m_vertices[2] = 0;
  }
};  // ��CP_Triagle�������
typedef vector<CP_Triagle> VT_TriagleArray;

class CP_TriagleMesh {
 public:
  VT_TriagleArray m_triagleIDArray;
  CP_Polygon* m_polygon;

 public:
  CP_TriagleMesh() : m_polygon(NULL) {}
  void mb_clear() {
    m_polygon = NULL;
    m_triagleIDArray.clear();
  }
  void mb_buildTriagleMesh(CP_Polygon& pn);
};  // ��CP_TriagleMesh�������

#endif
