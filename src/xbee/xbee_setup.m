function x_port = xbee_setup(com_port, data_bits, stop_bits, baud_rate, parity)

x_port = serial(com_port);
set(x_port, 'DataBits', data_bits);
set(x_port, 'StopBits', stop_bits);
set(x_port, 'BaudRate', baud_rate);
set(x_port, 'Parity', parity);
fopen(x_port);

% Send acknowledgement
fprintf(x_port, '%c', 'a');
disp('Sent acknowledgement')

% Wait for acknowledgement
a = '';
while (a ~= 'a')
    a = fread(x_port, 1, 'uchar');
end
disp('Received acknowledgement')
disp('XBee communication ready')

end
