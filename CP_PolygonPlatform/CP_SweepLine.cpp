#include "stdafx.h"
#include "CP_SweepLine.h"
#include "CP_Connector.h"
#include <iterator>
#include <limits>

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
  if (a == b)
    return false;
  // a, b ������
  if (toLeftTest(a->point, a->other_event->point, b->point) != 0 || toLeftTest(a->point, a->other_event->point, b->other_event->point) != 0) {
    // �����˵���ȣ����Ҷ˵�Ƚ�
    if (a->point == b->point)
      return a->below(b->other_event->point);
    // ��ͬ�ĵ�
    queue_comparator comp;
    // a �Ȳ������
    if (comp(a, b))
      return b->above(a->point);
    return a->below(b->point);
  }
  else {
    if (a->point == b->point)
      return a < b;
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
  // ��ɳ�ʼ��
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
int CP_SweepLine::possibleIntersection(SweepEvent* ab, SweepEvent * uv) {
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
    divideSegment(ab, i, ab->polygon_type);
    divideSegment(uv, i, uv->polygon_type);
    return 1;
  }
  // һ���߶εĶ˵�����һ���߶���
  // TODO: ���߶��ص�
  // �����߶��ཻ�ڶ˵�
  // ���ཻ
  return 0;
}

void CP_SweepLine::divideSegment(SweepEvent* ab, const CP_Point& p, PolygonType type) {
  SweepEvent *re = storeSweepEvent(SweepEvent(p, ab, type));
  re->setLeftFlag();

  SweepEvent *le = storeSweepEvent(SweepEvent(p, ab->other_event, type));
  le->setLeftFlag();

  ab->other_event->other_event = le;
  ab->other_event = re;

  event_queue.push(le);
  event_queue.push(re);
}

int CP_SweepLine::segmentIntersect(const Segment & ab, const Segment & uv) {
  auto a = ab.source;
  auto b = ab.target;
  auto u = uv.source;
  auto v = uv.target;
  double d_abu = toLeftTest(a, b, u);
  double d_abv = toLeftTest(a, b, v);
  double d_uva = toLeftTest(u, v, a);
  double d_uvb = toLeftTest(u, v, b);
  if (Great(d_uva*d_uvb, 0.0) || Great(d_abu*d_abv, 0.0)) // û�н���
    return 0;
  if (Equal(d_abu, 0.0) && Equal(d_abv, 0.0) && Equal(d_uva, 0.0) && Equal(d_uvb, 0.0)) // ����
    return 2;
  return 1; // �����ཻ���
}

void CP_SweepLine::booleanOperation(CP_Polygon & result, OperationType op_type) {
  Connector connector;
  StatusSet status_set;
  StatusSet::iterator position, previous, next;
  SweepEvent* sweep_event;
  while (!event_queue.empty()) {
    sweep_event = event_queue.top();
    event_queue.pop();
    if (sweep_event->left) { // left point
      // ��sweep_event �������
      auto in = status_set.insert(sweep_event);
      sweep_event->poss = position = in.first;
      next = previous = position;
      (previous != status_set.begin()) ? --previous : previous = status_set.end();
      // set information
      if (previous == status_set.end()) {  // ��һ����
        sweep_event->inOut = sweep_event->inside = false;
      }
      else {  // ���ǵ�һ����
        if (sweep_event->polygon_type == (*previous)->polygon_type) {
          sweep_event->inside = (*previous)->inside;
          sweep_event->inOut = !(*previous)->inOut;
        }
        else {
          sweep_event->inside = !(*previous)->inOut;
          sweep_event->inOut = (*previous)->inside;
        }
      }
      // set information
      if ((++next) != status_set.end())
        possibleIntersection(sweep_event, *next);
      if (previous != status_set.end())
        possibleIntersection(sweep_event, *previous);
    }
    else { // right point
      next = previous = position = sweep_event->other_event->poss;
      // ������
      switch (op_type) {
        // �󽻵�
        case kSubdivision:      
          if (sweep_event->polygon_type == PolygonType::kPolygonResult)
            result.m_pointArray.push_back(sweep_event->point);
          break;
          // ��
        case kUnion:
          if (!sweep_event->other_event->inside)
            connector.add(sweep_event->segment());
          break;
        case kIntersection:
          if (sweep_event->other_event->inside)
            connector.add(sweep_event->segment());
          break;
        case kA_B:
          if ((sweep_event->polygon_type == kPolygonA && !sweep_event->other_event->inside) ||
            (sweep_event->polygon_type == kPolygonB && sweep_event->other_event->inside))
            connector.add(sweep_event->segment());
          break;
        case kB_A:
          if ((sweep_event->polygon_type == kPolygonB && !sweep_event->other_event->inside) ||
            (sweep_event->polygon_type == kPolygonA && sweep_event->other_event->inside))
            connector.add(sweep_event->segment());
          break;
        default:
          break;
      }
      ++next;
      (previous != status_set.begin()) ? --previous : previous = status_set.end();
      status_set.erase(position);
      if (previous != status_set.end() && next != status_set.end())
        possibleIntersection(*previous, *next);
    } 
  } 
  if (op_type != kSubdivision) {
    connector.toPolygon(result);
  }
}

