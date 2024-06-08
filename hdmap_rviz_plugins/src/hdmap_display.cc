#include "hdmap_rviz_plugins/hdmap_display.h"

namespace hdmap_rviz_plugins {

MapDisplay::MapDisplay()
    : global_map_topic_("/hdmap_server/global_map"),
      current_region_topic_("/hdmap_server/current_region") {}

MapDisplay::~MapDisplay() {}

void MapDisplay::onInitialize() {
  rviz_common::Display::onInitialize();
  rviz_rendering::RenderSystem::get()->prepareOverlays(scene_manager_);
  node_ = context_->getRosNodeAbstraction().lock()->get_raw_node();
  SetupRosSubscriptions();
  SetupRosService();
  SetupRosTimer();
  SetupOverlay();
  ShowGlobalMap();
}

void MapDisplay::SetupRosSubscriptions() {
  current_region_sub_ = node_->create_subscription<hdmap_msgs::msg::Region>(
      current_region_topic_, 1,
      std::bind(&MapDisplay::CurrentRegionCallback, this,
                std::placeholders::_1));
}

void MapDisplay::SetupRosService() {
  global_map_client_ =
      node_->create_client<hdmap_msgs::srv::GetGlobalMap>(global_map_topic_);
  while (!global_map_client_->wait_for_service(std::chrono::seconds(1))) {
    if (!rclcpp::ok()) {
      return;
    }
  }
}

void MapDisplay::SetupRosTimer() {
  timers_.emplace_back(node_->create_wall_timer(
      // 10Hz
      rclcpp::Rate(10).period(),
      std::bind(&MapDisplay::ShowCurrentRegion, this)));
}

void MapDisplay::SetupOverlay() {
  overlay_ = std::make_shared<OverlayComponent>();
  overlap_ui_ = std::make_shared<CurrentRegionOverlayUI>();
}

void MapDisplay::ShowGlobalMap() {
  std::lock_guard<std::mutex> guard(mutex_);
  auto request = std::make_shared<hdmap_msgs::srv::GetGlobalMap::Request>();
  auto response_callback =
      [this](
          rclcpp::Client<hdmap_msgs::srv::GetGlobalMap>::SharedFuture future) {
        auto response = future.get();
        GlobalMapMsgToBillboardLines(response->map, rviz_lines_);
      };
  auto future =
      global_map_client_->async_send_request(request, response_callback);
}

void MapDisplay::ShowCurrentRegion() {
  hdmap_msgs::msg::Region current_region;
  {
    std::lock_guard<std::mutex> guard(mutex_);
    if (current_region_msg_.id.empty()) {
      return;
    }
    current_region = current_region_msg_;
  }
  *overlap_ui_->mutable_id() = current_region_msg_.id;
  overlap_ui_->mutable_point()->clear();
  overlap_ui_->mutable_point()->emplace_back(current_region_msg_.point.x);
  overlap_ui_->mutable_point()->emplace_back(current_region_msg_.point.y);
  overlap_ui_->mutable_point()->emplace_back(current_region_msg_.heading);
  overlay_->Clean();
  overlay_->Update(overlap_ui_.get());
  overlay_->Show();
}

void MapDisplay::CurrentRegionCallback(
    const hdmap_msgs::msg::Region::SharedPtr msg) {
  std::lock_guard<std::mutex> guard(mutex_);
  current_region_msg_ = *msg;
}

void MapDisplay::GlobalMapMsgToBillboardLines(
    const hdmap_msgs::msg::Map& map,
    std::vector<std::shared_ptr<rviz_rendering::BillboardLine>>& lines) {
  if (map.roads.empty()) {
    return;
  }
  lines.clear();
  for (const auto& road : map.roads) {
    for (const auto& section : road.sections) {
      for (const auto& lane : section.lanes) {
        const int line_size = std::max<int>(
            0, std::min<int>(lane.central_curve.pts.size(),
                             std::min<int>(lane.left_boundary.pts.size(),
                                           lane.left_boundary.pts.size())));
        {
          /// fill central line
          auto rviz_line = std::make_shared<rviz_rendering::BillboardLine>(
              scene_manager_, scene_node_);
          rviz_line->setMaxPointsPerLine(line_size);
          rviz_line->setNumLines(1);
          for (int i = 0; i < line_size; i++) {
            rviz_line->addPoint(
                Ogre::Vector3(lane.central_curve.pts.at(i).point.x,
                              lane.central_curve.pts.at(i).point.y,
                              lane.central_curve.pts.at(i).point.z));
          }
          rviz_lines_.emplace_back(rviz_line);
        }
        {
          /// fill left boundary line
          auto rviz_line = std::make_shared<rviz_rendering::BillboardLine>(
              scene_manager_, scene_node_);
          rviz_line->setMaxPointsPerLine(line_size);
          rviz_line->setNumLines(1);
          for (int i = 0; i < line_size; i++) {
            rviz_line->addPoint(
                Ogre::Vector3(lane.left_boundary.pts.at(i).point.x,
                              lane.left_boundary.pts.at(i).point.y,
                              lane.left_boundary.pts.at(i).point.z));
          }
          rviz_lines_.emplace_back(rviz_line);
        }
        {
          /// fill right boundary line
          auto rviz_line = std::make_shared<rviz_rendering::BillboardLine>(
              scene_manager_, scene_node_);
          rviz_line->setMaxPointsPerLine(line_size);
          rviz_line->setNumLines(1);
          for (int i = 0; i < line_size; i++) {
            rviz_line->addPoint(
                Ogre::Vector3(lane.right_boundary.pts.at(i).point.x,
                              lane.right_boundary.pts.at(i).point.y,
                              lane.right_boundary.pts.at(i).point.z));
          }
          rviz_lines_.emplace_back(rviz_line);
        }
      }
    }
  }
}

}  // namespace hdmap_rviz_plugins

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(hdmap_rviz_plugins::MapDisplay, rviz_common::Display)
