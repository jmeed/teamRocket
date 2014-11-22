function xbee_station_visualizer(client, ~)
% Parses barometer, accelerometer, and GPS data coming from
% the rocket downlink, and attempts real-time visualization

% Link globals
global p_count altitude temp launch dt
global accel_x accel_y accel_z 
global rot_x rot_y rot_z 
global gps_x gps_y gps_z
global pitch yaw roll
global position fig

% Check for legal packet
start_char = fread(client, 1, 'char');
if (start_char ~= 'S')
    disp('Invalid start character')
    return
end

% Read in values
altitude(p_count, 1) = fread(client, 1, 'float32');
temp(p_count, 1) = fread(client, 1, 'float32');
accel_x(p_count, 1) = fread(client, 1, 'float32');
accel_y(p_count, 1) = fread(client, 1, 'float32');
accel_z(p_count, 1) = fread(client, 1, 'float32');
rot_x(p_count, 1) = fread(client, 1, 'float32');
rot_y(p_count, 1) = fread(client, 1, 'float32');
rot_z(p_count, 1) = fread(client, 1, 'float32');
gps_x(p_count, 1) = fread(client, 1, 'float32');
gps_y(p_count, 1) = fread(client, 1, 'float32');
gps_z(p_count, 1) = fread(client, 1, 'float32');

% Check for launch
acc_threshold = 3;
accel = [accel_x; accel_y; accel_z];
max_accel = max(abs(accel));
if (max_accel > acc_threshold) && (launch == 0)
    pitch = 0;
    yaw = 0;
    roll = 0;
    launch = 1;
end

% Visualize flight
if launch == 1
    pitch = pitch + rot_y(p_count);
    yaw = yaw + rot_x(p_count);
    roll = roll + rot_z(p_count);
    r_acc = [accel_x(p_count); accel_y(p_count); accel_z(p_count)];
    Trb = T_rocket2base(pitch, yaw, roll);
    b_acc = Trb * r_acc;
    new_position = position + -1.*dt.*dt.*b_acc;
    rocket = line([position(1) new_position(1)],[position(2) new_position(2)],[position(3) new_position(3)]);
    set(rocket,'Color','b');
    plot3(new_position(1),new_position(2),new_position(3),'.ra','MarkerSize',10);
    position = new_position;
end
        
% Increment counter
p_count = p_count + 1;

end
