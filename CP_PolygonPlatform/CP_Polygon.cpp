#include "stdafx.h"
#include "CP_Polygon.h"

void gb_distanceMinPointLoop(double& d, int& idRegion, int& idLoop,
                             CP_Point& pt, CP_Polygon& pn) {
  d = 0.0;
  idRegion = -1;
  idLoop = -1;
  int nr = pn.m_regionArray.size();
  int i, j, k, nl, nv, v1, v2;
  double dt;
  for (i = 0; i < nr; i++) {
    nl = pn.m_regionArray[i].m_loopArray.size();
    for (j = 0; j < nl; j++) {
      nv = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray.size();
      for (k = 0; k < nv; k++) {
        v1 = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray[k];
        if (k == nv - 1)
          v2 = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray[0];
        else
          v2 = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray[k + 1];
        dt = gb_distancePointSegment(pt, pn.m_pointArray[v1],
                                     pn.m_pointArray[v2]);
        if ((idLoop == -1) || (d > dt)) {
          d = dt;
          idRegion = i;
          idLoop = j;
        }  // if结束
      }    // for(k)结束
    }      // for(j)结束
  }        // for(i)结束
}  // 函数gb_distanceMinPointPolygon结束

void gb_distanceMinPointPolygon(double& d, int& id, CP_Point& pt,
                                CP_Polygon& pn) {
  d = 0.0;
  id = -1;
  int n = pn.m_pointArray.size();
  if (n <= 0) return;
  d = gb_distancePointPoint(pt, pn.m_pointArray[0]);
  id = 0;
  int i;
  double dt;
  for (i = 1; i < n; i++) {
    dt = gb_distancePointPoint(pt, pn.m_pointArray[i]);
    if (dt < d) {
      d = dt;
      id = i;
    }  // if结束
  }    // for结束
}  // 函数gb_distanceMinPointPolygon结束

double gb_distancePointPoint(CP_Point& p1, CP_Point& p2) {
  double dx = p1.m_x - p2.m_x;
  double dy = p1.m_y - p2.m_y;
  double d2 = dx * dx + dy * dy;
  double d = sqrt(d2);
  return d;
}  // 函数gb_distancePointPoint结束

double gb_distancePointSegment(CP_Point& pt, CP_Point& p1, CP_Point& p2) {
  double dx0 = p2.m_x - p1.m_x;
  double dy0 = p2.m_y - p1.m_y;
  double dx1 = pt.m_x - p1.m_x;
  double dy1 = pt.m_y - p1.m_y;
  double dx2 = pt.m_x - p2.m_x;
  double dy2 = pt.m_y - p2.m_y;
  double d1 = dx1 * dx1 + dy1 * dy1;
  double d2 = dx2 * dx2 + dy2 * dy2;
  double d01 = dx1 * dx0 + dy1 * dy0;
  double d02 = -dx2 * dx0 - dy2 * dy0;
  double d, d0;
  if ((d01 > 0) && (d02 > 0)) {
    d0 = dx0 * dx0 + dy0 * dy0;
    d = d01 * d01 / d0;  // 如果计算溢出，如何处理?
    d = d1 - d;
    d = sqrt(d);
    return d;
  }  // if结束
  if (d1 > d2)
    d = d2;
  else
    d = d1;
  d = sqrt(d);
  return d;
}  // 函数gb_distancePointPoint结束

void gb_getIntArrayPointInPolygon(VT_IntArray& vi, CP_Polygon& pn, CP_Point& p,
                                  double eT) {
  int i, n;
  double d;
  n = pn.m_pointArray.size();
  for (i = 0; i < n; i++) {
    d = gb_distancePointPoint(p, pn.m_pointArray[i]);
    if (d <= eT) {
      vi[i] = i;
    }  // if结束
  }    // for(i)结束
}  // 函数gb_getIntArrayPointInPolygon结束

