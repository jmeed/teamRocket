/*
 * Copyright (C) 2014 Microchip Technology Inc. and its subsidiaries.  You may use this software and any derivatives
 * exclusively with Microchip products.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS
 * SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE,
 * COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF
 * THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON
 * ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID
 * DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * This file includes code modified from "The Android Open Source Project" copyright (C) 2013.
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
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE TERMS. 
 */

package edu.teamrocket.pegasusconnectble;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;
import java.util.List;
import java.util.UUID;

    // ----------------------------------------------------------------------------------------------------------------
    // A service for managing the connection and data communication with a GATT server on a Bluetooth LE device.
public class BluetoothLeService extends Service {

    private final static String TAG = BluetoothLeService.class.getSimpleName();         //Get name of service to tag debug and warning messages
    private BluetoothManager mBluetoothManager;
    private BluetoothAdapter mBluetoothAdapter;                                         //The BluetoothAdapter controls the BLE radio in the phone/tablet
    private BluetoothGatt mBluetoothGatt;                                               //BluetoothGatt controls the Bluetooth communication link
    private String mBluetoothDeviceAddress;                                             //Address of the connected BLE device

    public final static String ACTION_GATT_CONNECTED = "edu.teamrocket.pegasusconnectble.ACTION_GATT_CONNECTED"; //Strings representing actions to broadcast to activities
    public final static String ACTION_GATT_DISCONNECTED = "edu.teamrocket.pegasusconnectble.ACTION_GATT_DISCONNECTED";
    public final static String ACTION_GATT_SERVICES_DISCOVERED = "edu.teamrocket.pegasusconnectble.ACTION_GATT_SERVICES_DISCOVERED";
    public final static String ACTION_DATA_AVAILABLE = "edu.teamrocket.pegasusconnectble.ACTION_DATA_AVAILABLE";
    public final static String ACTION_DATA_WRITTEN = "edu.teamrocket.pegasusconnectble.ACTION_DATA_WRITTEN";
    public final static String EXTRA_DATA = "edu.teamrocket.pegasusconnectble.EXTRA_DATA";

    public final static UUID UUID_MLDP_DATA_PRIVATE_CHARACTERISTIC = UUID.fromString(MainActivity.MLDP_DATA_PRIVATE_CHAR);
    public final static UUID UUID_CHARACTERISTIC_NOTIFICATION_CONFIG = UUID.fromString(MainActivity.CHARACTERISTIC_NOTIFICATION_CONFIG);

