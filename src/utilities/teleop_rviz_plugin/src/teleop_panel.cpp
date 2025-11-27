#include "drive_widget.h"
#include "teleop_panel.h"
// #include 
#include <rclcpp/time.hpp>

namespace teleop_rviz_plugin
{

TeleopPanel::TeleopPanel( QWidget* parent )
  : rviz_common::Panel( parent )
  , linear_velocity_( 0 )
  , angular_velocity_( 0 )
  , mouse_pressed_( false )
  , mouse_pressed_sent_( false )
  , control_enabled_( false )
{
  node_ = rclcpp::Node::make_shared("teleop_panel_node");

  QVBoxLayout* layout = new QVBoxLayout;
  
  // Status Label
  status_label_ = new QLabel("Current Mode: MANUAL", this);
  status_label_->setAlignment(Qt::AlignCenter);
  status_label_->setStyleSheet("font-weight: bold; font-size: 14px; color: black; background-color: #FFCCCC; border: 1px solid red; padding: 5px;");
  layout->addWidget(status_label_);
  
  // Control Mode Button
  control_mode_button_ = new QPushButton( "Change to AUTO Mode", this );
  control_mode_button_->setStyleSheet("background-color: green; color: white; font-weight: bold; padding: 5px;");
  layout->addWidget( control_mode_button_ );

  push_button_1_ = new QPushButton( "Resume Navigation to Goal", this );
  layout->addWidget( push_button_1_ );
  drive_widget_ = new DriveWidget;
  layout->addWidget( drive_widget_ );
  setLayout( layout );

  QTimer* output_timer = new QTimer( this );

  connect( push_button_1_, SIGNAL( pressed() ), this, SLOT( pressButton1() ));
  connect( control_mode_button_, SIGNAL( pressed() ), this, SLOT( toggleControlMode() ));
  connect( drive_widget_, SIGNAL( outputVelocity( float, float, bool )), this, SLOT( setVel( float, float, bool )));
  connect( output_timer, SIGNAL( timeout() ), this, SLOT( sendVel() ));

  output_timer->start( 100 );

  velocity_publisher_ = node_->create_publisher<sensor_msgs::msg::Joy>("/joy", 5);
  control_mode_publisher_ = node_->create_publisher<std_msgs::msg::Bool>("/control_mode", 5);

  drive_widget_->setEnabled( true );
}

void TeleopPanel::pressButton1()
{
  if (rclcpp::ok() && velocity_publisher_->get_subscription_count() > 0)
  {
    sensor_msgs::msg::Joy joy;

    joy.axes.push_back(0);
    joy.axes.push_back(0);
    joy.axes.push_back(-1.0);
    joy.axes.push_back(0);
    joy.axes.push_back(1.0);
    joy.axes.push_back(1.0);
    joy.axes.push_back(0);
    joy.axes.push_back(0);

    joy.buttons.push_back(0);
    joy.buttons.push_back(0);
    joy.buttons.push_back(0);
    joy.buttons.push_back(0);
    joy.buttons.push_back(0);
    joy.buttons.push_back(0);
    joy.buttons.push_back(0);
    joy.buttons.push_back(1);
    joy.buttons.push_back(0);
    joy.buttons.push_back(0);
    joy.buttons.push_back(0);

    joy.header.stamp = node_->now();
    joy.header.frame_id = "teleop_panel";
    velocity_publisher_->publish( joy );
  }
}

void TeleopPanel::toggleControlMode()
{
  control_enabled_ = !control_enabled_;
  
  std_msgs::msg::Bool msg;
  msg.data = control_enabled_;
  control_mode_publisher_->publish(msg);

  if (control_enabled_) {
    // Switched to AUTO
    status_label_->setText("Current Mode: AUTO");
    status_label_->setStyleSheet("font-weight: bold; font-size: 14px; color: black; background-color: #CCFFCC; border: 1px solid green; padding: 5px;");
    
    control_mode_button_->setText("Change to MANUAL Mode");
    control_mode_button_->setStyleSheet("background-color: red; color: white; font-weight: bold; padding: 5px;");
  } else {
    // Switched to MANUAL
    status_label_->setText("Current Mode: MANUAL");
    status_label_->setStyleSheet("font-weight: bold; font-size: 14px; color: black; background-color: #FFCCCC; border: 1px solid red; padding: 5px;");
    
    control_mode_button_->setText("Change to AUTO Mode");
    control_mode_button_->setStyleSheet("background-color: green; color: white; font-weight: bold; padding: 5px;");
  }
}

void TeleopPanel::setVel(float lin, float ang, bool pre)
{
  linear_velocity_ = lin;
  angular_velocity_ = ang;
  mouse_pressed_ = pre;
}

void TeleopPanel::sendVel()
{
  if (rclcpp::ok() && velocity_publisher_->get_subscription_count() > 0 && (mouse_pressed_ || mouse_pressed_sent_))
  {
    sensor_msgs::msg::Joy joy;

    joy.axes.push_back( 0 );
    joy.axes.push_back( 0 );
    joy.axes.push_back( 1.0 );
    joy.axes.push_back( angular_velocity_ );
    joy.axes.push_back( linear_velocity_ );
    joy.axes.push_back( 1.0 );
    joy.axes.push_back( 0 );
    joy.axes.push_back( 0 );

    joy.buttons.push_back( 0 );
    joy.buttons.push_back( 0 );
    joy.buttons.push_back( 0 );
    joy.buttons.push_back( 0 );
    joy.buttons.push_back( 0 );
    joy.buttons.push_back( 0 );
    joy.buttons.push_back( 0 );
    joy.buttons.push_back( 1 );
    joy.buttons.push_back( 0 );
    joy.buttons.push_back( 0 );
    joy.buttons.push_back( 0 );

    joy.header.stamp = node_->now();
    joy.header.frame_id = "teleop_panel";
    velocity_publisher_->publish(joy);

    mouse_pressed_sent_ = mouse_pressed_;
  }
}

} // end namespace teleop_rviz_plugin

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(teleop_rviz_plugin::TeleopPanel, rviz_common::Panel)
