#include "stdafx.h"
#include "CP_SweepLine.h"
#include "CP_Connector.h"
#include <iterator>

// to-left-test
inline double toLeftTest(const CP_Point& a, const CP_Point& b, const CP_Point& p) {
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

inline bool SweepEvent::below(const CP_Point & p) const { 
  return left ? toLeftTest(point, other_event->point, p) < 0 : toLeftTest(other_event->point, point, p) < 0;
}

inline bool SweepEvent::above(const CP_Point & p) const { return !below(p); }

bool queue_comparator::operator()(const SweepEvent* a, const SweepEvent* b) const {
  if (Great(a->point.m_x, b->point.m_x))
    return true;
  if (Less(a->point.m_x, b->point.m_x))
    return false;
  if (a->point != b->point)
    return Great(a->point.m_y, b->point.m_y);
  if (a->left != b->left)
    return a->left;
  // other_event�нϵ͵��Ǹ�
  return a->above(b->other_event->point);
}

bool status_comparator::operator()(const SweepEvent* a, const SweepEvent* b) const {
  // a, b ������
  if (toLeftTest(a->point, a->other_event->point, b->point) || toLeftTest(a->point, a->other_event->point, b->other_event->point)) {
    // �����˵���ȣ����Ҷ˵�Ƚ�
    if (a->point == b->point)
      return a->below(b->point);

    // ��ͬ�ĵ�
    queue_comparator comp;
    // a �Ȳ������
    if (comp(a, b))
      return b->above(a->point);
    return a->below(b->point);
  }
  else {
    if (a->point == b->point)
      return a->below(b->point);
    queue_comparator comp;
    return comp(a, b);
  }
}

void SweepEvent::setLeftFlag() {
  if (other_event != nullptr) {
    auto other_point = other_event->point;
    if (Less(point.m_x, other_point.m_x)) {
      left = true;
      other_event->left = false;
      return;
    }
    else if (Equal(point.m_x, other_point.m_x) && Less(point.m_y, other_point.m_y)) {
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
/***********************************************************************
ʵ�� CP_SweepLine ����
***********************************************************************/
void CP_SweepLine::initializeQueue(const CP_Polygon & polygon, PolygonType type) {
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
        auto source = polygon.m_pointArray[loop_first_id + d * (i % loop_size)];
        auto target = polygon.m_pointArray[loop_first_id + d * ((i + 1) % loop_size)];
        // �����߶ζ�Ӧ�� SweepEvent ���뵽EventQueue �С�
        Segment s(source, target);
        processSegment(s, type);
        ++point_id;
      }
    }
  }
}

// �����߶��¼�
void CP_SweepLine::processSegment(const Segment& s, PolygonType pl) {
  SweepEvent *event_s = storeSweepEvent(SweepEvent(s.source, nullptr, pl));
  SweepEvent *event_t = storeSweepEvent(SweepEvent(s.target, event_s, pl));
  event_s->other_event = event_t;
  event_s->setLeftFlag();
  event_queue.push(event_s);
  event_queue.push(event_t);
}

// �ָ��߶�


int CP_SweepLine::possibleIntersectionForDivision(SweepEvent* ab, SweepEvent * uv) {
  auto ba = ab->other_event; // ��Ӧb
  auto vu = uv->other_event; // ��Ӧv
  // ȡ��SweepEvent�еĵ�
  CP_Point a = ab->point;
  CP_Point b = ba->point;
  CP_Point u = uv->point;
  CP_Point v = vu->point;
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
    CP_Point i = intersectPoint(a, b, u, v);
    divideSegment(ab, i, PolygonType::kPolygonResult);
    divideSegment(uv, i, PolygonType::kPolygonResult);
  }
  // һ���߶εĶ˵�����һ���߶���
  // TODO: ���߶��ص�
  // �����߶��ཻ�ڶ˵�
  // ���ཻ
  return 0;
}

void CP_SweepLine::divideSegment(SweepEvent* ab, const CP_Point& p, PolygonType type) {
  SweepEvent *re = storeSweepEvent(SweepEvent(p, ab, type));
  ab->other_event = re;
  re->setLeftFlag();

  SweepEvent *le = storeSweepEvent(SweepEvent(p, ab->other_event, type));
  ab->other_event->other_event = le;
  le->setLeftFlag();

  event_queue.push(le);
  event_queue.push(re);
}

void CP_SweepLine::subdivision(CP_Polygon & result) {
  StatusSet status_set;
  StatusSet::iterator pos, prev, next;
  SweepEvent* sweep_event;
  while (!event_queue.empty()) {
    sweep_event = event_queue.top();
    event_queue.pop();
    if (sweep_event->left) { // left point
      // ��sweep_event �������
      sweep_event->poss = pos = status_set.insert(sweep_event).first;
      next = prev = pos;
      (prev != status_set.begin()) ? --prev : prev = status_set.end();
      // set information
      if (prev == status_set.end()) {  // ��һ����
        sweep_event->inOut = sweep_event->inside = false;
      }
      else {  // ���ǵ�һ����
        if (sweep_event->polygon_type == (*prev)->polygon_type) {
          sweep_event->inside = (*prev)->inside;
          sweep_event->inOut = !(*prev)->inOut;
        }
        else {
          sweep_event->inside = (*prev)->inOut;
          sweep_event->inOut = !(*prev)->inside;
        }
      }
      // set information
      if ((++next) != status_set.end())
        possibleIntersectionForDivision(sweep_event, *next);
      if (prev != status_set.end())
        possibleIntersectionForDivision(sweep_event, *prev);
    }
    else { // right point
      next = prev = pos = sweep_event->other_event->poss;
      ++next;
      (prev != status_set.begin()) ? --prev : prev = status_set.end();
      status_set.erase(pos);
      if (prev != status_set.end() && next != status_set.end())
        possibleIntersectionForDivision(*prev, *next);
    }
    // add 
    if (sweep_event->polygon_type == PolygonType::kPolygonResult) {
      result.m_pointArray.push_back(sweep_event->point);
    }
  }
}