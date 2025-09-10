import sys
sys.path.append('/home/plusai/workspace/common_protobuf/build/latest')

import rospy
import csv
from std_msgs.msg import String
# 导入你的模块
from plusai_common_proto.control.dbw_reports_pb2 import DbwReports
# /home/plusai/workspace/common_protobuf/build/latest/plusai_common_proto/control
import struct


def callback(msg):
    try:
        dbw_report = DbwReports()
        dbw_report.ParseFromString(msg.data)
        # 提取 steering_wheel_torque
        steering_wheel_angle = dbw_report.steering_report.steering_wheel_angle
        steering_wheel_torque = dbw_report.steering_report.steering_wheel_torque
        wheel_speed = dbw_report.wheel_speed_report.front_axle_speed
        yaw_rate = dbw_report.vehicle_dynamic.angular_velocity.z
        # 如果数据不是0.0，将数据添加到列表并写入 CSV
        if steering_wheel_torque != 0.0:
            write_to_csv([steering_wheel_angle,steering_wheel_torque,wheel_speed,yaw_rate])
    except Exception as e:
        rospy.logwarn(f"Failed to decode binary data: {e}")


def write_to_csv(data):
    with open('/home/plusai/workspace/HMS/csvLog/test_data.csv', 'a') as f:
        writer = csv.writer(f)
        writer.writerow(data)

def listener():
    rospy.init_node('dbw_reports_listener', anonymous=True)
    # 订阅话题
    rospy.Subscriber('/vehicle/dbw_reports', String, callback)
    rospy.spin()

if __name__ == '__main__':
    open('/home/plusai/workspace/HMS/csvLog/test_data.csv', 'w', newline='')
    listener()
