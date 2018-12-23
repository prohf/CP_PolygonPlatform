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
        // �ж��ĸ�����ߵĵ�
        event_s.setLeftFlag();
        event_t.setLeftFlag();
        // ���������
        event_queue.insert(event_s);
        event_queue.insert(event_t);
        ++point_id;
      }
    }
  }
}

// �����¼�����Ϣ
void setInformation(SweepEvent & pos, SweepEvent & prev) {
  if (pos.polygon_type == prev.polygon_type) {
    pos.inside = prev.inside;
    pos.inOut = !prev.inOut;
  }
  else {
    pos.inside = !prev.inOut;
    pos.inOut = prev.inside;
  }
}

// ��һ�׶��㷨���ָ��
// TODO: ʵ��setInformation �� possibleIntersection
void booleanOperation(EventQueue & event_queue, CP_Polygon & result, OperationType type) {
  StatusSet status_set;
  Connector connector;
  while (!event_queue.empty()) {
    //event_queue.top()
    auto sweep_event_it = event_queue.begin();
    auto sweep_event = *sweep_event_it;
    //event_queue.pop();
    event_queue.erase(sweep_event_it);
    if (sweep_event.left) { // left endpoint
      auto position_it = status_set.insert(sweep_event).first;
      if (position_it != status_set.end()) {
        // �ÿ����ĵ��������position_it
        auto position = *position_it;
        SweepEvent previous, next;
        bool prev_flag = false, next_flag = false;
        if (position_it != std::prev(status_set.end())) {
          next = *std::next(position_it);
          next_flag = true;
          std::prev(position_it);
        }
        if (position_it != status_set.begin()) {
          previous = *std::prev(position_it);
          prev_flag = true;
          std::next(position_it);
        }
        // setInformation
        if (!prev_flag) {
          position.inside = position.inOut = false;
        }
        else {
          setInformation(position, previous);
        }
        // ���²���
        position_it = status_set.find(position);
        status_set.erase(position_it);
        status_set.insert(position);
        // setInformation
        if (next_flag)
          possibleIntersection(position, next, event_queue, status_set);
        if (prev_flag)
          possibleIntersection(position, previous, event_queue, status_set);

      }
    }
    else { // right endpoint
      auto position_it = status_set.find(*sweep_event.other_event);
      if (position_it != status_set.end()) { // ����status_set��
        // �жϲ�����������
        switch (type) {
          case kUnion:
            if (!sweep_event.other_event->inside) {
              Segment s(*sweep_event.other_event);
              connector.add(s);
            }
            break;
          case kIntersection:
            if (sweep_event.other_event->inside) {
              Segment s(*sweep_event.other_event);
              connector.add(s);
            }
            break;
          case kA_B:
            if ((sweep_event.polygon_type == kPolygonA && !sweep_event.other_event->inside)
              || (sweep_event.polygon_type == kPolygonB && sweep_event.other_event->inside)) {
              Segment s(*sweep_event.other_event);
              connector.add(s);
            }
            break;
          case kB_A:
            if ((sweep_event.polygon_type == kPolygonB && !sweep_event.other_event->inside)
              || (sweep_event.polygon_type == kPolygonA && sweep_event.other_event->inside)) {
              Segment s(*sweep_event.other_event);
              connector.add(s);
            }
          default:
            break;
        }
        // �ÿ����ĵ��������position_it
        if (position_it != status_set.begin() && position_it != std::prev(status_set.end())) {
          auto previous = *std::prev(position_it);
          std::next(position_it);
          auto next = *std::next(position_it);
          status_set.erase(position_it);
          possibleIntersection(previous, next, event_queue, status_set);
        }
        else {
          status_set.erase(position_it);
        }
      }
    }
  }
  connector.toPolygon(result);
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
  auto ab_iter = status_set.find(ab);
  if (ab_iter != status_set.end()) {
    status_set.erase(ab_iter);
  }
  auto uv_iter = status_set.find(uv);
  if (uv_iter != status_set.end()) {
    status_set.erase(uv_iter);
  }
}

