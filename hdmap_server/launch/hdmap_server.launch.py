import os

import launch
import launch_ros
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    config = os.path.join(
        get_package_share_directory("hdmap_server"), "config", "hdmap_server.yaml"
    )

    return launch.LaunchDescription(
        [
            # hdmap server
            launch_ros.actions.Node(
                package="hdmap_server",
                executable="hdmap_server",
                parameters=[config],
                output="screen",
            ),
            # Rviz
            launch_ros.actions.Node(
                package="rviz2",
                executable="rviz2",
                name="rviz2",
                parameters=[{"rviz2": {"args": "-d hdmap_server.rviz"}}],
                output="screen",
            ),
        ],
    )
