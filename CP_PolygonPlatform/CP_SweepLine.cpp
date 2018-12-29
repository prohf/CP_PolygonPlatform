#include "stdafx.h"
#include "CP_SweepLine.h"
#include "CP_Connector.h"
#include <iterator>

// to-left-test
inline double toLeftTest(const CP_Point& a, const CP_Point& b, const CP_Point& p) {
  return (a.m_x - b.m_x) * (a.m_y - p.m_y) - (a.m_y - b.m_y) * (a.m_x - p.m_x);
}

// 求两线段交点函数
CP_Point intersectPoint(const CP_Point & a, const CP_Point & b, const CP_Point & u, const CP_Point & v) {
  // 求出交点
  // b = a + p, v = u + q
  CP_Vector p(b.m_x - a.m_x, b.m_y - a.m_y);
  CP_Vector q(v.m_x - u.m_x, v.m_y - u.m_y);
  // i = a + s*p = u + t*q
  // s = (u - a) cross q / p cross q 
  double p_cross_q = p.m_x * q.m_y - p.m_y * q.m_x;
  CP_Vector u_a(u.m_x - a.m_x, u.m_y - a.m_y); // u - a
  double u_a_cross_q = u_a.m_x * q.m_y - u_a.m_y * q.m_x;
  double s = u_a_cross_q / p_cross_q;
  return CP_Point(a.m_x + s * p.m_x, a.m_y + s * p.m_y); // 交点为i
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
  // other_event中较低的那个
  return a->above(b->other_event->point);
}

bool status_comparator::operator()(const SweepEvent* a, const SweepEvent* b) const {
  if (a == b)
    return false;
  // a, b 不共线
  if (toLeftTest(a->point, a->other_event->point, b->point) != 0 || toLeftTest(a->point, a->other_event->point, b->other_event->point) != 0) {
    // 如果左端点相等，用右端点比较
    if (a->point == b->point)
      return a->below(b->other_event->point);
    // 不同的点
    queue_comparator comp;
    // a 先插入队列
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
实现 CP_SweepLine 方法
***********************************************************************/
void CP_SweepLine::initializeQueue(const CP_Polygon & polygon, PolygonType type) {
  // TODO: 完成初始化
  int loop_first_id = 0;
  int loop_size = 0;
  int point_id = 0;
  // 遍历多边形的所有区域
  for (auto region : polygon.m_regionArray) {
    // 遍历区域中的所有环
    for (int l = 0; l < region.m_loopArray.size(); ++l) {
      auto loop = region.m_loopArray[l];
      loop_first_id = loop.m_pointIDArray[0];
      loop_size = loop.m_pointIDArray.size();
      int d = loop_first_id == point_id ? 1: -1;
      for (int i = 0; i < loop_size; ++i) {
        auto source = polygon.m_pointArray[loop_first_id + d * (i % loop_size)];
        auto target = polygon.m_pointArray[loop_first_id + d * ((i + 1) % loop_size)];
        // 将该线段对应的 SweepEvent 加入到EventQueue 中。
        Segment s(source, target);
        processSegment(s, type);
        ++point_id;
      }
    }
  }
}

// 处理线段事件
void CP_SweepLine::processSegment(const Segment& s, PolygonType pl) {
  SweepEvent *event_s = storeSweepEvent(SweepEvent(s.source, nullptr, pl));
  SweepEvent *event_t = storeSweepEvent(SweepEvent(s.target, event_s, pl));
  event_s->other_event = event_t;
  event_s->setLeftFlag();
  event_queue.push(event_s);
  event_queue.push(event_t);
}

// 分割线段


int CP_SweepLine::possibleIntersection(SweepEvent* ab, SweepEvent * uv) {
  auto ba = ab->other_event; // 对应b
  auto vu = uv->other_event; // 对应v
  // 取出SweepEvent中的点
  CP_Point a = ab->point;
  CP_Point b = ba->point;
  CP_Point u = uv->point;
  CP_Point v = vu->point;
  // 计算D-function
  double d_abu = toLeftTest(a, b, u);
  double d_abv = toLeftTest(a, b, v);
  double d_uva = toLeftTest(u, v, a);
  double d_uvb = toLeftTest(u, v, b);
  if (a == u || b == v) {
    return 1;
  }
  // 分情况讨论相交情况
  // 相交于两线段中
  // 求交算法https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
  // d_abu != 0 && d_abv != 0 && d_uva != 0 && d_uvb != 0
  if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && NotEqual(d_uvb, 0.0)
    && Less(d_abu * d_abv, 0.0) && Less(d_uva * d_uvb, 0.0)) {
    CP_Point i = intersectPoint(a, b, u, v);
    divideSegment(ab, i, ab->polygon_type);
    divideSegment(uv, i, uv->polygon_type);
  }
  // 一条线段的端点在另一条线段上
  // TODO: 两线段重叠
  // 两条线段相交于端点
  // 不相交
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

void CP_SweepLine::booleanOperation(CP_Polygon & result, OperationType op_type) {
  Connector connector;
  StatusSet status_set;
  StatusSet::iterator position, previous, next;
  SweepEvent* sweep_event;
  while (!event_queue.empty()) {
    sweep_event = event_queue.top();
    event_queue.pop();
    if (sweep_event->left) { // left point
      // 将sweep_event 插入队列
      auto in = status_set.insert(sweep_event);
      sweep_event->poss = position = in.first;
      next = previous = position;
      (previous != status_set.begin()) ? --previous : previous = status_set.end();
      // set information
      if (previous == status_set.end()) {  // 第一个点
        sweep_event->inOut = sweep_event->inside = false;
      }
      else {  // 不是第一个点
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
      // 处理结果
      switch (op_type) {
        // 求交点
        case kSubdivision:      
          if (sweep_event->polygon_type == PolygonType::kPolygonResult)
            result.m_pointArray.push_back(sweep_event->point);
          break;
          // 并
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