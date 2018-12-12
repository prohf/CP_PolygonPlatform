#pragma once
#include "CP_Polygon.h"
#include <queue>
#include <set>
const double TOLERANCE = 1e-6;

enum PolygonType {
  SUBJECT,
  CLIPPING
};

class SweepEvent {
public: 
  SweepEvent() = default;
  SweepEvent(const CP_Point& point) : point(std::make_shared<CP_Point>(point)) {}
  ~SweepEvent() = default;



public:
  // fields for first stage
  std::shared_ptr<CP_Point> point;  // point assoiated with this event 
  bool left;   // is the left endpoint of the edge
  std::shared_ptr<SweepEvent> other_event;  // other event of the edge
  PolygonType pol;  // can be SUBJECT or CLIPPING

  // fields for second stage
  int pos;
  bool result_in_out;
  int contour_id;
  int parent_id; 
  bool processed;
  int depth;
};

struct queue_comparator {
  inline bool operator() (const SweepEvent& a, const SweepEvent& b) const {
    return a.point->m_x < b.point->m_x;
  }
};
struct status_comparator {
  inline bool operator() (const SweepEvent& a, const SweepEvent& b) const {
    return a.point->m_y < b.point->m_y;
  }
};
typedef std::priority_queue<SweepEvent, std::vector<SweepEvent>, queue_comparator> EventQueue;
typedef std::set<SweepEvent, status_comparator> StatusSet;

// 浮点数比较运算
inline bool Equal(double a, double b, double tolerance = TOLERANCE) noexcept { return b - a <= tolerance && a - b <= tolerance; }

inline bool NotEqual(double a, double b, double tolerance = TOLERANCE) noexcept { return b - a > tolerance || a - b > tolerance; }

inline bool Great(double a, double b, double tolerance = TOLERANCE) noexcept { return a > b + tolerance; }

inline bool GreatEqual(double a, double b, double tolerance = TOLERANCE) noexcept { return a > b - tolerance; }

inline bool Less(double a, double b, double tolerance = TOLERANCE) noexcept { return a + tolerance < b; }

inline bool LessEqual(double a, double b, double tolerance = TOLERANCE) noexcept { return a < b + tolerance; }

// 初始化SweepEvent优先队列
extern EventQueue initializeQueue(const CP_Polygon& polygon);  
// First Stage: 分割边
extern void subdivision(EventQueue& event);  
// set information
extern void setInformation(SweepEvent& this_event, SweepEvent& other_event);
// 检查是否存在线段相交
extern void possibleIntersection(SweepEvent& a, SweepEvent& b);