function xbee_station_tracker(client, ~)
% Parses acceleration data and visualizes orentation

% Parameters
global p_count accel pt ln
X = 1;
Y = 2;
Z = 3;

% Check for legal packet
while char(fread(client, 1, 'char')) ~= 'S'
end

try
    % Parse packet
    junk1 = char(fread(client, 1, 'char')); % ,
    junk2 = char(fread(client, 1, 'char')); % I
    junk3 = char(fread(client, 1, 'char')); % M
    junk4 = char(fread(client, 1, 'char')); % U
    junk5 = char(fread(client, 1, 'char')); % A
    junk6 = char(fread(client, 1, 'char')); % C
    junk7 = char(fread(client, 1, 'char')); % C
    junk8 = char(fread(client, 1, 'char')); % ,

    % x-axis
    x_float = '';
    i_f = 1;
    sign = char(fread(client, 1, 'char')); % -
    if sign == '-'
        x_float(i_f) = sign;
        i_f = i_f + 1;
        sign = char(fread(client, 1, 'char')); % 0
    end
    x_float(i_f) = sign; % 0
    i_f = i_f + 1;
    x_float(i_f) = char(fread(client, 1, 'char')); % .
    i_f = i_f + 1;
    x_float(i_f) = char(fread(client, 1, 'char')); % 0
    i_f = i_f + 1;
    x_float(i_f) = char(fread(client, 1, 'char')); % 0
    junk9 = char(fread(client, 1, 'char')); % ,

    % y-axis
    y_float = '';
    i_f = 1;
    sign = char(fread(client, 1, 'char')); % -
    if sign == '-'
        y_float(i_f) = sign;
        i_f = i_f + 1;
        sign = char(fread(client, 1, 'char')); % 0
    end
    y_float(i_f) = sign; % 0
    i_f = i_f + 1;
    y_float(i_f) = char(fread(client, 1, 'char')); % .
    i_f = i_f + 1;
    y_float(i_f) = char(fread(client, 1, 'char')); % 0
    i_f = i_f + 1;
    y_float(i_f) = char(fread(client, 1, 'char')); % 0
    junk10 = char(fread(client, 1, 'char')); % ,

    % z-axis
    z_float = '';
    i_f = 1;
    sign = char(fread(client, 1, 'char')); % -
    if sign == '-'
        z_float(i_f) = sign;
        i_f = i_f + 1;
        sign = char(fread(client, 1, 'char')); % 0
    end
    z_float(i_f) = sign; % 0
    i_f = i_f + 1;
    z_float(i_f) = char(fread(client, 1, 'char')); % .
    i_f = i_f + 1;
    z_float(i_f) = char(fread(client, 1, 'char')); % 0
    i_f = i_f + 1;
    z_float(i_f) = char(fread(client, 1, 'char')); % 0
    junk11 = char(fread(client, 1, 'char')); % ,

    x_a = str2num(x_float);
    y_a = str2num(y_float);
    z_a = str2num(z_float);
    fprintf('x: %.2f, y: %.2f, z: %.2f\n', x_a, y_a, z_a);

    % Assign floats
    accel(p_count,X) = x_a;
    accel(p_count,Y) = y_a;
    accel(p_count,Z) = z_a;

    % Display values
    delete(ln); % clear last plot
    delete(pt);
    ln = line([0 accel(p_count,X)], [0 accel(p_count,Y)], [0 accel(p_count,Z)]);
    set(ln, 'Color', 'red')
    set(ln, 'LineWidth', 5)
    pt = plot3(accel(p_count,X), accel(p_count,Y), accel(p_count,Z), '.b', 'MarkerSize', 20);

    % Increment counter
    if (p_count > 500)
        p_count = 1;
    else
        p_count = p_count + 1;
    end

catch
%     disp('some error')
end

end
