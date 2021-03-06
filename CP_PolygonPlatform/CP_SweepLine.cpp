#include "stdafx.h"
#include "CP_SweepLine.h"
#include "CP_Connector.h"
#include <iterator>
#include <limits>

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
  // 处理重叠边的情况
  if (a->point == b->point && a->other_event->point == b->other_event->point)
    return a->polygon_type < b->polygon_type;
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
    if (a->point == b->point) // 处理重叠边的情况
      return a->polygon_type < b->polygon_type;
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
布尔运算部分
***********************************************************************/
void CP_SweepLine::initializeQueue(const CP_Polygon & polygon, PolygonType type) {
  // 完成初始化
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
  // 分情况讨论相交情况
  // 相交于两线段中
  // 求交算法https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
  // d_abu != 0 && d_abv != 0 && d_uva != 0 && d_uvb != 0
  if (NotEqual(d_abu, 0.0) && NotEqual(d_abv, 0.0) && NotEqual(d_uva, 0.0) && NotEqual(d_uvb, 0.0)
    && Less(d_abu * d_abv, 0.0) && Less(d_uva * d_uvb, 0.0)) {
    CP_Point i = intersectPoint(a, b, u, v);
    divideSegment(ab, i);
    divideSegment(uv, i);
    return 1;
  }
  // TODO: 两线段重叠
  // 这段代码的实现参考了作者源码
  if (Equal(d_abu, 0.0) && Equal(d_abv, 0.0) /*&& Equal(d_uva, 0.0) && Equal(d_uvb, 0.0)*/) {
    if (ab->point == vu->point || ba->point == uv->point)
      return 0;
    std::vector<SweepEvent*> sortedEvents;
    queue_comparator cmp;
    if (ab->point == uv->point) {
      sortedEvents.push_back(0);
    }
    else if (cmp(ab, uv)) {
      sortedEvents.push_back(uv);
      sortedEvents.push_back(ab);
    }
    else {
      sortedEvents.push_back(ab);
      sortedEvents.push_back(uv);
    }
    if (ba->point == vu->point) {
      sortedEvents.push_back(0);
    }
    else if (cmp(ba, vu)) {
      sortedEvents.push_back(vu);
      sortedEvents.push_back(ba);
    }
    else {
      sortedEvents.push_back(ba);
      sortedEvents.push_back(vu);
    }
    if (sortedEvents.size() == 2) { // 两线段重叠
      ab->edge_type = ba->edge_type = kNon_contributing;
      uv->edge_type = vu->edge_type = (ab->inOut == uv->inOut) ? kSame_trasition : kDifferent_trasition;
      return 0;
    }
    if (sortedEvents.size() == 3) { // 两线段有相同端点
      sortedEvents[1]->edge_type = sortedEvents[1]->other_event->edge_type = kNon_contributing;
      if (sortedEvents[0])  // 右端点重合？
        sortedEvents[0]->other_event->edge_type = (ab->inOut == uv->inOut) ? kSame_trasition : kDifferent_trasition;
      else // 左端点重合
        sortedEvents[2]->other_event->edge_type = (ab->inOut == uv->inOut) ? kSame_trasition : kDifferent_trasition;
      divideSegment(sortedEvents[0] ? sortedEvents[0] : sortedEvents[2]->other_event, sortedEvents[1]->point);
      return 0;
    }
    if (sortedEvents[0] != sortedEvents[3]->other_event) { // 两线段不完全覆盖
      sortedEvents[1]->edge_type = kNon_contributing;
      sortedEvents[2]->edge_type = (ab->inOut == uv->inOut) ? kSame_trasition : kDifferent_trasition;
      divideSegment(sortedEvents[0], sortedEvents[1]->point);
      divideSegment(sortedEvents[1], sortedEvents[2]->point);
      return 0;
    }
    // 一条线段包含另一条线段
    sortedEvents[1]->edge_type = sortedEvents[1]->other_event->edge_type = kNon_contributing;
    divideSegment(sortedEvents[0], sortedEvents[1]->point);
    sortedEvents[3]->edge_type = (ab->inOut == uv->inOut) ? kSame_trasition : kDifferent_trasition;
    divideSegment(sortedEvents[3]->other_event, sortedEvents[2]->point);
    return 0;
  }
  // 两条线段相交于端点
  // 不相交
  return 0;
}

