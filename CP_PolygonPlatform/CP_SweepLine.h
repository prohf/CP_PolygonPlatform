#pragma once
#include "CP_Polygon.h"
#include <queue>
#include <set>
// �����ݲ�
const double TOLERANCE = 1e-6;
// �������Ƚ�����
inline bool Equal(double a, double b, double tolerance = TOLERANCE) noexcept { return b - a <= tolerance && a - b <= tolerance; }

inline bool NotEqual(double a, double b, double tolerance = TOLERANCE) noexcept { return b - a > tolerance || a - b > tolerance; }

inline bool Great(double a, double b, double tolerance = TOLERANCE) noexcept { return a > b + tolerance; }

inline bool GreatEqual(double a, double b, double tolerance = TOLERANCE) noexcept { return a > b - tolerance; }

inline bool Less(double a, double b, double tolerance = TOLERANCE) noexcept { return a + tolerance < b; }

inline bool LessEqual(double a, double b, double tolerance = TOLERANCE) noexcept { return a < b + tolerance; }
// ������������CP_Vector
typedef CP_Point CP_Vector;
enum PolygonType {
  kPolygonA,
  kPolygonB,
  kPolygonResult
};

class SweepEvent {
public: 
  SweepEvent(const CP_Point& point, PolygonType type) :
    point(std::make_shared<CP_Point>(point)),
    polygon_type(type) {}
  ~SweepEvent() = default;
  void setLeftFlag();

public:
  // fields for first stage
  std::shared_ptr<CP_Point> point;  // point assoiated with this event 
  bool left;   // is the left endpoint of the edge
  std::shared_ptr<SweepEvent> other_event;  // other event of the edge
  //std::shared_ptr<CP_Polygon> polygon; // polygon this point belongs to
  PolygonType polygon_type;

  // fields for second stage
  int pos;
  bool result_in_out;
  int contour_id;
  int parent_id; 
  bool processed;
  int depth;
};

struct queue_comparator {
  bool operator() (const SweepEvent& a, const SweepEvent& b) const {
    //return a.point->m_x > b.point->m_x;
    if (Great(a.point->m_x , b.point->m_x)) {
      return true;
    }
    else if (Equal(a.point->m_x, b.point->m_x) && Great(a.point->m_y, b.point->m_y)) {
      return true;
    }
    else {
      return false;
    }
  }
};

struct status_comparator { 
  bool operator() (const SweepEvent& a, const SweepEvent& b) const {
    if (Great(a.point->m_y, b.point->m_y)) {
      return true;
    }
    else if (Equal(a.point->m_y, b.point->m_y) && Great(a.point->m_x, b.point->m_x)) {
      return true;
    }
    // �����¼���Ӧ�ĵ���ͬ���Ƚ�������ĵ㡣
    else if (Equal(a.point->m_x, b.point->m_x) && Equal(a.point->m_y, b.point->m_y)) {
      CP_Point a_o = *a.other_event->point, b_o = *b.other_event->point;
      if (Great(a_o.m_y, b_o.m_y)) {
        return true;
      }
      else if (Equal(a_o.m_y, b_o.m_y) && Great(a_o.m_x, b_o.m_x)) {
        return true;
      }
      else {
        return false;
      }
    }
    else {
      return false;
    }
  }
};

typedef std::priority_queue<SweepEvent, std::vector<SweepEvent>, queue_comparator> EventQueue;
typedef std::set<SweepEvent, status_comparator> StatusSet;

// ��ʼ��SweepEvent���ȶ���
extern void initializeQueue(const CP_Polygon& polygon, EventQueue & event_queue, PolygonType type);  
// First Stage: �ָ��
extern void subdivision(EventQueue& event, CP_Polygon& result);  
// set information
extern void setInformation(SweepEvent& this_event, SweepEvent& other_event);
// ����Ƿ�����߶��ཻ
extern int possibleIntersection(SweepEvent& a, SweepEvent& b, EventQueue & queue, CP_Polygon & result);