    private final IBinder mBinder = new LocalBinder();                                  //Binder for Activity that binds to this Service

    
    // ----------------------------------------------------------------------------------------------------------------
    // An activity has bound to this service
    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;                                                                 //Return LocalBinder when an Activity binds to this Service
    }

    // ----------------------------------------------------------------------------------------------------------------
    // An activity has unbound from this service 
    @Override
    public boolean onUnbind(Intent intent) {
        if (mBluetoothGatt != null) {                                                   //Check for existing BluetoothGatt connection
            mBluetoothGatt.close();                                                     //Close BluetoothGatt coonection for proper cleanup
            mBluetoothGatt = null;                                                      //No longer have a BluetoothGatt connection
        }
        return super.onUnbind(intent);
    }

    // ----------------------------------------------------------------------------------------------------------------
    // A Binder to return to an activity to let it bind to this service 
    public class LocalBinder extends Binder {
        BluetoothLeService getService() {
            return BluetoothLeService.this;                                             //Return this instance of BluetoothLeService so clients can call its public methods
        }
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Implements callback methods for GATT events that the app cares about.  For example: connection change and services discovered.
    // When onConnectionStateChange() is called with newState = STATE_CONNECTED then it calls mBluetoothGatt.discoverServices()
    // resulting in another callback to onServicesDiscovered()
    private final BluetoothGattCallback mGattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) { //Change in connection state
            if (newState == BluetoothProfile.STATE_CONNECTED) {                         //See if we are connected
                broadcastUpdate(ACTION_GATT_CONNECTED);                                 //Go broadcast an intent to say we are connected
                Log.i(TAG, "Connected to GATT server, starting service discovery");
                mBluetoothGatt.discoverServices();                                      //Discover services on connected BLE device
            } 
            else if (newState == BluetoothProfile.STATE_DISCONNECTED) {                 //See if we are not connected
                broadcastUpdate(ACTION_GATT_DISCONNECTED);                              //Go broadcast an intent to say we are disconnected
                Log.i(TAG, "Disconnected from GATT server.");
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {              //BLE service discovery complete
            if (status == BluetoothGatt.GATT_SUCCESS) {                                 //See if the service discovery was successful
                broadcastUpdate(ACTION_GATT_SERVICES_DISCOVERED);                       //Go broadcast an intent to say we have discovered services
            } 
            else {                                                                      //Service discovery failed so log a warning
                Log.w(TAG, "onServicesDiscovered received: " + status);
            }
        }

        //For information only. This application uses Indication to receive updated characteristic data, not Read
        @Override
        public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) { //A request to Read has completed
            if (status == BluetoothGatt.GATT_SUCCESS) {                                 //See if the read was successful
                //broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);                 //Go broadcast an intent with the characteristic data
            }
        }

        //For information only. This application sends small packets infrequently and does not need to know what the previous write completed
        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) { //A request to Write has completed
            if (status == BluetoothGatt.GATT_SUCCESS) {                                 //See if the write was successful
                broadcastUpdate(ACTION_DATA_WRITTEN, characteristic);                   //Go broadcast an intent to say we have have written data
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) { //Indication or notification was received
            broadcastUpdate(ACTION_DATA_AVAILABLE, characteristic);                     //Go broadcast an intent with the characteristic data
        }
    };

    // ----------------------------------------------------------------------------------------------------------------
    // Broadcast an intent with a string representing an action
    private void broadcastUpdate(final String action) {
        final Intent intent = new Intent(action);                                       //Create new intent to broadcast the action
        sendBroadcast(intent);                                                          //Broadcast the intent
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Broadcast an intent with a string representing an action an extra string with the data
    // Modify this code for data that is not in a string format 
    private void broadcastUpdate(final String action, final BluetoothGattCharacteristic characteristic) {
        final Intent intent = new Intent(action);                                       //Create new intent to broadcast the action
        if(action.equals(ACTION_DATA_AVAILABLE)) {                                      //See if we need to send data
            if (UUID_MLDP_DATA_PRIVATE_CHARACTERISTIC.equals(characteristic.getUuid())) { //See if this is the correct characteristic 
                String dataValue = characteristic.getStringValue(0);                    //Get the data (in this case it is a string)
                intent.putExtra(EXTRA_DATA, dataValue);                                 //Add the data string to the intent
            }
        }
        else {                                                                          //Did not get an action string we expect 
            Log.d(TAG, "Action: " + action);
        }
        sendBroadcast(intent);                                                          //Broadcast the intent
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Open a BluetoothGatt connection to a BLE device given its address
    public boolean connect(final String address) {
        if (mBluetoothAdapter == null || address == null) {                             //Check that we have a Bluetooth adappter and device address
            Log.w(TAG, "BluetoothAdapter not initialized or unspecified address.");     //Log a warning that something went wrong
            return false;                                                               //Failed to connect
        }

        // Previously connected device.  Try to reconnect.
        if (mBluetoothDeviceAddress != null && address.equals(mBluetoothDeviceAddress) && mBluetoothGatt != null) { //See if there was previous connection to the device
            Log.d(TAG, "Trying to use an existing mBluetoothGatt for connection.");
            if (mBluetoothGatt.connect()) {                                             //See if we can connect with the existing BluetoothGatt to connect
                return true;                                                            //Success
            } 
            else {
                return false;                                                           //Were not able to connect
            }
        }

        final BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);      //No previous device so get the Bluetooth device by referencing its address
        if (device == null) {                                                           //Check whether a device was returned
            Log.w(TAG, "Device not found.  Unable to connect.");                        //Warn that something went wrong
            return false;                                                               //Failed to find the device
        }

        mBluetoothGatt = device.connectGatt(this, false, mGattCallback);                //Directly connect to the device so autoConnect is false
        Log.d(TAG, "Trying to create a new connection.");
        mBluetoothDeviceAddress = address;                                              //Record the address in case we bneed to reconnect with the existing BluetoothGatt
        return true;
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Retrieve and return a list of supported GATT services on the connected device
    public List<BluetoothGattService> getSupportedGattServices() {
        if (mBluetoothGatt == null) {                                                   //Check that we have a valid GATT connection
            return null;
        }
        return mBluetoothGatt.getServices();                                            //Get the list of services
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Disconnects an existing connection or cancel a pending connection
    // BluetoothGattCallback.onConnectionStateChange() will get the result
    public void disconnect() {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {                      //Check that we have a GATT connection to disconnect
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        mBluetoothGatt.disconnect();                                                    //Disconnect GATT connection
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Request a read of a given BluetoothGattCharacteristic. The Read result is reported asynchronously through the
    // BluetoothGattCallback onCharacteristicRead callback method.
    // For information only. This application uses Indication to receive updated characteristic data, not Read
    public void readCharacteristic(BluetoothGattCharacteristic characteristic) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {                      //Check that we have access to a Bluetooth radio
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }

        mBluetoothGatt.readCharacteristic(characteristic);                              //Request the BluetoothGatt to Read the characteristic
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Write to a given characteristic. The completion of the write is reported asynchronously through the
    // BluetoothGattCallback onCharacteristicWrire callback method.
    public void writeCharacteristic(BluetoothGattCharacteristic characteristic) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {                      //Check that we have access to a Bluetooth radio
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        int test = characteristic.getProperties();                                      //Get the properties of the characteristic
        if ((test & BluetoothGattCharacteristic.PROPERTY_WRITE) == 0 && (test & BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE) == 0) { //Check that the property is writable 
            return;
        }

        if (mBluetoothGatt.writeCharacteristic(characteristic)) {                       //Request the BluetoothGatt to do the Write
            Log.d(TAG, "writeCharacteristic successful");                               //The request was accepted, this does not mean the write completed
        } 
        else {
            Log.d(TAG, "writeCharacteristic failed");                                   //Write request was not accepted by the BluetoothGatt
        }
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Enable notification on a characteristic
    // For information only. This application uses Indication, not Notification
    public void setCharacteristicNotification(BluetoothGattCharacteristic characteristic, boolean enabled) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {                      //Check that we have a GATT connection
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        mBluetoothGatt.setCharacteristicNotification(characteristic, enabled);          //Enable notification and indication for the characteristic

//        if (UUID_MLDP_DATA_PRIVATE_CHARACTERISTIC.equals(characteristic.getUuid())) { 
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(UUID_CHARACTERISTIC_NOTIFICATION_CONFIG); //Get the descripter that enables notification on the server
        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);         //Set the value of the descriptor to enable notification
        mBluetoothGatt.writeDescriptor(descriptor);                                     //Write the descriptor
//        }
    }
    // ----------------------------------------------------------------------------------------------------------------
    // Initialize by getting the BluetoothManager and BluetoothAdapter 
    public boolean initialize() {
        if (mBluetoothManager == null) {                                                //See if we do not already have the BluetoothManager
            mBluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE); //Get the BluetoothManager
            if (mBluetoothManager == null) {                                            //See if we failed
                Log.e(TAG, "Unable to initialize BluetoothManager.");
                return false;                                                           //Report the error
            }
        }

        mBluetoothAdapter = mBluetoothManager.getAdapter();                             //Ask the BluetoothManager to get the BluetoothAdapter
        if (mBluetoothAdapter == null) {                                                //See if we failed
            Log.e(TAG, "Unable to obtain a BluetoothAdapter.");
            return false;                                                               //Report the error
        }

        return true;                                                                                                       //Success, we have a BluetoothAdapter to control the radio
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Enable indication on a characteristic
    public void setCharacteristicIndication(BluetoothGattCharacteristic characteristic, boolean enabled) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) {                      //Check that we have a GATT connection
            Log.w(TAG, "BluetoothAdapter not initialized");
            return;
        }
        mBluetoothGatt.setCharacteristicNotification(characteristic, enabled);          //Enable notification and indication for the characteristic

        // This is specific to our custom profile
//        if (UUID_MLDP_DATA_PRIVATE_CHARACTERISTIC.equals(characteristic.getUuid())) {
        BluetoothGattDescriptor descriptor = characteristic.getDescriptor(UUID_CHARACTERISTIC_NOTIFICATION_CONFIG); //Get the descripter that enables indication on the server
        descriptor.setValue(BluetoothGattDescriptor.ENABLE_INDICATION_VALUE);           //Set the value of the descriptor to enable indication
        mBluetoothGatt.writeDescriptor(descriptor);                                     //Write the descriptor
//        }
    }
    
}
