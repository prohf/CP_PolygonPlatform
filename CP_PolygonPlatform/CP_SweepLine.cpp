#include "stdafx.h"
#include "CP_SweepLine.h"
#include <iterator>

void SweepEvent::setLeftFlag() {
  if (other_event) {
    auto other_point = other_event->point;
    if (Less(point->m_x, other_point->m_x)) {
      left = true;
      other_event->left = false;
    }
    else if (Equal(point->m_x, other_point->m_x) && Less(point->m_y, other_point->m_y)) {
      left = true;
      other_event->left = false;
    }
    else {
      left = false;
      other_event->left = true;
    }
  }
}

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
        SweepEvent event_s(source, polygon), event_t(target, polygon);
        event_s.other_event = std::make_shared<SweepEvent>(event_t);
        event_t.other_event = std::make_shared<SweepEvent>(event_s);
        // �ж��ĸ�����ߵĵ�
        event_s.setLeftFlag();
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
void subdivision(EventQueue & event_queue, CP_Polygon & result) {
  StatusSet status_set;
  while (!event_queue.empty()) {
    auto & event = event_queue.top();
    event_queue.pop();
    if (event.left) { // left endpoint
      auto position_it = status_set.insert(event).first;
      auto position = *position_it;
      if (position_it != status_set.begin()) {
        auto previous = *std::prev(position_it, 1);
        possibleIntersection(position, previous, event_queue, result);
      }
      if (position_it != std::prev(status_set.end())) {
        auto next = *std::next(position_it, 1);
        possibleIntersection(position, next, event_queue, result);
      }
    }
    else { // right endpoint
      auto position_it = status_set.find(*event.other_event);
      if (position_it != status_set.begin() && position_it != std::prev(status_set.end())) {
        auto previous = *std::prev(position_it, 1);
        auto next = *std::next(position_it, 1);
        possibleIntersection(previous, next, event_queue, result);
      }
      status_set.erase(position_it);
    }
    // if event is in result
    // add event to the result
    if (&*event.polygon == &result) {
      result.m_pointArray.push_back(*event.point);
    }
  }
}

void setInformation(SweepEvent & this_event, SweepEvent & other_event) {
  
}

// to-left-test
double toLeftTest(const CP_Point& a, const CP_Point& b, const CP_Point& p) {
  return (a.m_x - b.m_x) * (a.m_y - p.m_y) - (a.m_y - b.m_y) * (a.m_x - p.m_x);
}

// һ�߶εĶ˵�����һ�߶��У������������¼�
void intersectAtEndpoint(SweepEvent & intersect, SweepEvent& endpoint1, SweepEvent& endpoint2, EventQueue & event_queue, CP_Polygon & result) {
  // ���������¼������߶ζ˵�����������������
  CP_Point point = *intersect.point;
  SweepEvent intersect1(point, result), intersect2(point, result);
  intersect1.other_event = std::make_shared<SweepEvent>(endpoint1);
  endpoint1.other_event = std::make_shared<SweepEvent>(intersect1);
  intersect2.other_event = std::make_shared<SweepEvent>(endpoint2);
  endpoint2.other_event = std::make_shared<SweepEvent>(intersect2);
  intersect1.setLeftFlag();
  intersect2.setLeftFlag();
  event_queue.push(intersect1);
  event_queue.push(intersect2);
}

