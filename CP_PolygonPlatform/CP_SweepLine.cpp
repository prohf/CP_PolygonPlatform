#include "stdafx.h"
#include "CP_SweepLine.h"
#include <iterator>

void SweepEvent::setLeftFlag() {
  if (other_event != nullptr) {
    auto other_point = other_event->point;
    if (Less(point->m_x, other_point->m_x)) {
      left = true;
      other_event->left = false;
      return;
    }
    else if (Equal(point->m_x, other_point->m_x) && Less(point->m_y, other_point->m_y)) {
      left = true;
      other_event->left = false;
      return;
    }
    else {
      left = false;
      other_event->left = true;
      return;
    }
  }
}

void initializeQueue(const CP_Polygon & polygon, EventQueue & event_queue, PolygonType type) {
  // TODO: ��ɳ�ʼ��
  int loop_first_id = 0;
  int loop_size = 0;
  int point_id = 0;
  // ��������ε���������
  for (auto region : polygon.m_regionArray) {
    // ���������е����л�
    for (int l = 0; l < region.m_loopArray.size(); ++l) {
      auto loop = region.m_loopArray[l];
      loop_first_id = loop.m_pointIDArray[0];
      loop_size = loop.m_pointIDArray.size();
      int d = loop_first_id == point_id ? 1: -1;
      for (int i = 0; i < loop_size; ++i) {
        auto & source = polygon.m_pointArray[loop_first_id + d * (i % loop_size)];
        auto & target = polygon.m_pointArray[loop_first_id + d * ((i + 1) % loop_size)];
        // ��ʼ��SweepEvent
        SweepEvent event_s(source, type), event_t(target, type);
        event_s.other_event = std::make_shared<SweepEvent>(event_t);
        event_t.other_event = std::make_shared<SweepEvent>(event_s);
        event_s.other_event = std::make_shared<SweepEvent>(event_t);
        event_t.other_event = std::make_shared<SweepEvent>(event_s);
        // �ж��ĸ�����ߵĵ�
        event_s.setLeftFlag();
        event_t.setLeftFlag();
        // ���������
        event_queue.push(event_s);
        event_queue.push(event_t);
        ++point_id;
      }
    }
  }
}

// ��һ�׶��㷨���ָ��
// TODO: ʵ��setInformation �� possibleIntersection
void subdivision(EventQueue & event_queue, CP_Polygon & result) {
  StatusSet status_set;
  while (!event_queue.empty()) {
    auto sweep_event = event_queue.top();
    event_queue.pop();
    if (sweep_event.left) { // left endpoint
      auto position_it = status_set.insert(sweep_event).first;
      if (position_it != status_set.end()) {
        // �ÿ����ĵ��������position_it
        auto pos_it1 = position_it;
        auto pos_it2 = position_it;
        auto position = *position_it;
        SweepEvent previous, next;
        bool prev_flag = false, next_flag = false;
        if (position_it != status_set.begin()) {
          previous = *std::prev(pos_it1, 1);
          prev_flag = true;
        }
        if (position_it != std::prev(status_set.end(), 1)) {
          next = *std::next(pos_it2, 1);
          next_flag = true;
        }
        if (prev_flag)
          possibleIntersection(position, previous, event_queue, status_set); 
        if (next_flag)
          possibleIntersection(position, next, event_queue, status_set);
      }
    }
    else { // right endpoint
      auto position_it = status_set.find(*sweep_event.other_event);
      if (position_it != status_set.end()) { // ����status_set��
        // �ÿ����ĵ��������position_it
        auto pos_it1 = position_it;
        auto pos_it2 = position_it;
        if (position_it != status_set.begin() && position_it != std::prev(status_set.end(), 1)) {
          auto previous = *std::prev(position_it, 1);
          auto next = *std::next(position_it, 1);
          possibleIntersection(previous, next, event_queue, status_set);
        }
        status_set.erase(position_it);
      }
    }
    // if event is in result
    // add event to the result
    if (sweep_event.polygon_type == PolygonType::kPolygonResult) {
      result.m_pointArray.push_back(*sweep_event.point);
    }
  }
}

