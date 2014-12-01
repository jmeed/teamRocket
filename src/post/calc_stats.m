function stats = calc_stats(baro, imu)

% Parameters
m2ft = 3.28084;
clock = 48e6;
clock_scale = 5e4;

% Setup output
stats.altitude_m = 0;%
stats.altitude_ft = 0;%
stats.duration = 0;
stats.thrust_time = 0;
% stats.coast2apogee_time = 0;
% stats.apogee2eject_time = 0;
stats.peak_accel = 0;%
stats.avg_accel = 0;%

% Grab vectors
baro_t = baro(:,1);
imu_t = imu(:,1);
alt = baro(:,3);
accel = imu(:,(2:4));
accel = accel_filter(accel);
gyro = imu(:,(5:7));
mag = imu(:,(8:10));

% Calculate altitude
base_alt = min(alt);
top_alt = max(alt);
stats.altitude_m = top_alt - base_alt;
stats.altitude_ft = stats.altitude_m * m2ft;

% Find launch data
[accel_flight accel_thrust i_t_imu_flight i_t_imu_thrust] = find_flight_accel(accel);
t_imu_flight = imu_t(i_t_imu_flight);
t_imu_thrust = imu_t(i_t_imu_thrust);

[baro_flight i_t_baro_flight] = find_flight_alt(baro);
t_baro_flight = baro_t(i_t_baro_flight);

% Get peak and average accel
max_accel = max(accel_thrust(:,1));
i_max = find(accel_thrust(:,1) == max_accel);
max_accel = accel_thrust(i_max,:);
max_accel = max_accel(1,:);
stats.peak_accel = sqrt(sum(max_accel.^2)) - 1;
avg_accel = mean(accel_thrust);
stats.avg_accel = sqrt(sum(avg_accel.^2)) - 1;

% Get averaged total flight time
% Baro
baro_flight_ticks = t_baro_flight(end) - t_baro_flight(1);
baro_duration = baro_flight_ticks / clock * clock_scale;
% Accel
imu_flight_ticks = t_imu_flight(end) - t_imu_flight(1);
accel_duration = imu_flight_ticks / clock * clock_scale;
% Total
stats.duration = (accel_duration + baro_duration) / 2;
% Thrust
imu_thrust_ticks = t_imu_thrust(end) - t_imu_thrust(1);
stats.thrust_time = imu_thrust_ticks / clock * clock_scale;

end
