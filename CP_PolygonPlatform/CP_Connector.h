#pragma once
#include "CP_SweepLine.h"
#include "CP_Polygon.h"
#include <list>

class PointChain {
public:
  typedef list<CP_Point>::iterator point_iter;
  PointChain() : point_list(), is_closed(false) {}
  std::list<CP_Point> point_list;
  bool is_closed;

public:
  void init(const Segment& s);
  bool LinkSegment(const Segment& s);
  bool LinkPointChain(PointChain& chain);
  point_iter begin() { return point_list.begin(); }
  point_iter end() { return point_list.end(); }
  void clear() { point_list.clear(); }
  unsigned int size() const { return point_list.size(); }

};

class Connector {
public:
  typedef std::list<PointChain>::iterator iterator;
  Connector() : openPolygons(), closedPolygons() {}
  ~Connector() {}
  void add(const Segment& s);
  iterator begin() { return closedPolygons.begin(); }
  iterator end() { return closedPolygons.end(); }
  void clear() { closedPolygons.clear(); openPolygons.clear(); }
  unsigned int size() const { return closedPolygons.size(); }
  void toPolygon(CP_Polygon& polygon);
private:
  list<PointChain> openPolygons;
  list<PointChain> closedPolygons;
};