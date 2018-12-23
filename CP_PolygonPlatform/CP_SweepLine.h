#pragma once
#include "CP_Polygon.h"
#include <queue>
#include <set>
#include <list>
// 定义容差
const double TOLERANCE = 1e-6;
// 浮点数比较运算
inline bool Equal(double a, double b, double tolerance = TOLERANCE) noexcept { return b - a <= tolerance && a - b <= tolerance; }

inline bool NotEqual(double a, double b, double tolerance = TOLERANCE) noexcept { return b - a > tolerance || a - b > tolerance; }

inline bool Great(double a, double b, double tolerance = TOLERANCE) noexcept { return a > b + tolerance; }

inline bool GreatEqual(double a, double b, double tolerance = TOLERANCE) noexcept { return a > b - tolerance; }

inline bool Less(double a, double b, double tolerance = TOLERANCE) noexcept { return a + tolerance < b; }

inline bool LessEqual(double a, double b, double tolerance = TOLERANCE) noexcept { return a < b + tolerance; }

inline bool operator==(const CP_Point& lhs, const CP_Point& rhs) { return Equal(lhs.m_x, rhs.m_x) && Equal(lhs.m_y, rhs.m_y); }
inline bool operator!=(const CP_Point& lhs, const CP_Point& rhs) { return !(lhs == rhs); }

// 定义向量类型CP_Vector
typedef CP_Point CP_Vector;

enum OperationType {
  kUnion, 
  kIntersection,
  kA_B,
  kB_A,
  kXor
};
enum PolygonType {
  kPolygonA,
  kPolygonB,
  kPolygonResult
};

class SweepEvent {
public: 
  SweepEvent() = default;
  SweepEvent(const CP_Point& point, PolygonType type) :
    point(std::make_shared<CP_Point>(point)),
    polygon_type(type) {}
  ~SweepEvent() = default;
  void setLeftFlag();
  bool insideOtherPolygon() const { return inside; }

public:
  // fields for first stage
  std::shared_ptr<CP_Point> point;  // point assoiated with this event 
  bool left;   // is the left endpoint of the edge
  std::shared_ptr<SweepEvent> other_event;  // other event of the edge
  //std::shared_ptr<CP_Polygon> polygon; // polygon this point belongs to
  PolygonType polygon_type;

  //fields of informations
  bool inOut;
  bool inside;

  // fields for second stage
  //int pos;
  //bool result_in_out;
  //int contour_id;
  //int parent_id; 
  //bool processed;
  //int depth;
};

struct queue_comparator {
  //bool operator() (const SweepEvent& a, const SweepEvent& b) const {
  //  // 比较x坐标
  //  if (Less(a.point->m_x , b.point->m_x)) {
  //    return true;
  //  }
  //  // x坐标相同，比较y坐标
  //  else if (Equal(a.point->m_x, b.point->m_x) && Less(a.point->m_y, b.point->m_y)) {
  //    return true;
  //  }
  //  // 两事件关联的点重合，比较其关联的点
  //  else if (Equal(a.point->m_x, b.point->m_x) && Equal(a.point->m_y, b.point->m_y)) {
  //    // 如果otherevent存在
  //    //CP_Point a_o = *a.other_event->point, b_o = *b.other_event->point;
  //    //if (Less(a_o.m_y, b_o.m_y)) {
  //    //  return true;
  //    //}
  //    //else if (Equal(a_o.m_y, b_o.m_y) && Less(a_o.m_x, b_o.m_x)) {
  //    //  return true;
  //    //}
  //    //else {
  //    //  return false;
  //    //}
  //    if (a.left == false && b.left == true) {
  //      return true;
  //    }
  //    else if (a.left == true && b.left == false) {
  //      return false;
  //    }
  //    else {
  //      CP_Point a_o = *a.other_event->point, b_o = *b.other_event->point;
  //      if (Less(a_o.m_y, b_o.m_y)) {
  //        return true;
  //      }
  //      else if (Equal(a_o.m_y, b_o.m_y) && Less(a_o.m_x, b_o.m_x)) {
  //        return true;
  //      }
  //      else {
  //        return false;
  //      }
  //    }
  //  }
  //  else {
  //    return false;
  //  }
  //}
  bool operator() (const SweepEvent& a, const SweepEvent& b) const {
    if (Less(a.point->m_x, b.point->m_x))
      return true;
    if (Great(a.point->m_x, b.point->m_x))
      return false;
    if (*a.point != *b.point)
      return Less(a.point->m_y, b.point->m_y);
    if (a.left != b.left)
      return !a.left;
    // other_event中较低的那个
    CP_Point a_o = *a.other_event->point, b_o = *b.other_event->point;
    if (Less(a_o.m_y, b_o.m_y))
      return true;
    if (Great(a_o.m_y, b_o.m_y))
      return false;
    if (a_o != b_o)
      return Less(a_o.m_x, b_o.m_x);
  }
};

struct status_comparator { 
  bool operator() (const SweepEvent& a, const SweepEvent& b) const {
    // 比较y坐标
    if (Less(a.point->m_y, b.point->m_y)) {
      return true;
    }
    // y坐标相同，比较x坐标
    else if (Equal(a.point->m_y, b.point->m_y) && Less(a.point->m_x, b.point->m_x)) {
      return true;
    }
    // 两个事件对应的点相同，比较其关联的点。
    else if (Equal(a.point->m_x, b.point->m_x) && Equal(a.point->m_y, b.point->m_y)) {
      CP_Point a_o = *a.other_event->point, b_o = *b.other_event->point;
      if (Less(a_o.m_y, b_o.m_y)) {
        return true;
      }
      //else if (Equal(a_o.m_y, b_o.m_y) && Less(a_o.m_x, b_o.m_x)) {
      //  return true;
      //}
      else {
        return false;
      }
    }
    else {
      return false;
    }
  }
};

typedef std::set<SweepEvent, queue_comparator> EventQueue;
typedef std::set<SweepEvent, status_comparator> StatusSet;

// 初始化SweepEvent优先队列
extern void initializeQueue(const CP_Polygon& polygon, EventQueue & event_queue, PolygonType type);  
// First Stage: 分割边
extern void booleanOperation(EventQueue& event, CP_Polygon& result, OperationType type);
// First Stage: 分割边
extern void subdivision(EventQueue& event, CP_Polygon& result);
//// set information
//extern void setInformation(SweepEvent & pos, SweepEvent & prev);
// 检查是否存在线段相交
extern int possibleIntersection(SweepEvent& a, SweepEvent& b, EventQueue & event_queue, StatusSet & status_set);

class Segment {
public:
  /** Default constructor */
  Segment() {}
  ~Segment() {}
  Segment(const SweepEvent& e) {
    source = *e.point;
    target = *e.other_event->point;
  }
public:
  CP_Point source, target;
};

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