bool gb_findPointInLoop(CP_Polygon& pn, int& idRegion, int& idLoop,
                        int& idPointInLoop, int pointInPolygon) {
  idRegion = 0;
  idLoop = 0;
  idPointInLoop = 0;
  int nr, nL, nv;
  int i, j, k;
  nr = pn.m_regionArray.size();
  for (i = 0; i < nr; i++) {
    nL = pn.m_regionArray[i].m_loopArray.size();
    for (j = 0; j < nL; j++) {
      nv = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray.size();
      for (k = 0; k < nv; k++) {
        if (pn.m_regionArray[i].m_loopArray[j].m_pointIDArray[k] ==
            pointInPolygon) {
          idRegion = i;
          idLoop = j;
          idPointInLoop = k;
          return true;
        }  // if结束
      }    // for(nv)结束
    }      // for(nL)结束
  }        // for(nr)结束
  return false;
}  // 函数gb_findPointInLoop结束

// 这里假设所有的条件都成立，即函数内部不判断输入的合法性
void gb_insertPointInPolygon(CP_Polygon& pn, int& idRegion, int& idLoop,
                             int& idPointInLoop, CP_Point& newPoint) {
  int nv = pn.m_pointArray.size();
  pn.m_pointArray.push_back(newPoint);
  pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray.insert(
      pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray.begin() +
          idPointInLoop + 1,
      nv);
}  // 函数gb_findPointInLoop结束

void gb_intArrayInit(VT_IntArray& vi, int data) {
  int n = vi.size();
  int i;
  for (i = 0; i < n; i++) vi[i] = data;
}  // 函数gb_intArrayInit结束

void gb_intArrayInitLoop(VT_IntArray& vi, CP_Polygon& pn, int idRegion,
                         int idLoop, double eT) {
  int i, v;
  int n = pn.m_pointArray.size();
  vi.resize(n);
  n = pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray.size();
  for (i = 0; i < n; i++) {
    v = pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray[i];
    vi[v] = v;
  }  // for结束
  gb_intArrayInitPointSame(vi, pn, eT);
}  // 函数gb_intArrayInitLoop结束

void gb_intArrayInitPoint(VT_IntArray& vi, CP_Polygon& pn, int v, double eT) {
  int n = pn.m_pointArray.size();
  if (n <= 0) {
    vi.clear();
    return;
  }  // if结束
  vi.resize(n);
  int i;
  double d;
  for (i = 0; i < n; i++) {
    if (i == v)
      vi[i] = i;
    else {
      d = gb_distancePointPoint(pn.m_pointArray[i], pn.m_pointArray[v]);
      if (d <= eT)
        vi[i] = i;
      else
        vi[i] = -1;
    }  // if/else结束
  }    // for结束
}  // 函数gb_intArrayInitPoint结束

void gb_intArrayInitPointSame(VT_IntArray& vi, CP_Polygon& pn, double eT) {
  int i, j, n;
  double d;
  n = vi.size();
  if (n <= 0) return;
  for (i = 0; i < n; i++) {
    if (vi[i] >= 0) {
      for (j = 0; j < n; j++) {
        if (vi[j] < 0) {
          d = gb_distancePointPoint(pn.m_pointArray[i], pn.m_pointArray[j]);
          if (d <= eT) vi[j] = j;
        }  // if结束
      }    // for(j)结束
    }      // if结束
  }        // for(i)结束
}  // 函数gb_intArrayInitPointSame结束

void gb_intArrayInitPolygon(VT_IntArray& vi, CP_Polygon& pn) {
  int i;
  int n = pn.m_pointArray.size();
  vi.resize(n);
  for (i = 0; i < n; i++) vi[i] = i;
}  // 函数gb_intArrayInitPolygon结束

