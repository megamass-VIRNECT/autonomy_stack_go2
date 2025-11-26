#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

cd $SCRIPT_DIR
source ../../install/setup.bash
source ../../src/unitree_ros2/unitree_ros2_setup.sh
ros2 launch vehicle_simulator system_real_robot_autonomous.launch
