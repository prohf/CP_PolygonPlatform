#include "stdafx.h"

#include "CP_Triangle.h"

void CP_TriagleMesh::mb_buildTriagleMesh(CP_Polygon& pn) {
  m_polygon = &pn;
  int n = pn.m_pointArray.size();
  if (n < 3) return;
  m_triagleIDArray.resize(1);
  m_triagleIDArray[0].m_vertices[0] = 0;
  m_triagleIDArray[0].m_vertices[0] = 1;
  m_triagleIDArray[0].m_vertices[0] = 2;
}  // ��CP_TriagleMesh�ĳ�Ա����mb_buildTriagleMesh����
