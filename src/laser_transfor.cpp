#include "laser_geometry/laser_geometry.h"
#include "tf/tf.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/timer.hpp>

#include <pcl_ros/transforms.h>
#include<pcl_ros/point_cloud.h>
#include <fstream>


ros::Subscriber sub;
ros::Publisher pub;


boost::asio::io_service io;
boost::asio::serial_port sp(io,"/dev/ttyUSB0");
double w=0;
int last_sum;
double last_endtime;

//for debug
std::fstream fout;

double time_before,time_now;
double sum_before,sum_now;



void lCallback(const sensor_msgs::LaserScan::ConstPtr& scan_msg)
{

   // double gettime = ros::Time::now().toSec();

    //ROS_INFO("1");
    laser_geometry::LaserProjection p;

    sensor_msgs::PointCloud2 pointcloud_tmp;




    p.projectLaser(*scan_msg,pointcloud_tmp,-1.0,3);//1.0 - 2.0  or -1.0

    pointcloud_tmp.header.frame_id="/camera";

    Eigen::Matrix4f Tm= Eigen::Matrix4f::Identity();
    Eigen::Matrix4f Tm1 = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f Tm2 = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f Tm3 = Eigen::Matrix4f::Identity();

    ROS_INFO("2");
    // set the w to get angle
    bool isok(false);
    char bufread[10];
    int a(0),b(0),c(0),sum;
    sum = 0;
    int times(0);
    double endtime;
    while(isok==false)
    {
        memset(bufread,0,10);
        //bool err(false);
        if((sp.is_open()))
        {
            boost::asio::read(sp,boost::asio::buffer(bufread));
        }else{
            ROS_INFO("sp error");
        }
        endtime = ros::Time::now().toSec();
        time_now = endtime;
         for(int i =0 ;i< 10;i++)
         {
             bufread[i] =  bufread[i] & 0xff ;
             printf("%x--- ",bufread[i]);
        }
         //std::cout << std::endl;

        for(int i = 0;i < 10;i++)
        {

            if((bufread[i] & 0xff) == 0x55)
            {

                a = 0xff & bufread[i+1];
                b = 0xff & bufread[i+2];
                c = 0xff & bufread[i+3];

                //std::cout << bufread[i+1]<< "   " << bufread[i+2]<< std::endl;
                if( (b > -1) && (c == ((a+b) % 255)) )
                {

                    if(c != (a+b)) return;
                    sum = a * 256 + b;
                    std::cout << sum << std::endl;
                    if(sum<721 )
                    {
                        isok = true;
                        std::cout << "sumis:::::::::::::::::::::::::::"<<sum<<"(a,b)"<<a<<b<<std::endl;
                        sum_now = sum;
                        break;
                    }

                }
            }

        }
        //if(!isok) return;
        times++;
        if(times > 1) return;

    }
    std::cout << "times = " << times <<std::endl;

    ROS_INFO("3");
    endtime = endtime - pointcloud_tmp.header.stamp.toSec();

    //recover the sum


    int nsum;
    std::cout << "endtime is :;:::"<<endtime <<std::endl;
    if(sum >= last_sum)
    {
        nsum = sum - (endtime - 0.01) * std::abs((sum_before - sum_now)/(time_before - time_now));//700;//(sum - last_sum)/(endtime - last_endtime);//720;
        if(nsum < 0)
        {

            nsum = -1 * nsum ;
        }
    }else{
        nsum = sum + (endtime - 0.01) * std::abs((sum_before - sum_now)/(time_before - time_now));//700;//(su690m - last_sum)/(endtime - last_endtime);//720;
        if(nsum > 720)
        {

            nsum = 720 - (nsum -720) ;
        }
    }

    time_before = time_now;
    sum_before = sum_now;
    //fout << sum <<","<<nsum<<","<<pointcloud_tmp.header.stamp.toSec()<<","<<std::endl;
    sum = nsum;




    //w is the angle of lidar
    w =(sum /4 )-90 ;
    last_endtime = endtime;
    last_sum = sum;
    float theta=3.1415926 * w / 180;
    std::cout <<"theta:::::::"<<theta<<std::endl;

//    Tm1(0,0)=cos(theta);
//    Tm1(0,1)=-sin(theta);
//    Tm1(1,0)=sin(theta);
//    Tm1(1,1)=cos(theta);
//    Tm1(2,3)=2.5;
//    Tm1(1,3)=2.5;
//    Tm1(3,3)=1;


   //ROS_INFO("%f",theta);
    //ROS_INFO("w is:%f",w);

    Tm(0,0)=1;
    Tm(1,1)=cos(theta);
    Tm(1,2)=-sin(theta);
    Tm(2,1)=sin(theta);
    Tm(2,2)=cos(theta);


    Tm(2,3)=cos(theta) * 0.04;// * 50;//axis-z
    Tm(1,3)=-sin(theta) * 0.04 ;//* 50;//axis-y
    Tm(3,3)=1;

    //z -90
    Tm3(0,1)=1;
    Tm3(1,0)=-1;
    Tm3(2,2)=1;

    //x -90
    Tm2(0,0)=1;
    Tm2(1,2)=1;
    Tm2(2,1)=-1;
   // ROS_INFO("4");
    //z +180
    Tm1(0,0)=-1;
    Tm1(1,1)=-1;
    Tm1(2,2)=1;


   std::cout<< "pointcloud data:"<<pointcloud_tmp.data[20]<<"   size:"<<pointcloud_tmp.data.size()<<std::endl;

   std::cout <<"list"<<pcl::getFieldsList(pointcloud_tmp)<<std::endl;
   //get the really y z of the point
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   ///
   ///
   ///
   ///
   ///
   ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//   //out
  sensor_msgs::PointCloud2 out;
//   //define a transfomr (may same as Tm)
  Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
  transform = Tm;
//   transform(0,0)=1;
//   transform(1,1)=cos(theta);
//   transform(1,2)=-sin(theta);
//   transform(2,1)=sin(theta);
//   transform(2,2)=cos(theta);
//   transform(2,3)=cos(theta) * 0.035;
//   transform(1,3)=-sin(theta) * 0.035;
//   transform(3,3) = 1;
   // Get X-Y-Z indices;
   int x_idx = pcl::getFieldIndex(pointcloud_tmp,"x");
   int y_idx = pcl::getFieldIndex(pointcloud_tmp,"y");
   int z_idx = pcl::getFieldIndex(pointcloud_tmp,"z");


   //check if distance is available
   int dist_idx = pcl::getFieldIndex(pointcloud_tmp,"distance");
   //xyz_offset
   Eigen::Array4i xyz_offset (pointcloud_tmp.fields[x_idx].offset,pointcloud_tmp.fields[y_idx].offset,pointcloud_tmp.fields[z_idx].offset,0);

   for (size_t i = 0;i<pointcloud_tmp.width * pointcloud_tmp.height;++i)
   {
       transform(0,0)=1;
       transform(1,1)=cos(theta);
       transform(1,2)=-sin(theta);
       transform(2,1)=sin(theta);
       transform(2,2)=cos(theta);
       transform(2,3)=cos(theta) * 0.035;
       transform(1,3)=-sin(theta) * 0.035;
       transform(3,3) = 1;
       Eigen::Vector4f pt(*(float *)&pointcloud_tmp.data[xyz_offset[0]],*(float*)&pointcloud_tmp.data[xyz_offset[1]],*(float*)&pointcloud_tmp.data[xyz_offset[2]],1);
       Eigen::Vector4f pt_out;

       bool max_range_point = false;
       int distance_ptr_offset = i * pointcloud_tmp.point_step + pointcloud_tmp.fields[dist_idx].offset;
       float * distance_ptr = (dist_idx<0?NULL:(float*)(&pointcloud_tmp.data[distance_ptr_offset]));
       if(!std::isfinite(pt[0]) || !std::isfinite(pt[1]) || !std::isfinite(pt[2]))
       {
           if (distance_ptr == NULL || !std::isfinite(*distance_ptr))   //Invalid point
           {
               pt_out = pt;
           }else{//max range point
               pt[0] = *distance_ptr; //Replace x with the x value saved in distance
               pt_out = transform * pt;//
               max_range_point = true;
           }
       }else{
           pt_out = transform * pt;
       }

       if(max_range_point)
       {
           //Save x. value in distance again
           *(float*)(&pointcloud_tmp.data[distance_ptr_offset]) = pt_out[0];
           pt_out[0] = std::numeric_limits<float> ::quiet_NaN();
       }

       memcpy(&pointcloud_tmp.data[xyz_offset[0]],&pt_out[0], sizeof(float));
       memcpy(&pointcloud_tmp.data[xyz_offset[1]],&pt_out[1], sizeof(float));
       memcpy(&pointcloud_tmp.data[xyz_offset[2]],&pt_out[2], sizeof(float));

       xyz_offset += pointcloud_tmp.point_step;


   }



   //check if the viewpoint infomation is persent
  // int vp_idx = pcl::getFieldIndex(pointcloud_tmp,"vp_x");
  // if()


   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   ///
   ///
   ///

   // pcl_ros::transformPointCloud(Tm,pointcloud_tmp,pointcloud_tmp);


    pcl_ros::transformPointCloud(Tm3,pointcloud_tmp,pointcloud_tmp);
   pcl_ros::transformPointCloud(Tm3,pointcloud_tmp,pointcloud_tmp);

  pcl_ros::transformPointCloud(Tm2,pointcloud_tmp,pointcloud_tmp);
    pcl_ros::transformPointCloud(Tm2,pointcloud_tmp,pointcloud_tmp);


  //  ROS_INFO("5");

    pub.publish(pointcloud_tmp);

}

int main(int argc,char **argv)
{
    ros::init(argc,argv,"laser_geometry_node");
    ros::NodeHandle n;
    ROS_INFO("Now,start the node succeful!");


    fout.open("/home/home.txt");


    sp.set_option(boost::asio::serial_port::baud_rate(115200));
    sp.set_option(boost::asio::serial_port::flow_control());
    sp.set_option(boost::asio::serial_port::parity());
    sp.set_option(boost::asio::serial_port::stop_bits());
    sp.set_option(boost::asio::serial_port::character_size(8));

    ////////begin
    boost::asio::write(sp,boost::asio::buffer("\xAA",1));
    boost::asio::write(sp,boost::asio::buffer("\x55",1));
    boost::asio::write(sp,boost::asio::buffer("\x0A",1));
    boost::asio::write(sp,boost::asio::buffer("\x01",1));
    boost::asio::write(sp,boost::asio::buffer("\x0B",1));




    ROS_INFO("step2111");



    pub=n.advertise<sensor_msgs::PointCloud2>("sync_scan_cloud_filtered",1);
    sub=n.subscribe("first",1,lCallback);

    ros::spin();
    return 0;
}