void modifyEventQueue(SweepEvent & other, SweepEvent & intersect, EventQueue & event_queue, StatusSet & status_set) {
  intersect.other_event = std::make_shared<SweepEvent>(other);
  auto iter = event_queue.find(other);
  if (iter != event_queue.end()) {
    event_queue.erase(iter);
    other.other_event = std::make_shared<SweepEvent>(intersect);
    intersect.other_event = std::make_shared<SweepEvent>(other);
    other.setLeftFlag();
    event_queue.insert(other);
    return;
  }
  else {
    other.other_event = std::make_shared<SweepEvent>(intersect);
    intersect.other_event = std::make_shared<SweepEvent>(other);
    other.setLeftFlag();
    auto position_it = status_set.insert(other).first;
    auto position = *position_it;
    if (position_it == status_set.begin()) {
      position.inOut = position.inside = false;
    }
    else {
      auto previous = *std::prev(position_it);
      setInformation(position, previous);
      std::next(position_it);
    }
    // ���²���
    position_it = status_set.find(position);
    //if (position_it != status_set.end()) {
      status_set.erase(position_it);
      status_set.insert(position);
    //}
    return;
  }
}

// ���߶ν����߶��У��޸� EventQueue �� StatusSet
int intersectAtCrossPoint(SweepEvent & intersect, SweepEvent & other, EventQueue & event_queue, StatusSet & status_set) {
  //intersect.other_event = std::make_shared<SweepEvent>(other);
  //other.other_event = std::make_shared<SweepEvent>(intersect);
  modifyEventQueue(other, intersect, event_queue, status_set);

  intersect.setLeftFlag();

  // event_queue.push(intersect)
  event_queue.insert(intersect);
  //if (other.left) {
  //  status_set.insert(other);
  //}
  return 1;
}

//// һ�߶εĶ˵�����һ�߶��ϣ������������¼����޸� EventQueue �� StatusSet
//int intersectAtEndpoint(SweepEvent & intersect, SweepEvent& endpoint1, SweepEvent& endpoint2, EventQueue & event_queue, StatusSet & status_set) {
//  // ���������¼������߶ζ˵�����������������
//  if (intersect.polygon_type != PolygonType::kPolygonResult) { // ���벻���ཻ�Ľ���
//    CP_Point point = *intersect.point;
//    SweepEvent intersect1(point, PolygonType::kPolygonResult), intersect2(point, PolygonType::kPolygonResult);
//
//    //intersect1.other_event = std::make_shared<SweepEvent>(endpoint1);
//    //endpoint1.other_event = std::make_shared<SweepEvent>(intersect1);
//    modifyEventQueue(endpoint1, intersect1, event_queue, status_set);
//
//    //intersect2.other_event = std::make_shared<SweepEvent>(endpoint2);
//    //endpoint2.other_event = std::make_shared<SweepEvent>(intersect2);
//    modifyEventQueue(endpoint2, intersect2, event_queue, status_set);
//
//    intersect1.setLeftFlag();
//    intersect2.setLeftFlag();
//    //endpoint1.setLeftFlag();
//    //endpoint2.setLeftFlag();
//    // event_queue.push(intersect1)
//    event_queue.insert(intersect1);
//    // event_queue.push(intersect2)
//    event_queue.insert(intersect2);
//    //if (endpoint1.left) {
//    //  status_set.insert(endpoint1);
//    //}
//    //if (endpoint2.left) {
//    //  status_set.insert(endpoint2);
//    //}
//    return 1;
//  }
//  return 0;
//}

