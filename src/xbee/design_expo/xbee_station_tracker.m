function xbee_station_tracker(client, ~)
% Parses acceleration data and visualizes orentation

% Parameters
global p_count accel
X = 1;
Y = 2;
Z = 3;

% Check for legal packet
start_char = fread(client, 1, 'char');
if start_char ~= 'S'
    disp('Invalid start character')
    return
end

% Read in values
accel(p_count, X) = fread(client, 1, 'float32');
accel(p_count, Y) = fread(client, 1, 'float32');
accel(p_count, Z) = fread(client, 1, 'float32');

% Display values
my_line = line([0 accel(p_count,X)], [0 accel(p_count,Y)], [0 accel(p_count,Z)]);
set(my_line, 'Color', 'blue')
set(my_line, 'LineWidth', 5)
plot3(accel(p_count,X), accel(p_count,Y), accel(p_count,Z), '.b', 'MarkerSize', 20)

% Increment counter
p_count = p_count + 1;

end
