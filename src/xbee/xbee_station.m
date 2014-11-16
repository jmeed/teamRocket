function xbee_station(client, ~)
% Parses and displays barometer, accelerometer, and GPS data coming from
% the rocket downlink

% Link globals
global p_count altitude temp accel_x gps_x gps_y gps_z

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
gps_x(p_count, 1) = fread(client, 1, 'float32');
gps_y(p_count, 1) = fread(client, 1, 'float32');
gps_z(p_count, 1) = fread(client, 1, 'float32');

% Increment counter
p_count = p_count + 1;

end
