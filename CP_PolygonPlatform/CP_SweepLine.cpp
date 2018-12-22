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
        auto & source = polygon.m_pointArray[loop_first_id + d * (i % loop_size)];
        auto & target = polygon.m_pointArray[loop_first_id + d * ((i + 1) % loop_size)];
        // 初始化SweepEvent
        SweepEvent event_s(source, type), event_t(target, type);
        event_s.other_event = std::make_shared<SweepEvent>(event_t);
        event_t.other_event = std::make_shared<SweepEvent>(event_s);
        event_s.other_event = std::make_shared<SweepEvent>(event_t);
        event_t.other_event = std::make_shared<SweepEvent>(event_s);
        // 判断哪个是左边的点
        event_s.setLeftFlag();
        event_t.setLeftFlag();
        // 加入队列中
        event_queue.push(event_s);
        event_queue.push(event_t);
        ++point_id;
      }
    }
  }
}

// 第一阶段算法：分割边
// TODO: 实现setInformation 和 possibleIntersection
void subdivision(EventQueue & event_queue, CP_Polygon & result) {
  StatusSet status_set;
  while (!event_queue.empty()) {
    auto sweep_event = event_queue.top();
    event_queue.pop();
    if (sweep_event.left) { // left endpoint
      auto position_it = status_set.insert(sweep_event).first;
      if (position_it != status_set.end()) {
        // 用拷贝的迭代器替代position_it
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
      if (position_it != status_set.end()) { // 点在status_set中
        // 用拷贝的迭代器替代position_it
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

// 设置事件的信息
void setInformation(SweepEvent & this_event, SweepEvent & other_event) {
  
}

// to-left-test
double toLeftTest(const CP_Point& a, const CP_Point& b, const CP_Point& p) {
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

// 将事件移出StatusSet 
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

// 两线段交与线段中，修改 EventQueue 和 StatusSet
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

// 一线段的端点在另一线段上，处理该情况的事件，修改 EventQueue 和 StatusSet
int intersectAtEndpoint(SweepEvent & intersect, SweepEvent& endpoint1, SweepEvent& endpoint2, EventQueue & event_queue, StatusSet & status_set) {
  // 生成两个事件，和线段端点关联，并插入队列中
  if (intersect.polygon_type != PolygonType::kPolygonResult) { // 必须不是相交的交点
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

// 计算可能存在的线段相交
// 采用下面这篇论文中提供的线段求交算法
// Bennellabc J A. The geometry of nesting problems: A tutorial[J]. European Journal of Operational Research, 2008, 184(2):397-415.
// D-function 为toLeftTest方法
int possibleIntersection(SweepEvent & ab, SweepEvent & uv, EventQueue & event_queue, StatusSet & status_set) {
  auto ba = ab.other_event; // 对应b
  auto vu = uv.other_event; // 对应v
  // 取出SweepEvent中的点
  CP_Point a = *ab.point;
  CP_Point b = *ba->point;
  CP_Point u = *uv.point;
  CP_Point v = *vu->point;
  // 计算D-function
  double d_abu = toLeftTest(a, b, u);
  double d_abv = toLeftTest(a, b, v);
  double d_uva = toLeftTest(u, v, a);
  double d_uvb = toLeftTest(u, v, b);
  // 分情况讨论相交情况
  // 相交于两线段中
  // 求交算法https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
  // d_abu != 0 && d_abv != 0 && d_uva != 0 && d_uvb != 0
  if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && NotEqual(d_uvb, 0.0)
      && Less(d_abu * d_abv, 0.0) && Less(d_uva * d_uvb, 0.0)) { 
    // 将事件ab 和 uv 移出 StatusSet
    removeFromStatusSet(ab, uv, status_set);
    // 求出交点
    CP_Point i = intersectPoint(a, b, u, v);
    // 修改SweepEvent 和 Status_set,将其插入队列和状态树中(插入四个事件）
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
  // TODO：一条线段的端点在另一条线段上
  // u 在 ab 上
  if (Equal(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && NotEqual(d_uvb, 0.0) && NotEqual(d_uva, d_uvb)) {
    removeFromStatusSet(ab, uv, status_set);
    return intersectAtEndpoint(uv, ab, *ba, event_queue, status_set);
  }
  // v 在 ab 上
  if (NotEqual(d_abu, 0.0) && Equal(d_abv, 0.0) && NotEqual(d_uva, 0, 0) && NotEqual(d_uvb, 0.0) && NotEqual(d_uva, d_uvb)) {
    removeFromStatusSet(ab, uv, status_set);
    return intersectAtEndpoint(*vu, ab, *ba, event_queue, status_set);
  }
  // a 在 uv 上
  if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && Equal(d_uva, 0.0) && NotEqual(d_uvb, 0.0) && NotEqual(d_abu, d_abv)) {
    removeFromStatusSet(ab, uv, status_set);
    return intersectAtEndpoint(ab, uv, *vu, event_queue, status_set);
  }
  // b 在 uv 上
  if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && Equal(d_uvb, 0.0) && NotEqual(d_abu, d_abv)) {
    removeFromStatusSet(ab, uv, status_set);
    return intersectAtEndpoint(*ba, uv, *vu, event_queue, status_set);
  }
  // TODO: 两线段重叠
  // 两条线段相交于端点
  // 不相交
  return 0;
}