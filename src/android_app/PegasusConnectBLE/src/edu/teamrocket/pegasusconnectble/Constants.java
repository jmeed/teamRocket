/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Used under the conditions of the Apache License, Version 2.0, linked above.
 * This file was modified by Patrick D'Agostino, from the
 * googlesamples/android_BluetoothChat repository on GitHub, at
 * https://github.com/googlesamples/android-BluetoothChat/blob/master/Application/src/main/java/com/example/android/bluetoothchat/Constants.java
 */

package edu.teamrocket.pegasusconnectble;

/**
 * Defines several constants used between {@link BluetoothChatService} and the UI.
 */
public interface Constants {

    // Message types sent from the BluetoothService Handler
    public static final int MESSAGE_STATE_CHANGE = 1;
    public static final int MESSAGE_READ = 2;
    public static final int MESSAGE_WRITE = 3;
    public static final int MESSAGE_DEVICE_NAME = 4;
    public static final int MESSAGE_TOAST = 5;
    public static final int MESSAGE_FLIGHT_DATA = 6;
    public static final int MESSAGE_PARAMETER_DATA = 7;
    public static final int MESSAGE_STATUS_DATA = 8;
    public static final int MESSAGE_DISCOVER_DEVICE = 9;
    
    public static final int BLUETOOTH_OUT = 11;
    public static final int BLUETOOTH_IN = 12;
    
    public static final int START_MESSAGE_READ = 13;

    // Key names received from the BluetoothService Handler
    public static final String DEVICE_NAME = "device_name";
    public static final String TOAST = "toast";
    public static final String MAXALT = "max_altitude";
    public static final String MAXACC = "max_acceleration";
    public static final String AVGACC = "avg_acceleration";
    public static final String DESCENT = "descent_rate";
    public static final String DURATION = "duration";
    public static final String MAXSPD = "max_speed";
    public static final String CURALT = "current_altitude";
    public static final String CURSPD = "current_speed";
    
    public static final int NOTIFY_CHANGE = 10;
    
    public static final int HOME = 100;
    public static final int STATUS = 101;
    public static final int FLIGHT = 102;
    public static final int PARAM = 103;
    public static final int PAIR = 104;
    
    public static final String CONNECTED = "connected";
    public static final String GPS_CONNECTED = "gps_connected";
    public static final String IMU_CONNECTED = "imu_connected";
    public static final String FIRING_BOARD_CONNECTED = "firing_board_connected";
    public static final String HIGHG_CONNECTED = "highg_connected";
    public static final String BARO_CONNECTED = "baro_connected";


}