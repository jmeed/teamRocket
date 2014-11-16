% xbee.m
% Must define PORT and BAUD

% Initialize storage and globals
global p_count altitude temp accel_x gps_x gps_y gps_z
p_count = 1;
altitude = [];
temp = [];
accel_x = [];
gps_x = [];
gps_y = [];
gps_z = [];

% Setup connection
xbee_setup(PORT, BAUD);
