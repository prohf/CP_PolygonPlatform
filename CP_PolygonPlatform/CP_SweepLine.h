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
class SweepEvent;
class Segment {
public:
  /** Default constructor */
  Segment() {}
  Segment(const CP_Point& s, const CP_Point& t) : source(s), target(t) {}
  ~Segment() {}
public:
  CP_Point source, target;
};

enum OperationType {
  kUnion, 
  kIntersection,
  kA_B,
  kB_A,
  kXor,
  kSubdivision
};
enum PolygonType {
  kPolygonA,
  kPolygonB,
  kPolygonResult
};

class queue_comparator : public binary_function<SweepEvent*, SweepEvent*, bool> {
public:
  bool operator() (const SweepEvent* a, const SweepEvent* b) const;
};

class status_comparator : public binary_function<SweepEvent*, SweepEvent*, bool> {
public:
  bool operator() (const SweepEvent* a, const SweepEvent* b) const;
};

typedef std::priority_queue<SweepEvent*, std::vector<SweepEvent*>, queue_comparator> EventQueue;
typedef std::set<SweepEvent*, status_comparator> StatusSet;

class SweepEvent {
public: 
  SweepEvent() = default;
  SweepEvent(const CP_Point& point, SweepEvent* other, PolygonType type) : point(point), other_event(other), polygon_type(type) {}
  void setLeftFlag();
  Segment segment() { return Segment(point, other_event->point); }
  inline bool below(const CP_Point& p) const;
  inline bool above(const CP_Point& p) const;

public:
  // fields for first stage
  CP_Point point;  // point assoiated with this event 
  bool left;   // is the left endpoint of the edge
  SweepEvent* other_event;  // other event of the edge
  PolygonType polygon_type; // polygon type
  StatusSet::iterator poss; // the position in StatusSet

  //fields of informations
  bool inOut;
  bool inside;
};

class CP_SweepLine {
public:
  CP_SweepLine() : event_queue(), event_holder() {}

  void initializeQueue(const CP_Polygon& polygon, PolygonType type);

  void booleanOperation(CP_Polygon& result, OperationType type);

  void clear();

private:
  EventQueue event_queue;
  deque<SweepEvent> event_holder;

  void processSegment(const Segment& s, PolygonType pl);

  int possibleIntersection(SweepEvent *e1, SweepEvent *e2);

  void divideSegment(SweepEvent *e, const CP_Point& p, PolygonType type);

  SweepEvent *storeSweepEvent(const SweepEvent& e) { event_holder.push_back(e); return &event_holder.back(); }
};