void CP_SweepLine::divideSegment(SweepEvent* ab, const CP_Point& p) {
  SweepEvent *re = storeSweepEvent(SweepEvent(p, ab, ab->polygon_type));
  re->setLeftFlag();

  SweepEvent *le = storeSweepEvent(SweepEvent(p, ab->other_event, ab->polygon_type));
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
      sweep_event->poss = position = status_set.insert(sweep_event).first;
      next = previous = position;
      (previous != status_set.begin()) ? --previous : previous = status_set.end();
      // setInformation
      if (previous == status_set.end()) {  // 第一个点
        sweep_event->inOut = sweep_event->inside = false;
      }
      else {  // 不是第一个点
        // 是否是重叠边
        // 这段实现参考作者源码
        if ((*previous)->edge_type != kNormal) {
          if (previous == status_set.begin()) {
            sweep_event->inside = true;
            sweep_event->inOut = false;
          }
          else {
            auto tmp = previous;
            --tmp;
            if ((*previous)->polygon_type == sweep_event->polygon_type) {
              sweep_event->inOut = !(*previous)->inOut;
              sweep_event->inside = !(*tmp)->inOut;
            }
            else {
              sweep_event->inOut = !(*tmp)->inOut;
              sweep_event->inside = !(*previous)->inOut;
            }
          }
        }
        else { // 不是重叠边，正常计算
          if (sweep_event->polygon_type == (*previous)->polygon_type) {
            sweep_event->inside = (*previous)->inside;
            sweep_event->inOut = !(*previous)->inOut;
          }
          else {
            sweep_event->inside = !(*previous)->inOut;
            sweep_event->inOut = (*previous)->inside;
          }
        }
      }
      // end setInformation
      if ((++next) != status_set.end())
        possibleIntersection(sweep_event, *next);
      if (previous != status_set.end())
        possibleIntersection(sweep_event, *previous);
    }
    else { // right point
      next = previous = position = sweep_event->other_event->poss;
      // 处理结果(添加重叠边处理）
      switch (sweep_event->edge_type) {
      case kNormal:
        switch (op_type) {
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
        case kXOR:
          connector.add(sweep_event->segment());
          break;
        default:
          break;
        }
        break;
      case kSame_trasition:
        if (op_type == kUnion || op_type == kIntersection)
          connector.add(sweep_event->segment());
        break;
      case kDifferent_trasition:
        if (op_type == kA_B || op_type == kB_A)
          connector.add(sweep_event->segment());
        break;
      }
      ++next;
      (previous != status_set.begin()) ? --previous : previous = status_set.end();
      status_set.erase(position);
      if (previous != status_set.end() && next != status_set.end())
        possibleIntersection(*previous, *next);
    }
  }
  clear();
  connector.toPolygon(result);
  processLoop(result);
}

void CP_SweepLine::clear() {
  event_holder.clear();
  event_queue = {};
}

/***************************************************
合法性检验与多边形结果生成
***************************************************/

void CP_SweepLine::processSegment(const Segment & s, int loop_id, bool isOut, const CP_Point & bottom) {
  SweepEvent *event_s = storeSweepEvent(SweepEvent(s.source, nullptr, loop_id, isOut));
  SweepEvent *event_t = storeSweepEvent(SweepEvent(s.target, event_s, loop_id, isOut));
  event_s->other_event = event_t;
  event_s->setLeftFlag();
  if (bottom == s.source)
    event_s->is_bottom = true;
  if (bottom == s.target)
    event_t->is_bottom = true;
  event_queue.push(event_s);
  event_queue.push(event_t);
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
  if (Great(d_uva*d_uvb, 0.0) || Great(d_abu*d_abv, 0.0)) // 没有交点
    return 0;
  if (Equal(d_abu, 0.0) && Equal(d_abv, 0.0) && Equal(d_uva, 0.0) && Equal(d_uvb, 0.0)) // 共线
    return 2;
  return 1; // 其他相交情况
}

