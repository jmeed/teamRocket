% xbee_logger.m
% Must define PORT and BAUD

% Initialize storage and globals
global p_count altitude temp launch dt
global accel_x accel_y accel_z 
global rot_x rot_y rot_z 
global gps_x gps_y gps_z
global pitch yaw roll
global position fig
p_count = 1;
altitude = [];
temp = [];
accel_x = [];
accel_y = [];
accel_z = [];
rot_x = [];
rot_y = [];
rot_z = [];
gps_x = [];
gps_y = [];
gps_z = [];
launch = 0;
pitch = 0;
yaw = 0;
roll = 0;
dt = 1/100;
position = [0 0 0];
% fig = figure;
% xlabel('x'), ylabel('y'), zlabel('z')
% hold on
% grid on
% axis([-10 10 -10 10 0 100])

% Setup connection
xbee_setup_logger(PORT, BAUD);