void gb_intArrayInitPolygonSamePoint(VT_IntArray& vr, CP_Polygon& pr,
                                     VT_IntArray& vs, CP_Polygon& ps,
                                     double eT) {
  int i, j;
  int n0, n1;
  double da;
  n1 = pr.m_pointArray.size();
  if (n1 <= 0) {
    vr.clear();
    return;
  }  // if结束
  vr.resize(n1);
  gb_intArrayInit(vr, -1);
  n0 = ps.m_pointArray.size();
  for (i = 0; i < n0; i++) {
    if (vs[i] < 0) continue;
    for (j = 0; j < n1; j++) {
      if (vr[j] < 0) {
        da = gb_distancePointPoint(ps.m_pointArray[i], pr.m_pointArray[j]);
        if (da <= eT) vr[j] = j;
      }  // if结束
    }    // for(j)结束
  }      // for(i)结束
}  // 函数gb_intArrayInitPolygonSamePoint结束

void gb_intArrayInitRegion(VT_IntArray& vi, CP_Polygon& pn, int idRegion,
                           double eT) {
  int i, j, nr, v;
  int n = pn.m_pointArray.size();
  vi.resize(n);
  nr = pn.m_regionArray[idRegion].m_loopArray.size();
  for (i = 0; i < nr; i++) {
    n = pn.m_regionArray[idRegion].m_loopArray[i].m_pointIDArray.size();
    for (j = 0; j < n; j++) {
      v = pn.m_regionArray[idRegion].m_loopArray[i].m_pointIDArray[j];
      vi[v] = v;
    }  // for(j)结束
  }    // for(i)结束
  gb_intArrayInitPointSame(vi, pn, eT);
}  // 函数gb_intArrayInitRegion结束

void gb_moveLoop(CP_Polygon& pn, int idRegion, int idLoop, double vx,
                 double vy) {
  int nr, nL, nv;
  int i, id;
  nr = pn.m_regionArray.size();
  if ((idRegion < 0) || (idRegion >= nr)) return;
  nL = pn.m_regionArray[idRegion].m_loopArray.size();
  if ((idLoop < 0) || (idLoop >= nL)) return;
  nv = pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray.size();
  for (i = 0; i < nv; i++) {
    id = pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray[i];
    pn.m_pointArray[id].m_x += vx;
    pn.m_pointArray[id].m_y += vy;
  }  // for结束
}  // 函数gb_moveLoop结束

void gb_movePoint(CP_Polygon& pn, int id, double vx, double vy) {
  int n = pn.m_pointArray.size();
  if ((id < 0) || (id >= n)) return;
  pn.m_pointArray[id].m_x += vx;
  pn.m_pointArray[id].m_y += vy;
}  // 函数gb_movePoint结束

void gb_movePointIntArray(CP_Polygon& pn, VT_IntArray& vi, double vx,
                          double vy) {
  int n = vi.size();
  int i;
  for (i = 0; i < n; i++) gb_movePoint(pn, vi[i], vx, vy);
}  // 函数gb_movePoint结束

void gb_movePolygon(CP_Polygon& pn, double vx, double vy) {
  int n = pn.m_pointArray.size();
  int i;
  for (i = 0; i < n; i++) {
    pn.m_pointArray[i].m_x += vx;
    pn.m_pointArray[i].m_y += vy;
  }  // for结束
}  // 函数gb_movePolygon结束

void gb_moveRegion(CP_Polygon& pn, int idRegion, double vx, double vy) {
  int nr, nL, nv;
  int i, j, k, id;
  nr = pn.m_regionArray.size();
  if ((idRegion < 0) || (idRegion >= nr)) return;
  i = idRegion;
  nL = pn.m_regionArray[i].m_loopArray.size();
  for (j = 0; j < nL; j++) {
    nv = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray.size();
    for (k = 0; k < nv; k++) {
      id = pn.m_regionArray[i].m_loopArray[j].m_pointIDArray[k];
      pn.m_pointArray[id].m_x += vx;
      pn.m_pointArray[id].m_y += vy;
    }  // for结束
  }    // for结束
}  // 函数gb_moveRegion结束

