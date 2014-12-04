function x_port = xbee_setup_tracker(com_port, baud_rate)
% Sets up a callback function that will track the rockets
% orientation based on the streaming acceleration data

delete(instrfindall)

% Paramters
global p_count accel pt ln
p_count = 1;
accel = [0 0 0];

% Initialize serial port
x_port = serial(com_port);

% Configure serial port
set(x_port, 'BaudRate', baud_rate);
set(x_port, 'DataBits', 8);
set(x_port, 'StopBits', 1);
set(x_port, 'Parity', 'none');
set(x_port, 'ByteOrder', 'littleEndian');
set(x_port, 'Timeout', 0);

% Setup callback
x_port.BytesAvailableFcnCount = 4*3 + 1*9;
x_port.BytesAvailableFcnMode = 'byte';
x_port.BytesAvailableFcn = @xbee_station_tracker;

% Open serial port
fopen(x_port);
disp('Serial port open')

% Setup visualization
figure;
grid on
hold on
axis equal
axis([-3 3 -3 3 -3 3])
title('Modular Flight Recorder Orientation Tracker')
xlabel('x')
ylabel('y')
zlabel('z')


ln = line([0 0], [0 0], [0 0]);
pt = plot3(0, 0, 0, '.b');

end
