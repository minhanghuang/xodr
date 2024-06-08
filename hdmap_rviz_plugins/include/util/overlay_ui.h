#ifndef RVIZ_PLUGIN_OVERLAY_UI_H_
#define RVIZ_PLUGIN_OVERLAY_UI_H_

#include <string>
#include <unordered_map>
#include <vector>

namespace hdmap_rviz_plugins {

/**
 * @class OverlayUI
 * @brief
 *    key: double
 *    key: std::string
 *    key: std::vector<double>
 *    key: std::vector<std::string>
 *
 */
class OverlayUI {
 public:
  OverlayUI() = default;
  virtual ~OverlayUI() = default;
  virtual std::vector<std::string> Format() = 0;
};

class CurrentRegionOverlayUI : public OverlayUI {
 public:
  CurrentRegionOverlayUI() { point_.reserve(3); }
  virtual std::vector<std::string> Format() override {
    std::vector<std::string> ret;
    // id
    std::string id_text;
    id_text.append("id: ");
    id_text.append(id_);
    ret.emplace_back(id_text);

    // point
    std::string point_text;
    point_text.append("point: [");
    for (int i = 0; i < 2; i++) {
      point_text.append(std::to_string(point_.at(i)));
      point_text.append("  ");
    }
    point_text.append("]");
    ret.emplace_back(point_text);

    // heading
    std::string heading_text;
    heading_text.append("heading: ");
    heading_text.append(std::to_string(point_.at(2)));
    ret.emplace_back(heading_text);
    return ret;
  }

  std::string id() const { return id_; }

  std::string* mutable_id() { return &id_; }

  std::vector<double> point() const { return point_; }

  std::vector<double>* mutable_point() { return &point_; }

 private:
  std::string id_;  // lane id

  std::vector<double> point_;  // [x, y, heading]
};

}  // namespace hdmap_rviz_plugins

#endif  // RVIZ_PLUGIN_OVERLAY_UI_H_