// 将在全局坐标系下的点转换成为在屏幕坐标下的点
// result:      输出的在屏幕坐标下的点;
// pointGlobal: 输入的在全局坐标系下的点;
// scale:       输入的比例因子;
// translation: 输入的平移坐标值。
void gb_pointConvertFromGlobalToScreen(CP_Point& result, CP_Point pointGlobal,
                                       double scale, CP_Point translation,
                                       int screenX, int screenY) {
  result.m_x = (pointGlobal.m_x - translation.m_x) * scale;
  result.m_y = (pointGlobal.m_y - translation.m_y) * scale;
  result.m_x += (screenX / 2);
  result.m_y = screenY / 2 - result.m_y;
}  // 函数PointConvertFromGlobalToScreen结束

// 将在屏幕坐标下的点转换成为在全局坐标系下的点
// result:      输出的在全局坐标系下的点;
// pointScreen: 输入的在屏幕坐标系下的点;
// scale:       输入的比例因子;
// translation: 输入的平移坐标值。
void gb_pointConvertFromScreenToGlobal(CP_Point& result, CP_Point pointScreen,
                                       double scale, CP_Point translation,
                                       int screenX, int screenY) {
  result.m_x = pointScreen.m_x - screenX / 2;
  result.m_y = screenY / 2 - pointScreen.m_y;
  result.m_x = result.m_x / scale + translation.m_x;
  result.m_y = result.m_y / scale + translation.m_y;
}  // 函数gb_PointConvertFromScreenToGlobal结束

// 给多边形p增加新的内环，该内环是外接圆半径为r的正n边形。
bool gb_polygonNewInLoopRegular(CP_Polygon& p, int idRegion, int n, double r,
                                double cx, double cy) {
  if (n < 3) return false;
  int nr = p.m_regionArray.size();
  if ((idRegion < 0) || (idRegion >= nr)) return false;
  int nL = p.m_regionArray[idRegion].m_loopArray.size();
  if (nL <= 0) return false;
  p.m_regionArray[idRegion].m_loopArray.resize(nL + 1);
  int s = p.m_pointArray.size();
  int t = s + n;
  int i, k;
  p.m_pointArray.resize(t);
  double da = DOUBLE_PI / n;
  double d = 0.0;
  for (i = s; i < t; i++, d += da) {
    p.m_pointArray[i].m_x = cx + r * cos(d);
    p.m_pointArray[i].m_y = cy + r * sin(d);
  }  // for结束
  p.m_regionArray[idRegion].m_loopArray[nL].m_polygon = &p;
  p.m_regionArray[idRegion].m_loopArray[nL].m_regionIDinPolygon = idRegion;
  p.m_regionArray[idRegion].m_loopArray[nL].m_loopIDinRegion = nL;
  p.m_regionArray[idRegion].m_loopArray[nL].m_pointIDArray.resize(n);
  for (i = 0, k = t - 1; i < n; i++, k--) {
    p.m_regionArray[idRegion].m_loopArray[nL].m_pointIDArray[i] = k;
  }  // for结束
  return true;
}  // 函数gb_polygonNewInLoopRegular结束

// 给多边形p增加新的外环，该外环是外接圆半径为r的正n边形。
void gb_polygonNewOutLoopRegular(CP_Polygon& p, int n, double r, double cx,
                                 double cy) {
  if (n < 3) return;
  int s = p.m_pointArray.size();
  int t = s + n;
  int i, k;
  p.m_pointArray.resize(t);
  double da = DOUBLE_PI / n;
  double d = 0.0;
  for (i = s; i < t; i++, d += da) {
    p.m_pointArray[i].m_x = cx + r * cos(d);
    p.m_pointArray[i].m_y = cy + r * sin(d);
  }  // for结束
  int rs = p.m_regionArray.size();
  p.m_regionArray.resize(rs + 1);
  p.m_regionArray[rs].m_polygon = &p;
  p.m_regionArray[rs].m_regionIDinPolygon = rs;
  p.m_regionArray[rs].m_loopArray.resize(1);
  p.m_regionArray[rs].m_loopArray[0].m_polygon = &p;
  p.m_regionArray[rs].m_loopArray[0].m_regionIDinPolygon = rs;
  p.m_regionArray[rs].m_loopArray[0].m_loopIDinRegion = 0;
  p.m_regionArray[rs].m_loopArray[0].m_pointIDArray.resize(n);
  for (i = 0, k = s; i < n; i++, k++) {
    p.m_regionArray[rs].m_loopArray[0].m_pointIDArray[i] = k;
  }  // for结束
}  // 函数gb_polygonNewOutLoopRegular结束

