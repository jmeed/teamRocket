package edu.teamrocket.pegasusconnectble;

import android.support.v7.app.ActionBarActivity;
import android.support.v7.app.ActionBar;
import android.support.v4.app.Fragment;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;

import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

import edu.teamrocket.pegasusconnectble.BluetoothLeService;

import edu.teamrocket.pegasusconnectble.R;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;
import android.widget.Toast;
import android.app.AlertDialog;
import android.app.ProgressDialog;


//Bluetooth Adapter we are working with
//http://www.mouser.com/ProductDetail/Microchip-Technology/RN4020-V-RM/?qs=sGAEpiMZZMsjLMBIknjmko2zmeVevXBl0yBSR12fEkVlMpaBXEso2g%3D%3D

public class MainActivity extends ActionBarActivity implements
		ActionBar.OnNavigationListener {

	/**
	 * The serialization (saved instance state) Bundle key representing the
	 * current dropdown position.
	 */
	String NAME = "btConn";
	private static final String STATE_SELECTED_NAVIGATION_ITEM = "selected_navigation_item";
	private BluetoothAdapter bAdapter;
	private static Set<BluetoothDevice> bAllDevices;
	private int Units = METRIC_UNITS;
	private static BluetoothDevice paired_device;
	private static String paired_device_name;
	public static String message_to_send = "";
	public static String message_received = "";
	public String TAG = "BlueTooth Host Activity";

	private static final String MLDP_PRIVATE_SERVICE = "00035b03-58e6-07dd-021a-08123a000300"; 
	public static final String MLDP_DATA_PRIVATE_CHAR = "00035b03-58e6-07dd-021a-08123a000301";
	private static final String MLDP_CONTROL_PRIVATE_CHAR = "00035b03-58e6-07dd-021a-08123a0003ff";
	public static final String CHARACTERISTIC_NOTIFICATION_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";
	public static final String GET_1st_LAST_FLIGHT = "F1";
	public static final String GET_2nd_LAST_FLIGHT = "F2";
	public static final String GET_3rd_LAST_FLIGHT = "F3";
	public static final String GET_4th_LAST_FLIGHT = "F4";

	private static final int IMPERIAL_UNITS = 8;
	private static final int METRIC_UNITS = 9;
	public static final int STATE_CONNECTION_STARTED = 0;
	public static final int STATE_CONNECTION_LOST = 1;
	public static final int READY_TO_CONN = 2;
	public static final int REQUEST_ENABLE_BT = 4;
	public static final int SELECT_DEVICE_BT = 3;
	
	public static final int SCAN_PERIOD = 4000;
	private Handler mHandler  = new Handler();

	private BluetoothLeService bService;
	static Handler currentHandler;
	private BluetoothGattCharacteristic mControlMLDP;
	private BluetoothGattCharacteristic mDataMLDP;

	static UUID uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
	BroadcastReceiver receiver;
	BluetoothSocket bSocket;
	ArrayList<BluetoothSocket> bAllSockets = new ArrayList<BluetoothSocket>();
	ArrayList<BluetoothDevice> bFoundDevices = new ArrayList<BluetoothDevice>();
	int bArrayAdapterSize;
	StringBuffer outputBuffer;
	String input_additive_buffer;
	String secondary_additive_buffer;
	boolean complete_message;
	boolean mConnected;
	public static int current_fragment;

	ProgressDialog progress;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		// Set up the action bar to show a dropdown list.
		final ActionBar actionBar = getSupportActionBar();
		actionBar.setDisplayShowTitleEnabled(false);
		actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);

		// Set up the dropdown list navigation in the action bar.
		actionBar.setListNavigationCallbacks(
		// Specify a SpinnerAdapter to populate the dropdown list.
				new ArrayAdapter<String>(actionBar.getThemedContext(),
						android.R.layout.simple_list_item_1,
						android.R.id.text1, new String[] {
								getString(R.string.action_home),
								getString(R.string.View_Status),
								getString(R.string.view_flight_data),
								getString(R.string.Set_Parameters),
								getString(R.string.pair_your_pegasus) }), this);

		// find out if there is a Bluetooth adapter to use and if so get a
		// handle for it
		bAdapter = BluetoothAdapter.getDefaultAdapter();

		if (bAdapter == null) {
			// create dialog to tell the user they don't have a bluetooth module
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("No Bluetooth");
			alertDialog
					.setMessage("Bluetooth does not appear to be supported on this device.");
			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "OK",
					new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int which) {
							dialog.dismiss();
							finish();
						}
					});
			// display alert
			alertDialog.show();
		}
		// no paired device
		paired_device = null;
		paired_device_name = null;
		// set metric by default
		Units = METRIC_UNITS;

		if (!bAdapter.isEnabled()) {
			Intent enableIntent = new Intent(
					BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivityForResult(enableIntent, MainActivity.REQUEST_ENABLE_BT);
		}
		
		outputBuffer = new StringBuffer("");

		input_additive_buffer = new String("");
		secondary_additive_buffer = new String("");
		complete_message = false;
		mConnected = false;
		
        Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);          //Create Intent to start the BluetoothLeService 
        bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);           //Create and bind the new service to mServiceConnection object that handles service connect and disconnect 	}
	}
	
    // ----------------------------------------------------------------------------------------------------------------
    // Code to manage Service lifecycle.
    private final ServiceConnection mServiceConnection = new ServiceConnection() {		//Create new ServiceConnection interface to handle connection and disconnection
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder service) {	//Service connects
        	Log.i(TAG,"service being connected");
            bService = ((BluetoothLeService.LocalBinder) service).getService(); //Get a link to the service
            if (!bService.initialize()) {                                    //See if the service did not initialize properly
                Log.e(TAG, "Unable to initialize Bluetooth");
                finish();																//End the application
            }
            if(paired_device_name != null) {
            	bService.connect(paired_device_name);                                //Connects to the device selected and passed to us by the DeviceScanActivity
            }
            }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {                //Service disconnects
            bService = null;                                                 //Not bound to a service
        }
    };

	@Override
	protected void onResume() {
		super.onResume();
		 Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);          //Create Intent to start the BluetoothLeService 
	     bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);           //Create and bind the new service to mServiceConnection object that handles service connect and disconnect 	}
		
		registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter()); 
		if (bService != null && paired_device != null) { // Check that service is running
			final boolean result = bService.connect(paired_device.getAddress()); 
			Log.d(TAG, "Connect request result = " + result);
		}
	}

	public void onDestroy() {
		super.onDestroy();
		unregisterReceiver(mGattUpdateReceiver);
        unbindService(mServiceConnection);                                              //Activity ending so unbind the service (this will end the service if no other activities are bound to it)

	}

	// Intent filter to add Intent values that will be broadcast by the
	// BluetoothLeService to the mGattUpdateReceiver BroadcastReceiver
	private static IntentFilter makeGattUpdateIntentFilter() {
		final IntentFilter intentFilter = new IntentFilter();
		
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED);
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
		intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE);
		intentFilter.addAction(BluetoothLeService.ACTION_DATA_WRITTEN);
		return intentFilter;
	}

	public Set<BluetoothDevice> get_all_paired_devices(BluetoothAdapter adapter) {
		if (!adapter.isEnabled()) {
			Toast toast = Toast
					.makeText(
							this,
							"Bluetooth not enabled.  Please enable it to use this feature.",
							Toast.LENGTH_LONG);
			toast.show();
			Set<BluetoothDevice> empty = null;
			return empty;
		}
		Set<BluetoothDevice> paired_devices = adapter.getBondedDevices();
		return paired_devices;
	}

	@Override
	public void onRestoreInstanceState(Bundle savedInstanceState) {
		// Restore the previously serialized current dropdown position.
		if (savedInstanceState.containsKey(STATE_SELECTED_NAVIGATION_ITEM)) {
			getSupportActionBar().setSelectedNavigationItem(
					savedInstanceState.getInt(STATE_SELECTED_NAVIGATION_ITEM));
		}
	}

	@Override
	public void onSaveInstanceState(Bundle outState) {
		// Serialize the current dropdown position.
		outState.putInt(STATE_SELECTED_NAVIGATION_ITEM, getSupportActionBar()
				.getSelectedNavigationIndex());
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return super.onCreateOptionsMenu(menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	@Override
	public boolean onNavigationItemSelected(int position, long id) {
		// When the given dropdown item is selected, show its contents in the
		// container view.
		android.support.v4.app.FragmentManager fm = getSupportFragmentManager();
		Message writeMsg = handler.obtainMessage(Constants.MESSAGE_WRITE);
		Bundle writeBnd = new Bundle();
		
		switch (position) {
		case 0:
			fm.beginTransaction()
					.replace(R.id.container, HomeFragment.newInstance(), "Home")
					.commit();
			current_fragment = Constants.HOME;
			break;
			
		case 1:
			fm.beginTransaction()
					.replace(R.id.container, StatusFragment.newInstance(),
							"Status").commit();
			if(paired_device != null ) {
				writeBnd.putString("data", "stat");
				writeMsg.setData(writeBnd);
				handler.sendMessage(writeMsg);
				writeBnd = new Bundle();
				
				handler.sendEmptyMessage(Constants.START_MESSAGE_READ);
			}
			current_fragment = Constants.STATUS;
			break;
			
		case 2:
			fm.beginTransaction()
					.replace(R.id.container, FlightDataFragment.newInstance(),
							"FlightData").commit();

			if(paired_device != null ) {
				current_fragment = Constants.FLIGHT;
				new Thread( new Runnable() {
					public void run() {
						while( current_fragment == Constants.FLIGHT ) {
							Message writeMsg = handler.obtainMessage(Constants.MESSAGE_WRITE);
							Bundle writeBnd = new Bundle();
							writeBnd.putString("data", "fld");
							writeMsg.setData(writeBnd);
							handler.sendMessage(writeMsg);
							try {
								TimeUnit.MILLISECONDS.sleep(500);
							}
							catch(InterruptedException e) {
								Log.d(TAG, "sleep interrupted");
							}
							handler.sendEmptyMessage(Constants.START_MESSAGE_READ);
							try {
								TimeUnit.MILLISECONDS.sleep(500);
							}
							catch(InterruptedException e) {
								Log.d(TAG, "sleep interrupted");
							}
						}
					}
				}).start();
				
			}
			
			break;
			
		case 3:
			Context context = getActivity();
			fm.beginTransaction()
					.replace(R.id.container, ParametersFragment.newInstance(),
							"Parameters").commit();
			if(paired_device != null) {
				writeBnd.putString("data", "par");
				writeMsg.setData(writeBnd);
				handler.sendMessage(writeMsg);
				
				handler.sendEmptyMessage(Constants.START_MESSAGE_READ);
			}
			(Toast.makeText(context, "Parameters not yet implemented", Toast.LENGTH_LONG)).show();
			current_fragment = Constants.PARAM;
			break;
			
		case 4:
			fm.beginTransaction()
					.replace(R.id.container, PairDeviceFragment.newInstance(),
							"PairDevice").commit();
			choose_device();
			current_fragment = Constants.PAIR;
			
			break;
		}

		return true;
	}

	/**
	 * A placeholder fragment containing a simple view.
	 */
	public static class HomeFragment extends Fragment {

		public static HomeFragment newInstance() {
			HomeFragment fragment = new HomeFragment();
			return fragment;
		}

		public HomeFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
			View rootView = inflater.inflate(R.layout.fragment_main, container,
					false);
			return rootView;
		}
	}

	public static class StatusFragment extends Fragment {

		public static StatusFragment newInstance() {
			StatusFragment fragment = new StatusFragment();
			return fragment;
		}

		public StatusFragment() {
		}

		public void onResume() {
			super.onResume();
			currentHandler = new Handler(Looper.getMainLooper()) {
				@Override
				public void handleMessage(Message msg) {

					switch (msg.what) {
					case Constants.MESSAGE_STATUS_DATA:
						// update necessary textviews
						if(msg.getData().getChar(Constants.CONNECTED) == 'y') {
							((TextView)getView().findViewById(R.id.connected)).setText(R.string.device_connected);
						}
						if(msg.getData().getChar(Constants.GPS_CONNECTED) == 'y') {
							((TextView)getView().findViewById(R.id.gps_detected)).setText(R.string.gps_detected);
						}
						if(msg.getData().getChar(Constants.IMU_CONNECTED) == 'y') {
							((TextView)getView().findViewById(R.id.imu_detected)).setText(R.string.imu_detected);
						}
						if(msg.getData().getChar(Constants.HIGHG_CONNECTED) == 'y') {
							((TextView)getView().findViewById(R.id.highg_detected)).setText(R.string.highg_detected);
						}						
						if(msg.getData().getChar(Constants.FIRING_BOARD_CONNECTED) == 'y') {
							((TextView)getView().findViewById(R.id.firing_board_detected)).setText(R.string.firing_board_detected);
						}	
						if(msg.getData().getChar(Constants.BARO_CONNECTED) == 'y') {
							((TextView)getView().findViewById(R.id.baro_detected)).setText(R.string.baro_detected);
						}	
						break;
					case Constants.MESSAGE_FLIGHT_DATA:
						break;
					default:
						Log.e("curHandler", "Status fragment failed to understand internal message of type: " + msg.what);
						break;

					} // end switch
				};
			};
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
			View rootView = inflater.inflate(R.layout.fragment_view_status,
					container, false);
			return rootView;
		}
	}

	public static class FlightDataFragment extends Fragment {

		public static FlightDataFragment newInstance() {
			FlightDataFragment fragment = new FlightDataFragment();
			return fragment;
		}

		public FlightDataFragment() {
		}

		public void onResume() {
			super.onResume();
			currentHandler = new Handler(Looper.getMainLooper()) {
				@Override
				public void handleMessage(Message msg) {
					switch (msg.what) {
					case Constants.MESSAGE_FLIGHT_DATA:
						((TextView) getView().findViewById(
								R.id.data_max_altitude)).setText(msg.getData()
								.getString(Constants.MAXALT));
						((TextView) getView().findViewById(
								R.id.data_max_acceleration)).setText(msg
								.getData().getString(Constants.MAXACC));
						((TextView) getView().findViewById(
								R.id.data_cur_speed)).setText(msg
								.getData().getString(Constants.CURSPD));
						((TextView) getView().findViewById(
								R.id.data_descent_rate)).setText(msg.getData()
								.getString(Constants.DESCENT));
						((TextView) getView().findViewById(R.id.data_duration))
								.setText(msg.getData().getString(
										Constants.DURATION));
						((TextView) getView().findViewById(R.id.data_max_speed))
								.setText(msg.getData().getString(
										Constants.MAXSPD));
						((TextView) getView().findViewById(R.id.data_current_altitude))
								.setText(msg.getData().getString(
										Constants.CURALT));
						
						//set metric units
						if(msg.getData().getInt("units") == METRIC_UNITS) {
							((TextView) getView().findViewById(
									R.id.max_altitude_units)).setText("meters");
							((TextView) getView().findViewById(
									R.id.current_altitude_units)).setText("meters");
							((TextView) getView().findViewById(
									R.id.descent_rate_units)).setText("m/s");
							((TextView) getView().findViewById(R.id.speed_units))
									.setText("m/s");
							((TextView) getView().findViewById(
									R.id.cur_speed_units)).setText("m/s");
						}
						//set imperial units
						else {
							((TextView) getView().findViewById(
									R.id.max_altitude_units)).setText("feet");
							((TextView) getView().findViewById(
									R.id.current_altitude_units)).setText("feet");
							((TextView) getView().findViewById(
									R.id.descent_rate_units)).setText("mph");
							((TextView) getView().findViewById(R.id.speed_units))
									.setText("mph");
							((TextView) getView().findViewById(
									R.id.cur_speed_units)).setText("mph");
						}
						
						//Set units that are the same for both metric and imperial
						((TextView) getView().findViewById(
								R.id.max_acceleration_units)).setText("G's");
						((TextView) getView().findViewById(R.id.duration_units))
								.setText("seconds");

						break;
					default:
						Log.e("curHandler", "Flight Data fragment failed to understand internal message of type: " + msg.what);

						break;

					} // end switch
				};
			};
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
			View rootView = inflater.inflate(
					R.layout.fragment_view_flight_data, container, false);
			return rootView;
		}
	}

	public static class ParametersFragment extends Fragment {

		public static ParametersFragment newInstance() {
			ParametersFragment fragment = new ParametersFragment();
			return fragment;
		}

		public ParametersFragment() {
		}

		public void onResume() {
			super.onResume();
			currentHandler = new Handler(Looper.getMainLooper()) {
				@Override
				public void handleMessage(Message msg) {

					switch (msg.what) {
					case Constants.MESSAGE_PARAMETER_DATA:
						// update necessary textviews
						//((TextView)getView().findViewById(R.id.data_pair_device)).setText(msg.getData().getString(Constants.MAXALT));
						break;
					default:
						Log.e("curHandler", "Status fragment failed to understand internal message of type: " + msg.what);
						break;

					} // end switch
				};
			};
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
			View rootView = inflater.inflate(R.layout.fragment_set_parameters,
					container, false);
			return rootView;
		}
	}

	public static class PairDeviceFragment extends Fragment {
		public static PairDeviceFragment newInstance() {
			PairDeviceFragment fragment = new PairDeviceFragment();
			return fragment;
		}

		public PairDeviceFragment() {
		}

		public void onResume() {
			super.onResume();
			
			if (paired_device_name != null) {
				((TextView)getView().findViewById(R.id.data_pair_device))
						.setText(paired_device_name);
			}
			
			currentHandler = new Handler(Looper.getMainLooper()) {
				@Override
				public void handleMessage(Message msg) {
					Context context = getActivity();

					switch (msg.what) {
					case Constants.MESSAGE_DEVICE_NAME:
						// update necessary textviews
						(Toast.makeText(context, "Device Name: "
								+ msg.getData()
										.getString(Constants.DEVICE_NAME),
								Toast.LENGTH_LONG)).show();

						((TextView) getView().findViewById(
								R.id.data_pair_device)).setText(msg.getData()
								.getString(Constants.DEVICE_NAME));
						break;
					case Constants.MESSAGE_FLIGHT_DATA:
						break;
					default:
						Log.e("curHandler", "Status fragment failed to understand internal message of type: " + msg.what);
						break;

					} // end switch
				};
			};
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {
			View rootView = inflater.inflate(R.layout.fragment_pair_device,
					container, false);
			return rootView;
		}
	}
	
	

	// The below need filled out for the functionality of the buttons, if we
	// want them

	// right now this just tests if the button works
	public void set_metric_units(View view) {
		Toast toast = Toast.makeText(this, "Setting app to use metric units",
				Toast.LENGTH_LONG);
		toast.show();
		this.Units = MainActivity.METRIC_UNITS;
	}

	public void set_imperial_units(View view) {
		Toast toast = Toast.makeText(this, "Setting app to use imperial units",
				Toast.LENGTH_LONG);
		toast.show();

		this.Units = MainActivity.IMPERIAL_UNITS;
	}

	public void choose_device() {
		final Context context = getActivity();
		MainActivity.bAllDevices = get_all_paired_devices(this.bAdapter);
		AlertDialog.Builder builder = new AlertDialog.Builder(context);
		int counter = 0;
		final CharSequence[] names = new CharSequence[MainActivity.bAllDevices
				.size() + 1];
		for (BluetoothDevice dev : MainActivity.bAllDevices) {
			names[counter] = dev.getName().toString();
			counter++;
		}
		names[counter] = "Find A Device";

		if (this.bAdapter == null) {
			Toast toast = Toast.makeText(this, "No BluetoothDevices returned",
					Toast.LENGTH_LONG);
			toast.show();
		} else if (paired_device == null) {
			builder.setTitle(R.string.choose_device)
					.setNegativeButton(R.string.Close,
							new DialogInterface.OnClickListener() {
								@Override
								public void onClick(DialogInterface dialog,
										int which) {
									// Just close the dialog - done
									// automatically on button press
								}
							})
					.setItems(names, new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int which) {
							if (which < bAllDevices.size()) {
								BluetoothDevice temp = (BluetoothDevice) bAllDevices
										.toArray()[which];
								// connect to bluetooth
								bService.connect(temp.getAddress());
							} else {
								discover_new_device();
							}
						}
					});
			builder.show();
		}
	} // set_parameters

	public void change_device(View view) {
		paired_device = null;
		paired_device_name = null;
		choose_device();
	}

	public Context getActivity() {
		return this;
	}

	public String getPairedDevice() {
		if (paired_device != null) {
			return paired_device.getName();
		} else {
			return null;
		}
	}

	public void discover_new_device() {
		scanLeDevice();
	}

	Handler handler = new Handler(Looper.getMainLooper()) {
		@Override
		public void handleMessage(Message msg) {
			Context context = getActivity();

			switch (msg.what) {
			// choosing a device

			case Constants.MESSAGE_STATE_CHANGE:
				break;
				
			case Constants.START_MESSAGE_READ:
				if( bService != null ) {
					bService.readCharacteristic(mDataMLDP);
				}
				break;

			case Constants.MESSAGE_DISCOVER_DEVICE:
				if (bFoundDevices.size() > 0) {
					AlertDialog.Builder builder2 = new AlertDialog.Builder(context);
					int counter = 0;
					final CharSequence[] names = new CharSequence[bFoundDevices.size()];

					for (BluetoothDevice dev : bFoundDevices) {
						Log.i(TAG, dev.getAddress());
						if(dev.getName() != null) {
							names[counter] = dev.getName();
						}
						else {
							names[counter] = dev.getAddress();
						}
						counter++;
					}

					builder2.setTitle(R.string.choose_device)
							.setNegativeButton(R.string.Close,
									new DialogInterface.OnClickListener() {
										@Override
										public void onClick(
												DialogInterface dialog,
												int which) {
											// Just close the dialog - done
											// automatically on button press
										}
									})
							.setItems(names,
									new DialogInterface.OnClickListener() {
										public void onClick(DialogInterface dialog, int which) {
											BluetoothDevice temp = (BluetoothDevice) bFoundDevices.toArray()[which];
											// connect to bluetooth
											if( bService.connect(temp.getAddress()) ) {
												Message tmp = handler.obtainMessage(Constants.MESSAGE_DEVICE_NAME);
												Bundle devName = new Bundle();
												devName.putString(Constants.DEVICE_NAME, temp.getAddress());
												tmp.setData(devName);
												paired_device = temp;
												handler.sendMessage(tmp);
											}
											else {
												Message tmp = handler.obtainMessage(Constants.MESSAGE_TOAST);
												Bundle error = new Bundle();
												error.putString(Constants.TOAST, "Unable to connect to device " + temp.getName());
												tmp.setData(error);
												handler.sendMessage(tmp);
											}
										}
									});
					builder2.show();
				} else {
					(Toast.makeText(
							context,
							"No Devices Found.  Make sure the other device is discoverable.",
							Toast.LENGTH_LONG)).show();
				}
				break;

			case Constants.MESSAGE_DEVICE_NAME:
				paired_device_name = msg.getData().getString(
						Constants.DEVICE_NAME);
				Message outgoingmsg = currentHandler
						.obtainMessage(Constants.MESSAGE_DEVICE_NAME);
				Bundle bundle = new Bundle();
				bundle.putString(Constants.DEVICE_NAME, paired_device_name);
				outgoingmsg.setData(bundle);
				currentHandler.sendMessage(outgoingmsg);
				break;

			case Constants.MESSAGE_READ:
				// parse message from bluetooth, get type (flight, status, params) and then the data
				String readMessage = msg.getData().getString("data");
				compile_messages(readMessage);
				if (complete_message) {
					processMessage(input_additive_buffer);
					Log.i(TAG, input_additive_buffer);
					complete_message = false;
					input_additive_buffer = "";
					input_additive_buffer = secondary_additive_buffer;
					secondary_additive_buffer = "";
					String trash = input_additive_buffer;
					int count = trash.length() - trash.replace(" ", "").length();

					if(input_additive_buffer.contains("\n") | ( count == 8 & input_additive_buffer.contains("=F"))) {
				    	try {
				    		TimeUnit.MILLISECONDS.sleep(500);
				    	}
				    	catch (InterruptedException e) {
				    		Log.e(TAG, "sleep interrupted");
				    	}
						processMessage(input_additive_buffer);
						input_additive_buffer = "";
					}
					else if( input_additive_buffer.contains("\n") | (count == 6 & input_additive_buffer.contains("=S"))) {
				    	try {
				    		TimeUnit.MILLISECONDS.sleep(500);
				    	}
				    	catch (InterruptedException e) {
				    		Log.e(TAG, "sleep interrupted");
				    	}
						processMessage(input_additive_buffer);
						input_additive_buffer = "";
					}
					else if( count > 6 & input_additive_buffer.contains("=S")) {
						input_additive_buffer = "";
						secondary_additive_buffer = "";
					}
					else if( count > 8 & input_additive_buffer.contains("=F") ) {
						input_additive_buffer = "";
						secondary_additive_buffer = "";
					}
				}
				break;

			case Constants.MESSAGE_WRITE:
				mDataMLDP.setValue(msg.getData().getString("data") + '\n');
				bService.writeCharacteristic(mDataMLDP);
				break;

			case Constants.MESSAGE_TOAST:
				(Toast.makeText(context,
						"Msg: " + msg.getData().getString(Constants.TOAST),
						Toast.LENGTH_SHORT)).show();
				break;

			default:
				Log.e(TAG, "Main Handler: Unable to understand message of type " + msg.what );
				break;

			} // end switch
		};
	};

	public void compile_messages(String input) {
		if (!input.contains("\n")) {
			input_additive_buffer += input;
		} else {
			String[] parts = input.split("\n");
			if( parts.length != 0) {
				input_additive_buffer += parts[0];
			}
			if (parts.length > 1) {
				secondary_additive_buffer = parts[1];
			}
			complete_message = true;
		}
	}

	public void processMessage(String input) {

		String[] parts = input.split(" ");

		if (parts[0].equals("=F") & parts.length == 8 ) {
			if (Units == METRIC_UNITS) {
				Message tmpmsg = currentHandler
						.obtainMessage(Constants.MESSAGE_FLIGHT_DATA);
				Bundle bundle = new Bundle();
				bundle.putString(Constants.MAXALT, parts[1].substring(0, parts[1].indexOf(".") + 3));
				bundle.putString(Constants.MAXACC, parts[2].substring(0, parts[2].indexOf(".") + 3));
				bundle.putString(Constants.DESCENT, parts[3].substring(0, parts[3].indexOf(".") + 3));
				bundle.putString(Constants.DURATION, parts[4].substring(0, parts[4].indexOf(".") + 3));
				bundle.putString(Constants.MAXSPD, parts[5].substring(0, parts[5].indexOf(".") + 3));
				bundle.putString(Constants.CURSPD, parts[6].substring(0, parts[6].indexOf(".") + 3));
				bundle.putString(Constants.CURALT, parts[7].substring(0, parts[7].indexOf(".") + 3));
				bundle.putInt("units", Units);
				tmpmsg.setData(bundle);

				currentHandler.sendMessage(tmpmsg);
			} else {
				DecimalFormat df = new DecimalFormat("#0.00");
				Message tmpmsg = currentHandler
						.obtainMessage(Constants.MESSAGE_FLIGHT_DATA);
				Bundle bundle = new Bundle();
				// Convert Meters to Feet
				bundle.putString(Constants.MAXALT,
						String.valueOf(df.format(Float.parseFloat(parts[1]) / .3048)));
				// Already in G's
				bundle.putString(Constants.MAXACC, parts[2].substring(0, parts[2].indexOf(".") + 3));
				// Already in G's
//				bundle.putString(Constants.AVGACC, parts[3].substring(0, parts[3].indexOf(".") + 3));
				// Convert m/s to fps
				bundle.putString(Constants.DESCENT, String.valueOf(df.format(Float
						.parseFloat(parts[3]) * 3.28083989501312)));
				// Seconds are metric and imperial
				bundle.putString(Constants.DURATION, parts[4].substring(0, parts[4].indexOf(".") + 3));
				// Convert m/s to fps
				bundle.putString(Constants.MAXSPD, String.valueOf(df.format(Float
						.parseFloat(parts[5]) * 3.28083989501312)));
				bundle.putString(Constants.CURSPD, String.valueOf(df.format(Float
						.parseFloat(parts[6]) * 3.28083989501312)));
				bundle.putString(Constants.CURALT, 
						String.valueOf(df.format(Float.parseFloat(parts[7]) / .3048)));
				
				tmpmsg.setData(bundle);

				currentHandler.sendMessage(tmpmsg);
			}
		} else if (parts[0] == "=P") {
			// TODO fill out
		} else if (parts[0] == "=S" & parts.length == 6) {
			Message tmp = currentHandler.obtainMessage(Constants.MESSAGE_STATUS_DATA);
			Bundle bnd = new Bundle();
			if(parts[1] == "1") {
				bnd.putChar(Constants.GPS_CONNECTED, 'y');
			}
			else {
				bnd.putChar(Constants.GPS_CONNECTED, 'n');
			}
			if(parts[2] == "1") {
				bnd.putChar(Constants.IMU_CONNECTED, 'y');
			}
			else {
				bnd.putChar(Constants.IMU_CONNECTED, 'n');
			}			
			if(parts[3] == "1") {
				bnd.putChar(Constants.HIGHG_CONNECTED, 'y');
			}
			else {
				bnd.putChar(Constants.HIGHG_CONNECTED, 'n');
			}			
			if(parts[4] == "1") {
				bnd.putChar(Constants.FIRING_BOARD_CONNECTED, 'y');
			}
			else {
				bnd.putChar(Constants.FIRING_BOARD_CONNECTED, 'n');
			}			
			if(parts[5] == "1") {
				bnd.putChar(Constants.BARO_CONNECTED, 'y');
			}
			else {
				bnd.putChar(Constants.BARO_CONNECTED, 'n');
			}
			tmp.setData(bnd);
			currentHandler.sendMessage(tmp);
		}
	}

	public void sendMessage(String output) {
		if (output.length() > 0) {
			byte[] send = output.getBytes();
			mDataMLDP.setValue("=>C" + send + "\r\n");
			bService.writeCharacteristic(mDataMLDP);

			outputBuffer.setLength(0);
		} else {
			Message tmpmsg = handler.obtainMessage(Constants.MESSAGE_TOAST);
			Bundle bundle = new Bundle();
			bundle.putString(Constants.TOAST,
					"Unable to send message of length 0.");
			tmpmsg.setData(bundle);
			handler.sendMessage(tmpmsg);
		}
	}

	public void end_connection() {
		bSocket = null;
		paired_device = null;
		paired_device_name = null;
		Message tmpmsg = currentHandler
				.obtainMessage(Constants.MESSAGE_DEVICE_NAME);
		Bundle bundle = new Bundle();
		bundle.putString(Constants.DEVICE_NAME, "");
		tmpmsg.setData(bundle);
		currentHandler.sendMessage(tmpmsg);
		bService.disconnect();
	}

	public void disconnect_bluetooth(View view) {
		(Toast.makeText(this, "Device disconnected", Toast.LENGTH_SHORT))
				.show();
		
		end_connection();
	}

	// ----------------------------------------------------------------------------------------------------------------
	// Scan for BLE device for SCAN_PERIOD milliseconds.
	// The mLeScanCallback method is called each time a device is found during
	// the scan
	private void scanLeDevice() {
		// Stops scanning after a pre-defined scan period.
		mHandler.postDelayed(new Runnable() { // Create delayed runnable that will stop the scan when it runs after SCAN_PERIOD milliseconds
					@Override
					public void run() {
						bAdapter.stopLeScan(mLeScanCallback); // Stop scanning - callback method indicates which scan to stop
						progress.dismiss();
						handler.sendEmptyMessage(Constants.MESSAGE_DISCOVER_DEVICE);
					}
				}, SCAN_PERIOD);

		final Context context = getActivity();
		
		if (bFoundDevices.size() > 0) {
			bFoundDevices.clear();
		}

		progress = new ProgressDialog(context);
		progress.setTitle("Scanning for Bluetooth Devices");
		progress.setMessage("Please wait while scanning.  This will take about 4 seconds.");
		progress.setCanceledOnTouchOutside(false);
		progress.show();

		new Thread(new Runnable() {
			public void run() {
				Looper.prepare();
				bAdapter.startLeScan(mLeScanCallback);
			}
		}).start();
	}

	// ----------------------------------------------------------------------------------------------------------------
	// Device scan callback. Bluetooth adapter calls this method when a new
	// device is discovered during a scan.
	private final BluetoothAdapter.LeScanCallback mLeScanCallback = new BluetoothAdapter.LeScanCallback() {

		@Override
		public void onLeScan(final BluetoothDevice device, int rssi, byte[] scanRecord) { // Android calls method with Bluetooth device advertising information
			new Thread(new Runnable() { // Create runnable that will add the
											// device to the list adapter
				@Override
				public void run() {
					if(!bFoundDevices.contains(device)) {
						bFoundDevices.add(device); // Add the device to the list adapter that will show all the available devices
						Log.d(TAG, "Found BLE Name: " + device.getName()); //more debug info
						Log.d(TAG, "Found BLE Device: " + device.getAddress()); // Debug information to log the devices as they are found
					}
				}
			}).start();
		}
	};
	
    // ----------------------------------------------------------------------------------------------------------------
    // BroadcastReceiver handles various events fired by the BluetoothLeService service.
    private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();                                   //Get the action that was broadcast by the intent that was received

            if (BluetoothLeService.ACTION_GATT_CONNECTED.equals(action)) {              //Service has connected to BLE device
                mConnected = true;                                                      //Record the new connection state
            } 
            
            else if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {		//Service has disconnected from BLE device
                mConnected = false;                                                    //Record the new connection state
                paired_device_name = null;
				Message tmp = handler.obtainMessage(Constants.MESSAGE_TOAST);
				Bundle bnd = new Bundle();
				bnd.putString(Constants.TOAST, "Disconnected from BLE Device");
				tmp.setData(bnd);
				handler.sendMessage(tmp);                              //Update the display to say "Connected" 
            } 
            
            else if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) { //Service has discovered GATT services on BLE device
                findMldpGattService(bService.getSupportedGattServices()); 	//Show all the supported services and characteristics on the user interface
            } 
            
            else if (BluetoothLeService.ACTION_DATA_AVAILABLE.equals(action)) {         //Service has found new data available on BLE device
            	String dataValue = intent.getStringExtra(BluetoothLeService.EXTRA_DATA); //Get the value of the characteristic
                Message msg = handler.obtainMessage(Constants.MESSAGE_READ);
                Bundle bundle = new Bundle();
                bundle.putString("data", dataValue);
                msg.setData(bundle);
                handler.sendMessage(msg);			//Process the data that was received
            }
            
            //For information only. This application sends small packets infrequently and does not need to know what the previous write completed
            else if (BluetoothLeService.ACTION_DATA_WRITTEN.equals(action)) {			//Service has found new data available on BLE device
//            	Log.i(TAG, "Data Written");
            }
        }
    };
    
    private void findMldpGattService(List<BluetoothGattService> gattServices) {
        if (gattServices == null) {                                                     //Verify that list of GATT services is valid
            Log.d(TAG, "findMldpGattService found no Services");
            return;
        }
        String uuid;                                                                    //String to compare received UUID with desired known UUIDs
        mDataMLDP = null;                                                               //Searching for a characteristic, start with null value

        for (BluetoothGattService gattService : gattServices) {                         //Test each service in the list of services
            uuid = gattService.getUuid().toString();                                    //Get the string version of the service's UUID
            if (uuid.equals(MLDP_PRIVATE_SERVICE)) {                                    //See if it matches the UUID of the MLDP service
                List<BluetoothGattCharacteristic> gattCharacteristics = gattService.getCharacteristics(); //If so then get the service's list of characteristics
                for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) { //Test each characteristic in the list of characteristics
                    uuid = gattCharacteristic.getUuid().toString();                     //Get the string version of the characteristic's UUID
                    if (uuid.equals(MLDP_DATA_PRIVATE_CHAR)) {                          //See if it matches the UUID of the MLDP data characteristic
                        mDataMLDP = gattCharacteristic;                                 //If so then save the reference to the characteristic 
                        Log.d(TAG, "Found MLDP data characteristics");
                    } 
                    else if (uuid.equals(MLDP_CONTROL_PRIVATE_CHAR)) {                  //See if UUID matches the UUID of the MLDP control characteristic
                        mControlMLDP = gattCharacteristic;                              //If so then save the reference to the characteristic 
                        Log.d(TAG, "Found MLDP control characteristics");
                    }
                    final int characteristicProperties = gattCharacteristic.getProperties(); //Get the properties of the characteristic
                    if ((characteristicProperties & (BluetoothGattCharacteristic.PROPERTY_NOTIFY)) > 0) { //See if the characteristic has the Notify property
                        bService.setCharacteristicNotification(gattCharacteristic, true); //If so then enable notification in the BluetoothGatt
                    }
                    if ((characteristicProperties & (BluetoothGattCharacteristic.PROPERTY_INDICATE)) > 0) { //See if the characteristic has the Indicate property
                        bService.setCharacteristicIndication(gattCharacteristic, true); //If so then enable notification (and indication) in the BluetoothGatt
                    }
                    if ((characteristicProperties & (BluetoothGattCharacteristic.PROPERTY_WRITE)) > 0) { //See if the characteristic has the Write (acknowledged) property
                        gattCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT); //If so then set the write type (write with acknowledge) in the BluetoothGatt
                    }
                    if ((characteristicProperties & (BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)) > 0) { //See if the characteristic has the Write (unacknowledged) property
                        gattCharacteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE); //If so then set the write type (write with no acknowledge) in the BluetoothGatt
                    }
                }
                break;                  
            }
        }
        if (mDataMLDP == null) {                                                        //See if the MLDP data characteristic was not found
            Toast.makeText(this, R.string.mldp_not_supported, Toast.LENGTH_SHORT).show(); //If so then show an error message
            Log.d(TAG, "findMldpGattService found no MLDP service");
            finish();                                                                   //and end the activity
        }
    }
    
    public void refresh_most_recent_flight(View view) {
    	try {
    		TimeUnit.MILLISECONDS.sleep(1000);
    	}
    	catch (InterruptedException e) {
    		Log.e(TAG, "sleep interrupted");
    	}
    	Message writeMsg = handler.obtainMessage(Constants.MESSAGE_WRITE);
    	Bundle writeBnd = new Bundle();
		if(paired_device != null ) {
			writeBnd.putString("data", "fld");
			writeMsg.setData(writeBnd);
			handler.sendMessage(writeMsg);
			
			handler.sendEmptyMessage(Constants.START_MESSAGE_READ);
		}
    }
    public void set_imperial_units_refresh(View view) {
    	try {
    		TimeUnit.MILLISECONDS.sleep(500);
    	}
    	catch (InterruptedException e) {
    		Log.e(TAG, "sleep interrupted");
    	}

    	Units = IMPERIAL_UNITS;
    	Message writeMsg = handler.obtainMessage(Constants.MESSAGE_WRITE);
    	Bundle writeBnd = new Bundle();
		if(paired_device != null ) {
			writeBnd.putString("data", "fld");
			writeMsg.setData(writeBnd);
			handler.sendMessage(writeMsg);
			
			handler.sendEmptyMessage(Constants.START_MESSAGE_READ);
		}
    }
    
    public void set_metric_units_refresh(View view) {
    	try {
    		TimeUnit.MILLISECONDS.sleep(500);
    	}
    	catch (InterruptedException e) {
    		Log.e(TAG, "sleep interrupted");
    	}    	
    	
    	Units = METRIC_UNITS;
    	Message writeMsg = handler.obtainMessage(Constants.MESSAGE_WRITE);
    	Bundle writeBnd = new Bundle();
		if(paired_device != null ) {
			writeBnd.putString("data", "fld");
			writeMsg.setData(writeBnd);
			handler.sendMessage(writeMsg);
			
			handler.sendEmptyMessage(Constants.START_MESSAGE_READ);
		}
    }

}
