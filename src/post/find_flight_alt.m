function [alt_flight i_t_flight] = find_flight_alt(baro)

% Parameters
launch_threshold = 6;
crash_threshold = 10;

% Separate data
alt = baro(:,3);
t = baro(:,1);
temp = baro(:,2);

% Find launch via threshold
baseline = median(alt(10:50));
peak = max(alt);
ipeak = find(alt == peak);
ipeak = ipeak(1);
alt_before_peak = alt(1:ipeak);
above_thresh = find(alt_before_peak < baseline + launch_threshold);
above_thresh = above_thresh(end);
alt_flight = alt(above_thresh:end);
below_thresh = find(alt_flight < baseline + crash_threshold);
if (below_thresh(1) - above_thresh) > 100
    below_thresh = below_thresh(1);
else
    below_thresh = length(alt_flight);
end
alt_flight = alt_flight(1:below_thresh);
i_t_flight = (1:length(t))';
i_t_flight = i_t_flight(above_thresh:above_thresh+below_thresh-1);

end
