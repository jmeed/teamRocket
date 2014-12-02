function xbee_setup_tracker(com_port, baud_rate)
% Sets up a callback function that will track the rockets
% orientation based on the streaming acceleration data

% Paramters
global p_count accel
p_count = 0;
accel = [0 0 0];

% Initialize serial port
x_port = serial(com_port);

% Configure serial port
set(x_port, 'BaudRate', baud_rate);
set(x_port, 'DataBits', 8);
set(x_port, 'StopBits', 1);
set(x_port, 'Parity', 'none');
set(x_port, 'ByteOrder', 'littleEndian');

% Open serial port
fopen(x_port);
disp('Serial port open')

% Setup callback
x_port.BytesAvailableFcnCount = 4*3 + 1*1; % 3 floats, 1 char
x_port.BytesAvailableFcnMode = 'byte';
x_port.BytesAvailableFcn = @xbee_station_tracker;

% Setup visualization
figure;
grid on
hold off
axis equal
axis([-3 3 -3 3 -3 3])
title('Modular Flight Recorder Orientation Tracker')
xlabel('x')
ylabel('y')
zlabel('z')

end
