#pragma once
#include "CP_Polygon.h"
#include <queue>
#include <set>
#include <list>
// 定义容差
const double TOLERANCE = 1e-6;
// 浮点数比较运算
inline bool Equal(double a, double b, double tolerance = TOLERANCE) noexcept { return b - a <= tolerance && a - b <= tolerance; }

inline bool NotEqual(double a, double b, double tolerance = TOLERANCE) noexcept { return b - a > tolerance || a - b > tolerance; }

inline bool Great(double a, double b, double tolerance = TOLERANCE) noexcept { return a > b + tolerance; }

inline bool GreatEqual(double a, double b, double tolerance = TOLERANCE) noexcept { return a > b - tolerance; }

inline bool Less(double a, double b, double tolerance = TOLERANCE) noexcept { return a + tolerance < b; }

inline bool LessEqual(double a, double b, double tolerance = TOLERANCE) noexcept { return a < b + tolerance; }

inline bool operator==(const CP_Point& lhs, const CP_Point& rhs) { return Equal(lhs.m_x, rhs.m_x) && Equal(lhs.m_y, rhs.m_y); }
inline bool operator!=(const CP_Point& lhs, const CP_Point& rhs) { return !(lhs == rhs); }

// 定义向量类型CP_Vector
typedef CP_Point CP_Vector;
class SweepEvent;
class Segment {
public:
  /** Default constructor */
  Segment() {}
  Segment(const CP_Point& s, const CP_Point& t) : source(s), target(t) {}
  ~Segment() {}
public:
  CP_Point source, target;
};

enum OperationType {
  kUnion, 
  kIntersection,
  kA_B,
  kB_A,
  kXOR
};
enum PolygonType {
  kPolygonA,
  kPolygonB,
  kPolygonCheck
};
enum EdgeType { // 边的类型（是否重叠）
  kNormal, 
  kNon_contributing, 
  kSame_trasition, 
  kDifferent_trasition
};

class queue_comparator : public binary_function<SweepEvent*, SweepEvent*, bool> {
public:
  bool operator() (const SweepEvent* a, const SweepEvent* b) const;
};

class status_comparator : public binary_function<SweepEvent*, SweepEvent*, bool> {
public:
  bool operator() (const SweepEvent* a, const SweepEvent* b) const;
};

typedef std::priority_queue<SweepEvent*, std::vector<SweepEvent*>, queue_comparator> EventQueue;
typedef std::set<SweepEvent*, status_comparator> StatusSet;

class SweepEvent {
public: 
  SweepEvent() = default;
  // 布尔运算构造函数
  SweepEvent(const CP_Point& point, SweepEvent* other, PolygonType ptype, EdgeType etype = kNormal) 
    : point(point), other_event(other), polygon_type(ptype), edge_type(etype) {}
  // 内外环处理构造函数
  SweepEvent(const CP_Point& point, SweepEvent* other, int loopid, bool isOut, bool isBottom = false)
    : point(point), other_event(other), loop_id(loopid), is_external(isOut), is_bottom(isBottom) {}
  void setLeftFlag();
  Segment segment() { return Segment(point, other_event->point); }
  inline bool below(const CP_Point& p) const;
  inline bool above(const CP_Point& p) const;

public:
  // fields for boolean operation
  CP_Point point;  // point assoiated with this event 
  bool left;   // is the left endpoint of the edge
  SweepEvent* other_event;  // other event of the edge
  PolygonType polygon_type; // polygon type
  StatusSet::iterator poss; // the position in StatusSet
  EdgeType edge_type;
  //fields of informations
  bool inOut;
  bool inside;

  // fields for in/out loop check
  int loop_id;  // loop id for generation
  bool is_bottom;  // is this point top point of this loop
  bool is_external;  // is this loop an external loop
};

class CP_SweepLine {
public:
  // 布尔运算、求交方法
  CP_SweepLine() : event_queue(), event_holder() {}
  /*
  初始化SweepLine
  @polygon: 输入多边形
  @type: 多边形类型
  */
  void initializeQueue(const CP_Polygon& polygon, PolygonType type);
  /*
  执行布尔运算
  @result: 布尔运算结果
  @type: 布尔运算类型
  */
  void booleanOperation(CP_Polygon& result, OperationType type);
  // 清空
  void clear();
  // 合法性检验、多边形生成方法
  // 初始化队列
  void initializeQueue(const CP_Polygon& polygon);
  // 判断多边形是否合法
  bool check(const CP_Polygon& polygon);
  // 判断结果多边形内外环
  void processLoop(CP_Polygon& result);

private:
  EventQueue event_queue;
  deque<SweepEvent> event_holder;
  // 布尔运算所需的方法
  // 初始化处理线段
  void processSegment(const Segment& s, PolygonType pl);
  // 可能存在的交点
  int possibleIntersection(SweepEvent *e1, SweepEvent *e2);
  // 分割边
  void divideSegment(SweepEvent *e, const CP_Point& p);
  // 将事件插入队列中
  SweepEvent *storeSweepEvent(const SweepEvent& e) { event_holder.push_back(e); return &event_holder.back(); }
  // 合法性检验和生成结果多边形所需的方法
  // 合法性检验线段处理方法
  void processSegment(const Segment& s, int loop_id, bool isOut, const CP_Point& top);
  // 线段相交
  int segmentIntersect(const Segment& ab, const Segment& uv);
};