bool gb_removeLoop(CP_Polygon& pn, int idRegion, int idLoop) {
  int nL, nLv, iLv, v;
  nL = pn.m_regionArray[idRegion].m_loopArray.size();
  if ((idLoop == 0) || (nL < 2)) return (gb_removeRegion(pn, idRegion));
  nLv = pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray.size();
  for (iLv = 0; iLv < nLv; iLv++) {
    v = pn.m_regionArray[idRegion].m_loopArray[idLoop].m_pointIDArray[iLv];
    pn.m_pointArray.erase(pn.m_pointArray.begin() + v);
    gb_subtractOneAboveID(pn, v);
  }  // for(iLv)结束
  pn.m_regionArray[idRegion].m_loopArray.erase(
      pn.m_regionArray[idRegion].m_loopArray.begin() + idLoop);
  return true;
}  // 函数gb_removeLoop结束

bool gb_removePoint(CP_Polygon& pn, int id) {
  int ir, iL, iLv, nLv;
  bool rf = gb_findPointInLoop(pn, ir, iL, iLv, id);
  if (!rf) return false;
  nLv = pn.m_regionArray[ir].m_loopArray[iL].m_pointIDArray.size();
  if (nLv < 4)  // 删除整个环
    return (gb_removeLoop(pn, ir, iL));
  pn.m_regionArray[ir].m_loopArray[iL].m_pointIDArray.erase(
      pn.m_regionArray[ir].m_loopArray[iL].m_pointIDArray.begin() + iLv);
  pn.m_pointArray.erase(pn.m_pointArray.begin() + id);
  gb_subtractOneAboveID(pn, id);
  return true;
}  // 函数gb_removePoint结束

bool gb_removeRegion(CP_Polygon& pn, int idRegion) {
  int nr, nL, nLv, iL, iLv, v;
  nr = pn.m_regionArray.size();
  if (nr < 2) {
    pn.mb_clear();
    return true;
  }  // if结束`
  nL = pn.m_regionArray[idRegion].m_loopArray.size();
  for (iL = 0; iL < nL; iL++) {
    nLv = pn.m_regionArray[idRegion].m_loopArray[iL].m_pointIDArray.size();
    for (iLv = 0; iLv < nLv; iLv++) {
      v = pn.m_regionArray[idRegion].m_loopArray[iL].m_pointIDArray[iLv];
      pn.m_pointArray.erase(pn.m_pointArray.begin() + v);
      gb_subtractOneAboveID(pn, v);
    }  // for(iLv)结束
  }    // for(iL)结束
  pn.m_regionArray.erase(pn.m_regionArray.begin() + idRegion);
  return true;
}  // 函数gb_removeRegion结束

void gb_subtractOneAboveID(CP_Polygon& pn, int id) {
  int nr = pn.m_regionArray.size();
  int nL, nLv, ir, iL, iLv;
  for (ir = 0; ir < nr; ir++) {
    nL = pn.m_regionArray[ir].m_loopArray.size();
    for (iL = 0; iL < nL; iL++) {
      nLv = pn.m_regionArray[ir].m_loopArray[iL].m_pointIDArray.size();
      for (iLv = 0; iLv < nLv; iLv++) {
        if (pn.m_regionArray[ir].m_loopArray[iL].m_pointIDArray[iLv] >= id)
          pn.m_regionArray[ir].m_loopArray[iL].m_pointIDArray[iLv]--;
      }  // for(iLv)结束
    }    // for(iL)结束
  }      // for(ir)结束
}  // 函数gb_subtractOneAboveID结束