// ������ܴ��ڵ��߶��ཻ
// ����������ƪ�������ṩ���߶����㷨
// Bennellabc J A. The geometry of nesting problems: A tutorial[J]. European Journal of Operational Research, 2008, 184(2):397-415.
// D-function ΪtoLeftTest����
int possibleIntersection(SweepEvent & ab, SweepEvent & uv, EventQueue & event_queue, StatusSet & status_set) {
  auto& ba = *ab.other_event; // ��Ӧb
  auto& vu = *uv.other_event; // ��Ӧv
  // ȡ��SweepEvent�еĵ�
  CP_Point a = *ab.point;
  CP_Point b = *ba.point;
  CP_Point u = *uv.point;
  CP_Point v = *vu.point;
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
    //SweepEvent event_i_a(i, PolygonType::kPolygonResult);
    SweepEvent event_i_a(i, ab.polygon_type);
    intersectAtCrossPoint(event_i_a, ab, event_queue, status_set);
    // i<->b
    //SweepEvent event_i_b(i,PolygonType::kPolygonResult);
    SweepEvent event_i_b(i, ba.polygon_type);
    intersectAtCrossPoint(event_i_b, ba, event_queue, status_set);
    // i<->u
    //SweepEvent event_i_u(i, PolygonType::kPolygonResult);
    SweepEvent event_i_u(i, uv.polygon_type);
    intersectAtCrossPoint(event_i_u, uv, event_queue, status_set);
    // i<->v
    //SweepEvent event_i_v(i, PolygonType::kPolygonResult);
    SweepEvent event_i_v(i, vu.polygon_type);
    intersectAtCrossPoint(event_i_v, vu, event_queue, status_set);
    return 1;
  }
  // TODO��һ���߶εĶ˵�����һ���߶���
  // u �� ab ��
  //if (Equal(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && NotEqual(d_uvb, 0.0) && Less(d_uva * d_uvb, 0.0)) {
  //  removeFromStatusSet(ab, uv, status_set);
  //  return intersectAtEndpoint(uv, ab, ba, event_queue, status_set);
  //}
  //// v �� ab ��
  //if (NotEqual(d_abu, 0.0) && Equal(d_abv, 0.0) && NotEqual(d_uva, 0, 0) && NotEqual(d_uvb, 0.0) && Less(d_uva * d_uvb, 0.0)) {
  //  removeFromStatusSet(ab, uv, status_set);
  //  return intersectAtEndpoint(vu, ab, ba, event_queue, status_set);
  //}
  //// a �� uv ��
  //if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && Equal(d_uva, 0.0) && NotEqual(d_uvb, 0.0) && Less(d_abu * d_abv, 0.0)) {
  //  removeFromStatusSet(ab, uv, status_set);
  //  return intersectAtEndpoint(ab, uv, vu, event_queue, status_set);
  //}
  //// b �� uv ��
  //if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && Equal(d_uvb, 0.0) && Less(d_abu * d_abv, 0.0)) {
  //  removeFromStatusSet(ab, uv, status_set);
  //  return intersectAtEndpoint(ba, uv, vu, event_queue, status_set);
  //}
  // TODO: ���߶��ص�
  // �����߶��ཻ�ڶ˵�
  // ���ཻ
  return 0;
}

int possibleIntersectionForDivision(SweepEvent & ab, SweepEvent & uv, EventQueue & event_queue, StatusSet & status_set) {
  auto& ba = *ab.other_event; // ��Ӧb
  auto& vu = *uv.other_event; // ��Ӧv
  // ȡ��SweepEvent�еĵ�
  CP_Point a = *ab.point;
  CP_Point b = *ba.point;
  CP_Point u = *uv.point;
  CP_Point v = *vu.point;
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
    intersectAtCrossPoint(event_i_a, ab, event_queue, status_set);
    // i<->b
    SweepEvent event_i_b(i,PolygonType::kPolygonResult);
    intersectAtCrossPoint(event_i_b, ba, event_queue, status_set);
    // i<->u
    SweepEvent event_i_u(i, PolygonType::kPolygonResult);
    intersectAtCrossPoint(event_i_u, uv, event_queue, status_set);
    // i<->v
    SweepEvent event_i_v(i, PolygonType::kPolygonResult);
    intersectAtCrossPoint(event_i_v, vu, event_queue, status_set);
    return 1;
  }
  // TODO��һ���߶εĶ˵�����һ���߶���
  // TODO: ���߶��ص�
  // �����߶��ཻ�ڶ˵�
  // ���ཻ
  return 0;
}