void CP_SweepLine::initializeQueue(const CP_Polygon & polygon) {
  // 完成初始化
  int loop_first_id = 0;
  int loop_size = 0;
  int point_id = 0;
  // 环id
  int loop_id = 0;
  // 遍历多边形的所有区域
  for (auto region : polygon.m_regionArray) {
    // 遍历区域中的所有环
    for (int l = 0; l < region.m_loopArray.size(); ++l) {
      auto loop = region.m_loopArray[l];
      // 判断是否是外环
      bool isOut = loop.m_loopIDinRegion == 0;
      // 找到环的最低点
      double min_y = 1e18, min_x = 0;
      for (auto id : loop.m_pointIDArray) {
        if (polygon.m_pointArray[id].m_y < min_y) {
          min_y = polygon.m_pointArray[id].m_y;
          min_x = polygon.m_pointArray[id].m_x;
        }
      }
      CP_Point bottom_point(min_x, min_y);
      // 初始化事件队列
      loop_first_id = loop.m_pointIDArray[0];
      loop_size = loop.m_pointIDArray.size();
      int d = loop_first_id == point_id ? 1 : -1;
      for (int i = 0; i < loop_size; ++i) {
        auto source = polygon.m_pointArray[loop_first_id + d * (i % loop_size)];
        auto target = polygon.m_pointArray[loop_first_id + d * ((i + 1) % loop_size)];
        // 将该线段对应的 SweepEvent 加入到EventQueue 中。
        Segment s(source, target);
        processSegment(s, loop_id, isOut, bottom_point);
        ++point_id;
      }
      ++loop_id;
    }
  }
}

bool CP_SweepLine::check(const CP_Polygon & polygon) {
  // step 1: 判断是否存在边相交或者重叠
  initializeQueue(polygon, PolygonType::kPolygonCheck);
  StatusSet status_set;
  StatusSet::iterator position, previous, next;
  SweepEvent* sweep_event;
  int self_intersect = 0;
  while (!event_queue.empty()) {
    sweep_event = event_queue.top();
    event_queue.pop();
    if (sweep_event->left) { // left point
      // 将sweep_event 插入队列
      sweep_event->poss = position = status_set.insert(sweep_event).first;
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
  sweep_event = nullptr;
  status_set.clear();
  position = previous = next = status_set.end();
  // end step 1 复杂度O((N+k)logN)

  // step 2: 判断环的位置合法性
  // 找到环最上点，发出一条向上的射线与多边形相交
  // 外环的交点必须为偶数，内环的交点必须为奇数
  // 采用扫描线算法实现
  initializeQueue(polygon);
  while (!event_queue.empty()) {
    sweep_event = event_queue.top();
    event_queue.pop();
    if (sweep_event->left) // 将左顶点事件插入set
      sweep_event->poss = position = status_set.insert(sweep_event).first;
    else // 取出右顶点对应的事件
      position = sweep_event->other_event->poss;
    if (sweep_event->is_bottom) {  // 如果是最低点
      // 计算最低点射线与边的交点数
      int intersect = 0;
      Segment ray(sweep_event->point, CP_Point(sweep_event->point.m_x, -1e18));
      for (++position; position != status_set.end(); ++position) {
        Segment tmp((*position)->point, (*position)->other_event->point);
        if (tmp.source != ray.source && tmp.target != ray.source)
          intersect += segmentIntersect(tmp, ray);
      }
      if (sweep_event->is_external && intersect % 2 == 1)
        return false;
      if (!sweep_event->is_external && intersect % 2 == 0)
        return false;
    }
    if (!sweep_event->left)
      status_set.erase(sweep_event->other_event->poss);
  }
  // end step 2 复杂度O(NlogN)
  return true;
}

void CP_SweepLine::processLoop(CP_Polygon & result) {
  initializeQueue(result);
  StatusSet status_set;
  StatusSet::iterator position, previous, next;
  SweepEvent* sweep_event;
  std::vector<bool> is_processed(result.m_regionArray.size(), false);
  while (!event_queue.empty()) {
    sweep_event = event_queue.top();
    event_queue.pop();
    if (sweep_event->left) // 将左顶点事件插入set
      sweep_event->poss = position = status_set.insert(sweep_event).first;
    else // 取出右顶点对应的事件
      position = sweep_event->other_event->poss;
    int loopid = sweep_event->loop_id;
    if (sweep_event->is_bottom && !is_processed[loopid]) {
      // 计算最低点射线与边的交点数
      int intersect = 0;
      Segment ray(sweep_event->point, CP_Point(sweep_event->point.m_x, -1e18));
      for (++position; position != status_set.end(); ++position) {
        Segment tmp((*position)->point, (*position)->other_event->point);
        if (tmp.source != ray.source && tmp.target != ray.source)
          intersect += segmentIntersect(tmp, ray);
      }
      if (intersect % 2 == 1) { 
        CP_Loop & loop = result.m_regionArray[loopid].m_loopArray[0];
        loop.m_loopIDinRegion = 1;
      }
      is_processed[loopid] = true;
    }
    if (!sweep_event->left)
      status_set.erase(sweep_event->other_event->poss);
  }
  clear();
}
