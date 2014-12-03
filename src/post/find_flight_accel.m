function [accel_flight accel_thrust i_t_flight i_t_thrust] = find_flight_accel(accel)

% Parameters
launch_threshold = 2.5;
thrust_threshold = 1.3;

% Find launch via threshold
above_thresh = find(accel(:,1) > launch_threshold);
above_thresh = above_thresh(1);
accel_flight = accel((above_thresh:end),:);
i_t_flight_all = (1:size(accel,1))';
i_t_flight = i_t_flight_all((above_thresh:end),1);

% Find end of thrust via threshold
below_thresh = find(accel_flight(:,1) < thrust_threshold);
below_thresh = below_thresh(1);
accel_thrust = accel_flight((1:below_thresh-1),:);
i_t_thrust = i_t_flight_all((above_thresh:above_thresh+below_thresh-1),1);

end
