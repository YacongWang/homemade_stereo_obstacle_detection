#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <image_transport/subscriber_filter.h>
#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>
#include <message_filters/sync_policies/exact_time.h>
#include <stereo_msgs/DisparityImage.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/Image.h>
#include <boost/thread.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/highgui/highgui.hpp>
#include <sensor_msgs/Imu.h>
#include <tf/LinearMath/Matrix3x3.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/point_types.h>
#include <pcl/conversions.h>
#include <pcl_ros/transforms.h>
#include <pcl/search/search.h>
#include <pcl/search/kdtree.h>
#include <pcl/filters/passthrough.h>
#include <pcl/segmentation/region_growing_rgb.h>
#include <pcl/visualization/cloud_viewer.h>

using namespace sensor_msgs;
using namespace stereo_msgs;
using namespace message_filters::sync_policies;
static unsigned char colormap[768] =
  { 150, 150, 150,
    107, 0, 12,
    106, 0, 18,
    105, 0, 24,
    103, 0, 30,
    102, 0, 36,
    101, 0, 42,
    99, 0, 48,
    98, 0, 54,
    97, 0, 60,
    96, 0, 66,
    94, 0, 72,
    93, 0, 78,
    92, 0, 84,
    91, 0, 90,
    89, 0, 96,
    88, 0, 102,
    87, 0, 108,
    85, 0, 114,
    84, 0, 120,
    83, 0, 126,
    82, 0, 131,
    80, 0, 137,
    79, 0, 143,
    78, 0, 149,
    77, 0, 155,
    75, 0, 161,
    74, 0, 167,
    73, 0, 173,
    71, 0, 179,
    70, 0, 185,
    69, 0, 191,
    68, 0, 197,
    66, 0, 203,
    65, 0, 209,
    64, 0, 215,
    62, 0, 221,
    61, 0, 227,
    60, 0, 233,
    59, 0, 239,
    57, 0, 245,
    56, 0, 251,
    55, 0, 255,
    54, 0, 255,
    52, 0, 255,
    51, 0, 255,
    50, 0, 255,
    48, 0, 255,
    47, 0, 255,
    46, 0, 255,
    45, 0, 255,
    43, 0, 255,
    42, 0, 255,
    41, 0, 255,
    40, 0, 255,
    38, 0, 255,
    37, 0, 255,
    36, 0, 255,
    34, 0, 255,
    33, 0, 255,
    32, 0, 255,
    31, 0, 255,
    29, 0, 255,
    28, 0, 255,
    27, 0, 255,
    26, 0, 255,
    24, 0, 255,
    23, 0, 255,
    22, 0, 255,
    20, 0, 255,
    19, 0, 255,
    18, 0, 255,
    17, 0, 255,
    15, 0, 255,
    14, 0, 255,
    13, 0, 255,
    11, 0, 255,
    10, 0, 255,
    9, 0, 255,
    8, 0, 255,
    6, 0, 255,
    5, 0, 255,
    4, 0, 255,
    3, 0, 255,
    1, 0, 255,
    0, 4, 255,
    0, 10, 255,
    0, 16, 255,
    0, 22, 255,
    0, 28, 255,
    0, 34, 255,
    0, 40, 255,
    0, 46, 255,
    0, 52, 255,
    0, 58, 255,
    0, 64, 255,
    0, 70, 255,
    0, 76, 255,
    0, 82, 255,
    0, 88, 255,
    0, 94, 255,
    0, 100, 255,
    0, 106, 255,
    0, 112, 255,
    0, 118, 255,
    0, 124, 255,
    0, 129, 255,
    0, 135, 255,
    0, 141, 255,
    0, 147, 255,
    0, 153, 255,
    0, 159, 255,
    0, 165, 255,
    0, 171, 255,
    0, 177, 255,
    0, 183, 255,
    0, 189, 255,
    0, 195, 255,
    0, 201, 255,
    0, 207, 255,
    0, 213, 255,
    0, 219, 255,
    0, 225, 255,
    0, 231, 255,
    0, 237, 255,
    0, 243, 255,
    0, 249, 255,
    0, 255, 255,
    0, 255, 249,
    0, 255, 243,
    0, 255, 237,
    0, 255, 231,
    0, 255, 225,
    0, 255, 219,
    0, 255, 213,
    0, 255, 207,
    0, 255, 201,
    0, 255, 195,
    0, 255, 189,
    0, 255, 183,
    0, 255, 177,
    0, 255, 171,
    0, 255, 165,
    0, 255, 159,
    0, 255, 153,
    0, 255, 147,
    0, 255, 141,
    0, 255, 135,
    0, 255, 129,
    0, 255, 124,
    0, 255, 118,
    0, 255, 112,
    0, 255, 106,
    0, 255, 100,
    0, 255, 94,
    0, 255, 88,
    0, 255, 82,
    0, 255, 76,
    0, 255, 70,
    0, 255, 64,
    0, 255, 58,
    0, 255, 52,
    0, 255, 46,
    0, 255, 40,
    0, 255, 34,
    0, 255, 28,
    0, 255, 22,
    0, 255, 16,
    0, 255, 10,
    0, 255, 4,
    2, 255, 0,
    8, 255, 0,
    14, 255, 0,
    20, 255, 0,
    26, 255, 0,
    32, 255, 0,
    38, 255, 0,
    44, 255, 0,
    50, 255, 0,
    56, 255, 0,
    62, 255, 0,
    68, 255, 0,
    74, 255, 0,
    80, 255, 0,
    86, 255, 0,
    92, 255, 0,
    98, 255, 0,
    104, 255, 0,
    110, 255, 0,
    116, 255, 0,
    122, 255, 0,
    128, 255, 0,
    133, 255, 0,
    139, 255, 0,
    145, 255, 0,
    151, 255, 0,
    157, 255, 0,
    163, 255, 0,
    169, 255, 0,
    175, 255, 0,
    181, 255, 0,
    187, 255, 0,
    193, 255, 0,
    199, 255, 0,
    205, 255, 0,
    211, 255, 0,
    217, 255, 0,
    223, 255, 0,
    229, 255, 0,
    235, 255, 0,
    241, 255, 0,
    247, 255, 0,
    253, 255, 0,
    255, 251, 0,
    255, 245, 0,
    255, 239, 0,
    255, 233, 0,
    255, 227, 0,
    255, 221, 0,
    255, 215, 0,
    255, 209, 0,
    255, 203, 0,
    255, 197, 0,
    255, 191, 0,
    255, 185, 0,
    255, 179, 0,
    255, 173, 0,
    255, 167, 0,
    255, 161, 0,
    255, 155, 0,
    255, 149, 0,
    255, 143, 0,
    255, 137, 0,
    255, 131, 0,
    255, 126, 0,
    255, 120, 0,
    255, 114, 0,
    255, 108, 0,
    255, 102, 0,
    255, 96, 0,
    255, 90, 0,
    255, 84, 0,
    255, 78, 0,
    255, 72, 0,
    255, 66, 0,
    255, 60, 0,
    255, 54, 0,
    255, 48, 0,
    255, 42, 0,
    255, 36, 0,
    255, 30, 0,
    255, 24, 0,
    255, 18, 0,
    255, 12, 0,
    255,  6, 0,
    255,  0, 0,
  };