// �����¼�����Ϣ
void setInformation(SweepEvent & this_event, SweepEvent & other_event) {
  
}

// to-left-test
double toLeftTest(const CP_Point& a, const CP_Point& b, const CP_Point& p) {
  return (a.m_x - b.m_x) * (a.m_y - p.m_y) - (a.m_y - b.m_y) * (a.m_x - p.m_x);
}

// �����߶ν��㺯��
CP_Point intersectPoint(const CP_Point & a, const CP_Point & b, const CP_Point & u, const CP_Point & v) {
  // �������
  // b = a + p, v = u + q
  CP_Vector p(b.m_x - a.m_x, b.m_y - a.m_y);
  CP_Vector q(v.m_x - u.m_x, v.m_y - u.m_y);
  // i = a + s*p = u + t*q
  // s = (u - a) cross q / p cross q 
  double p_cross_q = p.m_x * q.m_y - p.m_y * q.m_x;
  CP_Vector u_a(u.m_x - a.m_x, u.m_y - a.m_y); // u - a
  double u_a_cross_q = u_a.m_x * q.m_y - u_a.m_y * q.m_x;
  double s = u_a_cross_q / p_cross_q;
  return CP_Point(a.m_x + s * p.m_x, a.m_y + s * p.m_y); // ����Ϊi
}

// ���¼��Ƴ�StatusSet 
void removeFromStatusSet(SweepEvent & ab, SweepEvent & uv, StatusSet & status_set) {
  if (ab.polygon_type != PolygonType::kPolygonResult && uv.polygon_type != PolygonType::kPolygonResult) {
    auto ab_iter = status_set.find(ab);
    auto uv_iter = status_set.find(uv);
    if (ab_iter != status_set.end()) {
      status_set.erase(ab_iter);
    }
    if (uv_iter != status_set.end()) {
      status_set.erase(uv_iter);
    }
  }
}

// ���߶ν����߶��У��޸� EventQueue �� StatusSet
int intersectAtCrossPoint(SweepEvent & intersect, std::shared_ptr<SweepEvent> other, EventQueue & event_queue, StatusSet & status_set) {
  intersect.other_event = other;
  other->other_event = std::make_shared<SweepEvent>(intersect);
  intersect.other_event = other;
  intersect.setLeftFlag();
  event_queue.push(intersect);
  if (intersect.left) {
    status_set.insert(intersect);
  }
  return 1;
}

// һ�߶εĶ˵�����һ�߶��ϣ������������¼����޸� EventQueue �� StatusSet
int intersectAtEndpoint(SweepEvent & intersect, SweepEvent& endpoint1, SweepEvent& endpoint2, EventQueue & event_queue, StatusSet & status_set) {
  // ���������¼������߶ζ˵�����������������
  if (intersect.polygon_type != PolygonType::kPolygonResult) { // ���벻���ཻ�Ľ���
    CP_Point point = *intersect.point;
    SweepEvent intersect1(point, PolygonType::kPolygonResult), intersect2(point, PolygonType::kPolygonResult);
    intersect1.other_event = std::make_shared<SweepEvent>(endpoint1);
    endpoint1.other_event = std::make_shared<SweepEvent>(intersect1);
    intersect1.other_event = std::make_shared<SweepEvent>(endpoint1);
    intersect2.other_event = std::make_shared<SweepEvent>(endpoint2);
    endpoint2.other_event = std::make_shared<SweepEvent>(intersect2);
    intersect2.other_event = std::make_shared<SweepEvent>(endpoint2);
    intersect1.setLeftFlag();
    intersect2.setLeftFlag();
    event_queue.push(intersect1);
    event_queue.push(intersect2);
    if (intersect1.left) {
      status_set.insert(intersect1);
    }
    if (intersect2.left) {
      status_set.insert(intersect2);
    }
    return 1;
  }
  return 0;
}