void subdivision(EventQueue & event_queue, CP_Polygon & result) {
  StatusSet status_set;
  while (!event_queue.empty()) {
    //event_queue.top()
    auto sweep_event_it = event_queue.begin();
    auto sweep_event = *sweep_event_it;
    //event_queue.pop();
    event_queue.erase(sweep_event_it);
    if (sweep_event.left) { // left endpoint
      auto position_it = status_set.insert(sweep_event).first;
      if (position_it != status_set.end()) {
        // �ÿ����ĵ��������position_it
        auto position = *position_it;
        SweepEvent previous, next;
        bool prev_flag = false, next_flag = false;
        if (position_it != std::prev(status_set.end())) {
          next = *std::next(position_it);
          next_flag = true;
          std::prev(position_it);
        }
        if (position_it != status_set.begin()) {
          previous = *std::prev(position_it);
          prev_flag = true;
          std::next(position_it);
        }
        if (next_flag)
          possibleIntersectionForDivision(position, next, event_queue, status_set);
        if (prev_flag)
          possibleIntersectionForDivision(position, previous, event_queue, status_set);

      }
    }
    else { // right endpoint
      auto position_it = status_set.find(*sweep_event.other_event);
      if (position_it != status_set.end()) { // ����status_set��
        // �ÿ����ĵ��������position_it
        if (position_it != status_set.begin() && position_it != std::prev(status_set.end())) {
          auto previous = *std::prev(position_it);
          std::next(position_it);
          auto next = *std::next(position_it);
          status_set.erase(position_it);
          possibleIntersectionForDivision(previous, next, event_queue, status_set);
        }
        else {
          status_set.erase(position_it);
        }
      }
    }
    // add 
    if (sweep_event.polygon_type == PolygonType::kPolygonResult) {
      result.m_pointArray.push_back(*sweep_event.point);
    }
  }
}

void PointChain::init(const Segment & s) {
  point_list.push_back(s.source);
  point_list.push_back(s.target);
}

bool PointChain::LinkSegment(const Segment& s) {
  if (s.source == point_list.front()) {
    if (s.target == point_list.back())
      is_closed = true;
    else
      point_list.push_front(s.target);
    return true;
  }
  if (s.target == point_list.back()) {
    if (s.source == point_list.front())
      is_closed = true;
    else
      point_list.push_back(s.source);
    return true;
  }
  if (s.target == point_list.front()) {
    if (s.source == point_list.back())
      is_closed = true;
    else
      point_list.push_front(s.source);
    return true;
  }
  if (s.source == point_list.back()) {
    if (s.target == point_list.front())
      is_closed = true;
    else
      point_list.push_back(s.target);
    return true;
  }
  return false;
}

bool PointChain::LinkPointChain(PointChain& chain) {
  if (chain.point_list.front() == point_list.back()) {
    chain.point_list.pop_front();
    point_list.splice(point_list.end(), chain.point_list);
    return true;
  }
  if (chain.point_list.back() == point_list.front()) {
    point_list.pop_front();
    point_list.splice(point_list.begin(), chain.point_list);
    return true;
  }
  if (chain.point_list.front() == point_list.front()) {
    point_list.pop_front();
    reverse(chain.point_list.begin(), chain.point_list.end());
    point_list.splice(point_list.begin(), chain.point_list);
    return true;
  }
  if (chain.point_list.back() == point_list.back()) {
    point_list.pop_back();
    reverse(chain.point_list.begin(), chain.point_list.end());
    point_list.splice(point_list.end(), chain.point_list);
    return true;
  }
  return false;
}

void Connector::add(const Segment& s) {
  iterator j = openPolygons.begin();
  while (j != openPolygons.end()) {
    if (j->LinkSegment(s)) {
      if (j->is_closed)
        closedPolygons.splice(closedPolygons.end(), openPolygons, j);
      else {
        list<PointChain>::iterator k = j;
        for (++k; k != openPolygons.end(); ++k) {
          if (j->LinkPointChain(*k)) {
            openPolygons.erase(k);
            break;
          }
        }
      }
      return;
    }
    ++j;
  }
  // The segment cannot be connected with any open polygon
  openPolygons.push_back(PointChain());
  openPolygons.back().init(s);
}

void Connector::toPolygon(CP_Polygon & polygon) {
  int point_id = 0, region_id = 0;
  for (iterator it = begin(); it != end(); ++it) {
    CP_Region region;
    region.m_polygon = &polygon;
    region.m_regionIDinPolygon = region_id;
    CP_Loop loop;
    loop.m_polygon = &polygon;
    loop.m_loopIDinRegion = 0;
    loop.m_regionIDinPolygon = region_id;
    for (PointChain::point_iter it2 = it->begin(); it2 != it->end(); ++it2) {
      polygon.m_pointArray.push_back(*it2);
      loop.m_pointIDArray.push_back(point_id);
      ++point_id;
    }
    region.m_loopArray.push_back(loop);
    polygon.m_regionArray.push_back(region);
    ++region_id;
  }
}