// ������ܴ��ڵ��߶��ཻ
// ����������ƪ�������ṩ���߶����㷨
// Bennellabc J A. The geometry of nesting problems: A tutorial[J]. European Journal of Operational Research, 2008, 184(2):397-415.
// D-function ΪtoLeftTest����
int possibleIntersection(SweepEvent & ab, SweepEvent & uv, EventQueue & event_queue, CP_Polygon & result) {
  auto ba = ab.other_event; // ��Ӧb
  auto vu = uv.other_event; // ��Ӧv
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
  // ����������ཻ���
  // �ཻ�����߶���
  // ���㷨https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
  // d_abu != 0 && d_abv != 0 && d_uva != 0 && d_uvb != 0
  if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && NotEqual(d_uvb, 0.0)) {
    // �������
    // b = a + p, v = u + q
    CP_Vector p(b.m_x - a.m_x, b.m_y - a.m_y);
    CP_Vector q(v.m_x - u.m_x, v.m_y - u.m_y);
    // i = a + s*p = u + t*q
    // s = (a - u) cross q / p cross q 
    double p_cross_q = p.m_x * q.m_y - p.m_y * q.m_x;
    CP_Vector a_u(a.m_x - u.m_x, a.m_y - u.m_y); // a - u
    double a_u_cross_q = a_u.m_x * q.m_y - a.m_y * q.m_x;
    double s = a_u_cross_q / p_cross_q;
    CP_Point i(a.m_x + s * p.m_x, a.m_y + s * p.m_y); // ����Ϊi
    // �޸�SweepEvent,������������
    // i<->a
    SweepEvent event_i_a(i, result);
    event_i_a.other_event = std::make_shared<SweepEvent>(ab);
    ab.other_event = std::make_shared<SweepEvent>(event_i_a);
    event_i_a.setLeftFlag();
    event_queue.push(event_i_a);
    // i<->b
    SweepEvent event_i_b(i, result);
    event_i_b.other_event = ba;
    ba->other_event = std::make_shared<SweepEvent>(event_i_b);
    event_i_b.setLeftFlag();
    event_queue.push(event_i_b);
    // i<->u
    SweepEvent event_i_u(i, result);
    event_i_u.other_event = std::make_shared<SweepEvent>(uv);
    uv.other_event = std::make_shared<SweepEvent>(event_i_u);
    event_i_u.setLeftFlag();
    event_queue.push(event_i_u);
    // i<->v
    SweepEvent event_i_v(i, result);
    event_i_v.other_event = vu;
    vu->other_event = std::make_shared<SweepEvent>(event_i_v);
    event_i_v.setLeftFlag();
    event_queue.push(event_i_v);
    return 1;
  }
  // TODO��һ���߶εĶ˵�����һ���߶���
  // u �� ab ��
  if (Equal(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && NotEqual(d_uvb, 0.0)) {
    intersectAtEndpoint(uv, ab, *ba, event_queue, result);
    return 1;
  }
  // v �� ab ��
  if (NotEqual(d_abu, 0.0) && Equal(d_abv, 0.0) && NotEqual(d_uva, 0, 0) && NotEqual(d_uvb, 0.0)) {
    intersectAtEndpoint(*vu, ab, *ba, event_queue, result);
    return 1;
  }
  // a �� uv ��
  if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && Equal(d_uva, 0.0) && NotEqual(d_uvb, 0.0)) {
    intersectAtEndpoint(ab, uv, *vu, event_queue, result);
    return 1;
  }
  // b �� uv ��
  if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && Equal(d_uvb, 0.0)) {
    intersectAtEndpoint(*ba, uv, *vu, event_queue, result);
    return 1;
  }
  // TODO: ���߶��ص�
  //if (Equal(d_abu, 0.0) && Equal(d_abv, 0.0)) {
  //  // ����������
  //  EventQueue tmp_queue;                                                                        
  //  tmp_queue.push(ab);
  //  tmp_queue.push(*ba);
  //  tmp_queue.push(uv);
  //  tmp_queue.push(*vu);
  //  auto left = tmp_queue.top();
  //  tmp_queue.pop();
  //  while (!tmp_queue.empty()) {
  //    auto right = tmp_queue.top();
  //    CP_Point right_point;
  //    left.other_event = std::make_shared<SweepEvent>(right);
  //    right.other_event = std::make_shared<SweepEvent>(left);
  //    
  //  }
  //}
  // �����߶��ཻ�ڶ˵�
  // ���ཻ
  return 0;
}