// ������ܴ��ڵ��߶��ཻ
// ����������ƪ�������ṩ���߶����㷨
// Bennellabc J A. The geometry of nesting problems: A tutorial[J]. European Journal of Operational Research, 2008, 184(2):397-415.
// D-function ΪtoLeftTest����
int possibleIntersection(SweepEvent & ab, SweepEvent & uv, EventQueue & event_queue, StatusSet & status_set) {
  auto ba = ab.other_event; // ��Ӧb
  auto vu = uv.other_event; // ��Ӧv
  // ȡ��SweepEvent�еĵ�
  CP_Point a = *ab.point;
  CP_Point b = *ba->point;
  CP_Point u = *uv.point;
  CP_Point v = *vu->point;
  // ����D-function
  double d_abu = toLeftTest(a, b, u);
  double d_abv = toLeftTest(a, b, v);
  double d_uva = toLeftTest(u, v, a);
  double d_uvb = toLeftTest(u, v, b);
  // ����������ཻ���
  // �ཻ�����߶���
  // ���㷨https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
  // d_abu != 0 && d_abv != 0 && d_uva != 0 && d_uvb != 0
  if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && NotEqual(d_uvb, 0.0)
      && Less(d_abu * d_abv, 0.0) && Less(d_uva * d_uvb, 0.0)) { 
    // ���¼�ab �� uv �Ƴ� StatusSet
    removeFromStatusSet(ab, uv, status_set);
    // �������
    CP_Point i = intersectPoint(a, b, u, v);
    // �޸�SweepEvent �� Status_set,���������к�״̬����(�����ĸ��¼���
    // i<->a
    SweepEvent event_i_a(i, PolygonType::kPolygonResult);
    intersectAtCrossPoint(event_i_a, std::make_shared<SweepEvent>(ab), event_queue, status_set);
    // i<->b
    SweepEvent event_i_b(i,PolygonType::kPolygonResult);
    intersectAtCrossPoint(event_i_b, ba, event_queue, status_set);
    // i<->u
    SweepEvent event_i_u(i, PolygonType::kPolygonResult);
    intersectAtCrossPoint(event_i_u, std::make_shared<SweepEvent>(uv), event_queue, status_set);
    // i<->v
    SweepEvent event_i_v(i, PolygonType::kPolygonResult);
    intersectAtCrossPoint(event_i_v, vu, event_queue, status_set);
    return 1;
  }
  // TODO��һ���߶εĶ˵�����һ���߶���
  // u �� ab ��
  if (Equal(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && NotEqual(d_uvb, 0.0) && NotEqual(d_uva, d_uvb)) {
    removeFromStatusSet(ab, uv, status_set);
    return intersectAtEndpoint(uv, ab, *ba, event_queue, status_set);
  }
  // v �� ab ��
  if (NotEqual(d_abu, 0.0) && Equal(d_abv, 0.0) && NotEqual(d_uva, 0, 0) && NotEqual(d_uvb, 0.0) && NotEqual(d_uva, d_uvb)) {
    removeFromStatusSet(ab, uv, status_set);
    return intersectAtEndpoint(*vu, ab, *ba, event_queue, status_set);
  }
  // a �� uv ��
  if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && Equal(d_uva, 0.0) && NotEqual(d_uvb, 0.0) && NotEqual(d_abu, d_abv)) {
    removeFromStatusSet(ab, uv, status_set);
    return intersectAtEndpoint(ab, uv, *vu, event_queue, status_set);
  }
  // b �� uv ��
  if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && Equal(d_uvb, 0.0) && NotEqual(d_abu, d_abv)) {
    removeFromStatusSet(ab, uv, status_set);
    return intersectAtEndpoint(*ba, uv, *vu, event_queue, status_set);
  }
  // TODO: ���߶��ص�
  // �����߶��ཻ�ڶ˵�
  // ���ཻ
  return 0;
}