class obstacle_detecition
{
private:
    image_transport::SubscriberFilter left_sub_,right_sub_;
    message_filters::Subscriber<DisparityImage> disparity_sub_;
    message_filters::Subscriber<PointCloud2> pointcloud2_sub_;
    message_filters::Subscriber<Imu> imu_sub_;

    typedef ExactTime<Image,Image,DisparityImage,PointCloud2,Imu> ExactPolicy;
    typedef ApproximateTime<Image,Image,DisparityImage,PointCloud2,Imu> ApproximatePolicy;

    typedef message_filters::Synchronizer<ExactPolicy> ExactSync;
    typedef message_filters::Synchronizer<ApproximatePolicy> ApproximateSync;
    boost::shared_ptr<ExactSync> exact_sync_;
    boost::shared_ptr<ApproximateSync> approximate_sync_;

    ImageConstPtr last_left_msg_,last_right_msg_;
    cv::Mat last_left_image_,last_right_image_;
    cv::Mat_<cv::Vec3b> disparity_color_;
    cv::Mat_<cv::Vec3f> points2_mat_;
    double roll,pitch,yaw;
    boost::mutex image_mutex_;
public:
    obstacle_detecition(const std::string& transport)
    {
      //   ROS_INFO("~~~~~~~~~~~~~~~~~~~stereo_obstacle_detection getMsgs construct~~~~~~~~~~~~~~~");
        ros::NodeHandle nh;
        ros::NodeHandle local_nh("~");// use this nh to load parameters
        bool autosize;
        local_nh.param("autosize",autosize,false);
        int flags = autosize ? cv::WND_PROP_AUTOSIZE:0;
        cv::namedWindow("left_obs",flags);
        cv::namedWindow("right_obs",flags);
        cv::namedWindow("disparity_obs",flags);

        image_transport::ImageTransport it(nh);
        left_sub_.subscribe(it,ros::names::clean("/stereo/left/image_rect_color"),1,transport);
        right_sub_.subscribe(it,ros::names::clean("/stereo/right/image_rect_color"),1,transport);
        disparity_sub_.subscribe(nh,ros::names::clean("/stereo/disparity"),1);
        pointcloud2_sub_.subscribe(nh,ros::names::clean("/stereo/points2"),1);
        imu_sub_.subscribe(nh,ros::names::clean("/inertialtf/Imu"),1);
        bool approx;
      //  ROS_INFO("stereo_obstacle_detection getMsgs");
        local_nh.param("approximate_sync",approx,true);
        if(approx)
        {
            approximate_sync_.reset(new ApproximateSync(ApproximatePolicy(5),left_sub_,right_sub_,disparity_sub_,pointcloud2_sub_,imu_sub_));
            approximate_sync_->registerCallback(boost::bind(&obstacle_detecition::imageCb,this,_1,_2,_3,_4,_5));

        }
        else
        {
            exact_sync_.reset(new ExactSync(ExactPolicy(5),left_sub_,right_sub_,disparity_sub_,pointcloud2_sub_,imu_sub_));
            exact_sync_->registerCallback(boost::bind(&obstacle_detecition::imageCb,this,_1,_2,_3,_4,_5));

        }
    }
    ~obstacle_detecition()
       {
            cv::destroyAllWindows();
        }
     void imageCb(const ImageConstPtr& left,const ImageConstPtr& right,const DisparityImageConstPtr& disparity_msg,const PointCloud2ConstPtr pointcloud2_msg,const ImuConstPtr imu_msg)
     {
         tf::Quaternion q(imu_msg->orientation.x,imu_msg->orientation.y,imu_msg->orientation.z,imu_msg->orientation.w);
         tf::Matrix3x3 m(q);
         m.getRPY(roll,pitch,yaw);

         image_mutex_.lock();
         last_left_image_ = cv_bridge::toCvShare(left,"bgr8")->image;
         last_right_image_ = cv_bridge::toCvShare(right,"bgr8")->image;
         float min_disparity = disparity_msg->min_disparity;
         float max_disparity = disparity_msg->max_disparity;
         float multiplier = 255.0f/(max_disparity-min_disparity);
         const cv::Mat_<float> dmat(disparity_msg->image.height,disparity_msg->image.width,
                                    (float*)&disparity_msg->image.data[0],disparity_msg->image.step);
         disparity_color_.create(disparity_msg->image.height,disparity_msg->image.width);
         for(int row = 0;row<disparity_color_.rows;++row)
         {
             const float* d=dmat[row];
             for(int col = 0;col < disparity_color_.cols;++col)
             {
                 int index = (d[col]-min_disparity)*multiplier+0.5;
                 index = std::min(255,std::max(0,index));
                 disparity_color_(row,col)[2] = colormap[3*index+0];
                 disparity_color_(row,col)[1] = colormap[3*index+1];
                 disparity_color_(row,col)[0] = colormap[3*index+2];
             }

         }

         image_mutex_.unlock();
         if(!last_left_image_.empty())
             cv::imshow("left_obs",last_left_image_);
         if(!last_right_image_.empty())
             cv::imshow("right_obs",last_right_image_);
         cv::imshow("disparity_obs",disparity_color_);
         cv::waitKey(10);

    }
//subscribe IMU information to filter the ground

};
int main(int argc,char **argv)
{
    //ROS_INFO("~~~~~~~~~~~~~~~~~~~stereo_obstacle_detection ~~~~~~~~~~~~~~~");
    ros::init(argc,argv,"stereo_obstacle_detection_imu");
    std::string transport = "raw";
    obstacle_detecition obsDect(transport);
    ros::spin();
    return 0;

}

