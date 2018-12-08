#include "stdafx.h"
#include "CP_SweepLine.h"
#include <iterator>

void SweepEvent::setInformation(const SweepEvent & other) {

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

void possibleIntersection(SweepEvent & a, SweepEvent & b) {

}

