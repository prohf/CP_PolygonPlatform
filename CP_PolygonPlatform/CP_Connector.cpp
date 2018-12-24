#include "stdafx.h"
#include "CP_Connector.h"

void PointChain::init(const Segment & s) {
  point_list.push_back(s.source);
  point_list.push_back(s.target);
  is_closed = false;
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

