import rclpy
from rclpy.node import Node
from interactive_markers.interactive_marker_server import InteractiveMarkerServer
from visualization_msgs.msg import InteractiveMarker, Marker, InteractiveMarkerControl
from std_msgs.msg import Bool

class ControlModeSwitcher(Node):
    def __init__(self):
        super().__init__('control_mode_switcher')
        self.server = InteractiveMarkerServer(self, 'control_mode_marker')
        self.pub_mode = self.create_publisher(Bool, '/control_mode', 10)
        self.control_enabled = True
        
        # Timer to republish state periodically (to ensure late joiners get it)
        self.timer = self.create_timer(1.0, self.timer_callback)
        
        self.create_marker()
        self.get_logger().info("Control Mode Switcher Initialized")

    def timer_callback(self):
        msg = Bool()
        msg.data = self.control_enabled
        self.pub_mode.publish(msg)

    def create_marker(self):
        int_marker = InteractiveMarker()
        int_marker.header.frame_id = "vehicle"
        int_marker.name = "mode_switch_button"
        int_marker.description = ""
        int_marker.scale = 0.5
        int_marker.pose.position.z = 1.0  # Above the robot

        # Button Box
        box_marker = Marker()
        box_marker.type = Marker.CUBE
        box_marker.scale.x = 0.5
        box_marker.scale.y = 1.2
        box_marker.scale.z = 0.3
        
        if self.control_enabled:
            box_marker.color.r = 0.0
            box_marker.color.g = 0.8
            box_marker.color.b = 0.0
        else:
            box_marker.color.r = 0.8
            box_marker.color.g = 0.0
            box_marker.color.b = 0.0
            
        box_marker.color.a = 0.9

        # Text
        text_marker = Marker()
        text_marker.type = Marker.TEXT_VIEW_FACING
        text_marker.scale.z = 0.25
        text_marker.text = "AUTO MODE" if self.control_enabled else "MANUAL MODE"
        text_marker.color.r = 1.0
        text_marker.color.g = 1.0
        text_marker.color.b = 1.0
        text_marker.color.a = 1.0
        text_marker.pose.position.z = 0.0

        # Control
        control = InteractiveMarkerControl()
        control.interaction_mode = InteractiveMarkerControl.BUTTON
        control.always_visible = True
        control.markers.append(box_marker)
        control.markers.append(text_marker)
        int_marker.controls.append(control)

        self.server.insert(int_marker, feedback_callback=self.process_feedback)
        self.server.applyChanges()

    def process_feedback(self, feedback):
        if feedback.event_type == InteractiveMarkerControl.MOUSE_DOWN:
            self.control_enabled = not self.control_enabled
            self.get_logger().info(f"Switched to {'AUTO' if self.control_enabled else 'MANUAL'}")
            
            # Publish topic immediately
            msg = Bool()
            msg.data = self.control_enabled
            self.pub_mode.publish(msg)
            
            # Update marker
            self.create_marker()

def main(args=None):
    rclpy.init(args=args)
    node = ControlModeSwitcher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
