#ifndef CP_POLYGON_H
#define CP_POLYGON_H

#include <iostream>
using namespace std;
#include <vector>

#define DOUBLE_PI 6.28318530717958647692
#define PI 3.14159265358979323846
#define HALF_PI 1.57079632679489661923

class CP_Point {
 public:
  double m_x, m_y;

 public:
  CP_Point(void) : m_x(0.0), m_y(0.0) {}
  CP_Point(double x, double y) : m_x(x), m_y(y) {}
};  // 类CP_Point定义结束
typedef vector<CP_Point> VT_PointArray;

class CP_Polygon;

typedef vector<int> VT_IntArray;
typedef vector<VT_IntArray> VT_IntArray2;

class CP_Loop {
 public:
  VT_IntArray m_pointIDArray;
  int m_loopIDinRegion;
  int m_regionIDinPolygon;
  CP_Polygon* m_polygon;

 public:
  CP_Loop(void)
      : m_loopIDinRegion(0), m_regionIDinPolygon(0), m_polygon(NULL) {}
};  // 类CP_Loop定义结束
typedef vector<CP_Loop> VT_LoopArray;

class CP_Region {
 public:
  VT_LoopArray m_loopArray;
  int m_regionIDinPolygon;
  CP_Polygon* m_polygon;

 public:
  CP_Region(void) : m_regionIDinPolygon(0), m_polygon(NULL) {}
};  // 类CP_Region定义结束
typedef vector<CP_Region> VT_RegionArray;

class CP_Polygon {
 public:
  VT_PointArray m_pointArray;
  VT_RegionArray m_regionArray;

 public:
  void mb_clear() {
    m_pointArray.clear();
    m_regionArray.clear();
  }
};  // 类CP_Polygon定义结束

extern void gb_distanceMinPointLoop(double& d, int& idRegion, int& idLoop,
                                    CP_Point& pt, CP_Polygon& pn);
extern void gb_distanceMinPointPolygon(double& d, int& id, CP_Point& pt,
                                       CP_Polygon& pn);
extern double gb_distancePointPoint(CP_Point& p1, CP_Point& p2);
extern double gb_distancePointSegment(CP_Point& pt, CP_Point& p1, CP_Point& p2);

extern void gb_getIntArrayPointInPolygon(VT_IntArray& vi, CP_Polygon& pn,
                                         CP_Point& p, double eT);
extern bool gb_findPointInLoop(CP_Polygon& pn, int& idRegion, int& idLoop,
                               int& idPointInLoop, int pointInPolygon);
extern void gb_insertPointInPolygon(CP_Polygon& pn, int& idRegion, int& idLoop,
                                    int& idPointInLoop, CP_Point& newPoint);
extern void gb_intArrayInit(VT_IntArray& vi, int data);
extern void gb_intArrayInitLoop(VT_IntArray& vi, CP_Polygon& pn, int idRgion,
                                int idLoop, double eT);
extern void gb_intArrayInitPoint(VT_IntArray& vi, CP_Polygon& pn, int v,
                                 double eT);
extern void gb_intArrayInitPointSame(VT_IntArray& vi, CP_Polygon& pn,
                                     double eT);
extern void gb_intArrayInitPolygon(VT_IntArray& vi, CP_Polygon& pn);
extern void gb_intArrayInitPolygonSamePoint(VT_IntArray& vr, CP_Polygon& pr,
                                            VT_IntArray& vs, CP_Polygon& ps,
                                            double eT);
extern void gb_intArrayInitRegion(VT_IntArray& vi, CP_Polygon& pn, int idRegion,
                                  double eT);
extern void gb_moveLoop(CP_Polygon& pn, int idRegion, int idLoop, double vx,
                        double vy);
extern void gb_movePoint(CP_Polygon& pn, int id, double vx, double vy);
extern void gb_movePointIntArray(CP_Polygon& pn, VT_IntArray& vi, double vx,
                                 double vy);
extern void gb_movePolygon(CP_Polygon& pn, double vx, double vy);
extern void gb_moveRegion(CP_Polygon& pn, int idRegion, double vx, double vy);
extern void gb_pointConvertFromGlobalToScreen(CP_Point& result,
                                              CP_Point pointGlobal,
                                              double scale,
                                              CP_Point translation, int screenX,
                                              int screenY);
extern void gb_pointConvertFromScreenToGlobal(CP_Point& result,
                                              CP_Point pointScreen,
                                              double scale,
                                              CP_Point translation, int screenX,
                                              int screenY);
extern bool gb_polygonNewInLoopRegular(CP_Polygon& p, int idRegion, int n,
                                       double r, double cx, double cy);
extern void gb_polygonNewOutLoopRegular(CP_Polygon& p, int n, double r,
                                        double cx, double cy);
extern bool gb_removeLoop(CP_Polygon& pn, int idRegion, int idLoop);
extern bool gb_removePoint(CP_Polygon& pn, int id);
extern bool gb_removeRegion(CP_Polygon& pn, int idRegion);
extern void gb_subtractOneAboveID(CP_Polygon& pn, int id);

#endif