void CP_SweepLine::clear() {
  event_holder.clear();
  event_queue = {};
}

bool CP_SweepLine::check(const CP_Polygon & polygon) {
  // step 1: �ж��Ƿ���ڱ��ཻ�����ص�
  initializeQueue(polygon, PolygonType::kPolygonResult);
  StatusSet status_set;
  StatusSet::iterator position, previous, next;
  SweepEvent* sweep_event;
  int self_intersect = 0;
  while (!event_queue.empty()) {
    sweep_event = event_queue.top();
    event_queue.pop();
    if (sweep_event->left) { // left point
      // ��sweep_event �������
      auto in = status_set.insert(sweep_event);
      sweep_event->poss = position = in.first;
      next = previous = position;
      (previous != status_set.begin()) ? --previous : previous = status_set.end();
      if ((++next) != status_set.end()) {
        self_intersect += possibleIntersection(sweep_event, *next);
      }  
      if (previous != status_set.end()) {
        self_intersect += possibleIntersection(sweep_event, *previous);
      }
    }
    else { // right point
      next = previous = position = sweep_event->other_event->poss;
      ++next;
      (previous != status_set.begin()) ? --previous : previous = status_set.end();
      status_set.erase(position);
      if (previous != status_set.end() && next != status_set.end()) {
        self_intersect += possibleIntersection(*previous, *next);
      }
    }
  }
  if (self_intersect > 0)
    return false;
  clear();
  // end step 1 ���Ӷ�O((N+k)logN)

  // step 2: �жϻ���λ�úϷ���
  // �ҵ������ϵ㣬����һ�����ϵ������������ཻ
  // �⻷�Ľ������Ϊż�����ڻ��Ľ������Ϊ����
  // TO_OPTIMIZE: ���ж�λ�úϷ��Ե��㷨�ϲ���ɨ�����㷨�У���ʱ�临�ӶȽ���O(NlogN)
  for (auto region : polygon.m_regionArray) {
    for (auto loop : region.m_loopArray) {
      // �ж��Ƿ����⻷
      bool isOut = loop.m_loopIDinRegion == 0;
      // �ҵ����Ķ��˵�
      double max_y = -1e18, max_x = 0;
      for (auto id : loop.m_pointIDArray) {
        if (polygon.m_pointArray[id].m_y > max_y) {
          max_y = polygon.m_pointArray[id].m_y;
          max_x = polygon.m_pointArray[id].m_x;
        }
      }
      CP_Point top_point(max_x, max_y);
      Segment ray(top_point, CP_Point(max_x, 1e18));
      // �ж����������εĽ�����
      int intersect_num = 0;
      int loop_first_id = 0;
      int loop_size = 0;
      int point_id = 0;
      // ��������ε���������
      for (auto region_2 : polygon.m_regionArray) {
        // ���������е����л�
        for (int l = 0; l < region_2.m_loopArray.size(); ++l) {
          auto loop_2 = region_2.m_loopArray[l];
          loop_first_id = loop_2.m_pointIDArray[0];
          loop_size = loop_2.m_pointIDArray.size();
          for (int i = 0; i < loop_size; ++i) {
            auto source = polygon.m_pointArray[loop_first_id + i % loop_size];
            auto target = polygon.m_pointArray[loop_first_id + (i + 1) % loop_size];
            // �����߶ζ�Ӧ�� SweepEvent ���뵽EventQueue �С�
            Segment s(source, target);
            // ���㽻��
            if (source != top_point || target != top_point) {
              intersect_num += segmentIntersect(s, ray);
            }
            ++point_id;
          }
        }
      } // end 
      int a = 1;
      if (isOut && intersect_num % 2 == 1) // ������⻷�ҽ���Ϊ����
        return false;
      if (!isOut && intersect_num % 2 == 0) // ������ڻ��ҽ���Ϊż��
        return false;
      // end �������
    } // end loop
  } // er
  // end step 2 ���Ӷ�O(n^2)
  return true;
}
