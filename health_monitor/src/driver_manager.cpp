/*
 * Copyright (C) 2019-2020 LEIDOS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

#include "driver_manager.h"

namespace health_monitor
{

    DriverManager::DriverManager() : driver_timeout_(1000) {}

    DriverManager::DriverManager(std::vector<std::string> critical_driver_names, const long driver_timeout, std::vector<std::string> lidar_gps_driver_names) 
    {
        em_ = EntryManager(critical_driver_names,lidar_gps_driver_names);
        driver_timeout_ = driver_timeout;
    }


    void DriverManager::update_driver_status(const cav_msgs::DriverStatusConstPtr& msg, long current_time)
    {
        // Params: bool available, bool active, std::string name, long timestamp, uint8_t type
        Entry driver_status(msg->status == cav_msgs::DriverStatus::OPERATIONAL || msg->status == cav_msgs::DriverStatus::DEGRADED,true, msg->name, current_time, 0, "");
        em_.update_entry(driver_status);
    }

    void DriverManager::evaluate_sensor(int &sensor_input,bool available,long current_time,long timestamp,long driver_timeout)
    {
        if((!available) || (current_time-timestamp > driver_timeout))
           {
            sensor_input=0;
           }
           else
           {
            sensor_input=1;
           }
    }

    std::string DriverManager::are_critical_drivers_operational_truck(long current_time)
    {
        int ssc=0;
        int lidar1=0;
        int lidar2=0;
        int gps=0;
        std::vector<Entry> driver_list = em_.get_entries(); //Real time driver list from driver status
        for(auto i = driver_list.begin(); i < driver_list.end(); ++i)
        {
            if(em_.is_entry_required(i->name_))
            {
              evaluate_sensor(ssc,i->available_,current_time,i->timestamp_,driver_timeout_);
            }

            if(em_.is_lidar_gps_entry_required(i->name_)==0) //Lidar1
            {
               evaluate_sensor(lidar1,i->available_,current_time,i->timestamp_,driver_timeout_);
            }
            else if(em_.is_lidar_gps_entry_required(i->name_)==1) //Lidar2
            {
              evaluate_sensor(lidar2,i->available_,current_time,i->timestamp_,driver_timeout_);
            }
            else if(em_.is_lidar_gps_entry_required(i->name_)==2) //GPS
            {
              evaluate_sensor(gps,i->available_,current_time,i->timestamp_,driver_timeout_);
            }
        }

       //Decision making 

        if(ssc==1)
        {

            if((lidar1==0) && (lidar2==0) && (gps==0))
            {
                return "s_1_l1_0_l2_0_g_0";
            }
            else if((lidar1==0) && (lidar2==0) && (gps==1))
            {
                return "s_1_l1_0_l2_0_g_1";
            }
            else if((lidar1==0) && (lidar2==1) && (gps==0))
            {
                return "s_1_l1_0_l2_1_g_0";
            }
            else if((lidar1==0) && (lidar2==1) && (gps==1))
            {
                return "s_1_l1_0_l2_1_g_1";
            }
            else if((lidar1==1) && (lidar2==0) && (gps==0))
            {
                return "s_1_l1_1_l2_0_g_0";
            }
            else if((lidar1==1) && (lidar2==0) && (gps==1))
            {
                return "s_1_l1_1_l2_0_g_1";
            }
            else if((lidar1==1) && (lidar2==1) && (gps==0))
            {
                return "s_1_l1_1_l2_1_g_0";
            }
            else if((lidar1==1) && (lidar2==1) && (gps==1))
            {
                return "s_1_l1_1_l2_1_g_1";
            }
        }
        else
        {

            return "s_0";
        }

    }


    std::string DriverManager::are_critical_drivers_operational_car(long current_time)
    {
        int ssc=0;
        int lidar=0;
        int gps=0;
        std::vector<Entry> driver_list = em_.get_entries(); //Real time driver list from driver status
        for(auto i = driver_list.begin(); i < driver_list.end(); ++i)
        {
            if(em_.is_entry_required(i->name_))
            {
                evaluate_sensor(ssc,i->available_,current_time,i->timestamp_,driver_timeout_);
            }

            if(em_.is_lidar_gps_entry_required(i->name_)==0) //Lidar
            {
                evaluate_sensor(lidar,i->available_,current_time,i->timestamp_,driver_timeout_);
            }
            else if(em_.is_lidar_gps_entry_required(i->name_)==1) //GPS
            {
                evaluate_sensor(gps,i->available_,current_time,i->timestamp_,driver_timeout_);
            }
        }

        //Decision making 

        if(ssc==1)
        {
            if((lidar==0) && (gps==0))
            {
                return "s_1_l_0_g_0";
            }
            else if((lidar==0) && (gps==1))
            {
                return "s_1_l_0_g_1";
            }
            else if((lidar==1) && (gps==0))
            {
                return "s_1_l_1_g_0";
            }
            else if((lidar==1) && (gps==1))
            {
                return "s_1_l_1_g_1";
            }
        }
        else
        {
            return "s_0";
        }

    }
    
    cav_msgs::SystemAlert DriverManager::handleSpin(bool truck,bool car,long time_now,long start_up_timestamp,long start_duration,bool is_zero)
    {
         cav_msgs::SystemAlert alert;

         if(truck==true)
        {
            if(are_critical_drivers_operational_truck(time_now)=="s_1_l1_1_l2_1_g_1")
            {
                alert.description = "All enssential drivers are ready(SSC,LIDAR1,LIDAR2 and GPS)";
                alert.type = cav_msgs::SystemAlert::DRIVERS_READY;
                return alert;
            } 
           else if(is_zero || time_now - start_up_timestamp <= start_duration)
            {
                alert.description = "System is starting up...";
                alert.type = cav_msgs::SystemAlert::NOT_READY;
                return alert;
            }
            else if((are_critical_drivers_operational_truck(time_now)=="s_1_l1_0_l2_1_g_1") || (are_critical_drivers_operational_truck(time_now)=="s_1_l1_1_l2_0_g_1"))
            {
            
                alert.description = "Only one LIDAR, GPS and SSC is currently working";
                alert.type = cav_msgs::SystemAlert::CAUTION;
                return alert;
            }
            else if((are_critical_drivers_operational_truck(time_now)=="s_1_l1_0_l2_1_g_0") || (are_critical_drivers_operational_truck(time_now)=="s_1_l1_1_l2_0_g_0"))
            {   
                alert.description = "GPS stopped running but one LIDAR and SSC is still working";
                alert.type = cav_msgs::SystemAlert::CAUTION;
                return alert;
            }
            else if(are_critical_drivers_operational_truck(time_now)=="s_1_l1_0_l2_0_g_1")
            {
                alert.description = "Both LIDARS are not running but GPS and SSC is still working";
                alert.type = cav_msgs::SystemAlert::WARNING;
                return alert;
            }
            else if(are_critical_drivers_operational_truck(time_now)=="s_1_l1_0_l2_0_g_0")
            {
                alert.description = "All LIDARS and GPS stopped running, but SSC is working";
                alert.type = cav_msgs::SystemAlert::FATAL;
                return alert;
            }
            else if(are_critical_drivers_operational_truck(time_now)=="s_0")
            {
                alert.description = "SSC stopped working";
                alert.type = cav_msgs::SystemAlert::FATAL;
                return alert;
            }
            else
            {
                alert.description = "UNKNOWN";
                alert.type = cav_msgs::SystemAlert::FATAL;
                return alert;  
            }
 
        }
        else if(car==true)
        {
            if(are_critical_drivers_operational_car(time_now)=="s_1_l_1_g_1")
            {
                alert.description = "All enssential drivers are ready(SSC,LIDAR and GPS)";
                alert.type = cav_msgs::SystemAlert::DRIVERS_READY;
                return alert; 
            }
            else if(is_zero || time_now - start_up_timestamp <= start_duration)
            {
                alert.description = "System is starting up...";
                alert.type = cav_msgs::SystemAlert::NOT_READY;
                return alert; 
            } 
            else if(are_critical_drivers_operational_car(time_now)=="s_1_l_1_g_0")
            {
                alert.description = "GPS stopped working but SSC and LIDAR  is still running";
                alert.type = cav_msgs::SystemAlert::CAUTION;
                return alert; 
            }
            else if(are_critical_drivers_operational_car(time_now)=="s_1_l_0_g_1")
            {
                alert.description = "LIDAR stopped working but SSC and GPS is still running";
                alert.type = cav_msgs::SystemAlert::WARNING;
                return alert; 
            }
            else if(are_critical_drivers_operational_car(time_now)=="s_1_l_0_g_0")
            {
                alert.description = "LIDAR, GPS stopped working but SSC is still running";
                alert.type = cav_msgs::SystemAlert::FATAL;
                return alert; 
            }
            else if(are_critical_drivers_operational_car(time_now)=="s_0")
            {
                alert.description = "SSC stopped working";
                alert.type = cav_msgs::SystemAlert::FATAL;
                return alert; 
            }
            else
            {
                alert.description = "UNKNOWN";
                alert.type = cav_msgs::SystemAlert::FATAL;
                return alert;  
            }
        }
        else
        {
            alert.description = "UNKNOWN";
            alert.type = cav_msgs::SystemAlert::FATAL;
            return alert; 
        }
    }

}

