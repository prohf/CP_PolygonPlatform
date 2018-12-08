#pragma once
#include "CP_Polygon.h"
#include <queue>
#include <set>

enum PolygonType {
  SUBJECT,
  CLIPPING
};

class SweepEvent {
public: 
  SweepEvent() = default;
  SweepEvent(const CP_Point& point) : point(std::make_shared<CP_Point>(point)) {}
  ~SweepEvent() = default;
  void setInformation(const SweepEvent& other);


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

// ��ʼ��SweepEvent���ȶ���
extern EventQueue initializeQueue(const CP_Polygon& polygon);  
// First Stage: �ָ��
extern void subdivision(EventQueue& event);  
// ����Ƿ�����߶��ཻ
extern void possibleIntersection(SweepEvent& a, SweepEvent& b);
// 