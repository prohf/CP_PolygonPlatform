#include "stdafx.h"
#include "CP_SweepLine.h"
#include <iterator>

EventQueue initializeQueue(const CP_Polygon & polygon) {
  EventQueue queue;
  // TODO: ��ɳ�ʼ��
  int loop_first_id = 0;
  int loop_size = 0;
  // ��������ε���������
  for (auto region : polygon.m_regionArray) {
    // ���������е����л�
    for (auto loop : region.m_loopArray) {
      // ������ʼid�ͻ��Ĵ�С
      loop_first_id = loop.m_pointIDArray[0];
      loop_size = loop.m_pointIDArray.size();
      // �Ի��е�ÿһ������д���
      for (int i = 0; i < loop_size; ++i) {
        auto & source = polygon.m_pointArray[loop_first_id + i % loop_size];
        auto & target = polygon.m_pointArray[loop_first_id + (i + 1) % loop_size];
        // ��ʼ��SweepEvent
        SweepEvent event_s(source), event_t(target);
        event_s.other_event = std::make_shared<SweepEvent>(event_t);
        event_t.other_event = std::make_shared<SweepEvent>(event_s);
        // �ж��ĸ�����ߵĵ�
        if (source.m_x < target.m_x) {
          event_s.left = true;
          event_t.left = false;
        } 
        else {
          event_s.left = false;
          event_t.left = true;
        }
        // ���������
        queue.push(event_s);
        queue.push(event_t);
      }
    }
  }
  return queue;
}

// ��һ�׶��㷨���ָ��
// TODO: ʵ��setInformation �� possibleIntersection
void subdivision(EventQueue & event_queue) {
  StatusSet status_set;
  while (!event_queue.empty()) {
    auto & event = event_queue.top();
    event_queue.pop();
    if (event.left) { // left endpoint
      auto position = status_set.insert(event).first;
      auto previous = std::prev(position, 1);
      auto next = std::next(position, 1);
      // possibleIntersection(pos, prev)
      // possibleIntersection(pos, next)
    }
    else { // right endpoint
      auto position = status_set.find(*event.other_event);
      auto previous = std::prev(position, 1);
      auto next = std::next(position, 1);
      status_set.erase(position);
      // possibleIntersection(prev, next)
    }
    // if event is in result
    // add event to the result
  }
}

void setInformation(SweepEvent & this_event, SweepEvent & other_event) {

}

// to-left-test
double toLeftTest(const CP_Point& a, const CP_Point& b, const CP_Point& p) {
  return (a.m_x - b.m_x) * (a.m_y - p.m_y) - (a.m_y - b.m_y) * (a.m_x - p.m_x);
}

// ������ܴ��ڵ��߶��ཻ
// ����������ƪ�������ṩ���߶����㷨
// Bennellabc J A. The geometry of nesting problems: A tutorial[J]. European Journal of Operational Research, 2008, 184(2):397-415.
// D-function ΪtoLeftTest����
void possibleIntersection(SweepEvent & ab, SweepEvent & uv) {
  // ȡ��SweepEvent�еĵ�
  CP_Point a = *ab.point;
  CP_Point b = *ab.other_event->point;
  CP_Point u = *uv.point;
  CP_Point v = *uv.other_event->point;
  // ����D-function
  double d_abu = toLeftTest(a, b, u);
  double d_abv = toLeftTest(a, b, v);
  double d_uva = toLeftTest(u, v, a);
  double d_uvb = toLeftTest(u, v, b);
  // 

}

