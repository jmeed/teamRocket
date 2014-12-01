function xbee_post_visualizer()
% Parses barometer, accelerometer, and GPS data coming from
% the rocket downlink, and attempts real-time visualization
% after all data has been collected

% Link globals
global dt
global accel_x accel_y accel_z 
global rot_x rot_y rot_z 

% Show flight
A_THRESH = -2;
GRAVITY = -1;

n = size(accel_x,1);
pos = [0 0 0];
pyr = [0 0 0];

colors = repmat(['r';'b';'g'],ceil(n/3),1);

figure;
xlabel('x'), ylabel('y'), zlabel('z')
hold on
grid on
axis([-10 10 -10 10 0 100])
pause(5);
detect_launch = false;
for i = 1:n
    this_accel = [accel_x(i); accel_y(i); accel_z(i)];
%     fprintf('this accel (%d) = <%.1f,%.1f,%.1f>\n', i, this_accel(1), this_accel(2), this_accel(3));
    pyr = pyr + [rot_y(i) rot_x(i) rot_z(i)];
%     fprintf('this pyr (%d) = <%.1f,%.1f,%.1f>\n', i, this_pyr(1), this_pyr(2), this_pyr(3));
    Trb = T_rocket2base(pyr(1), pyr(2), pyr(3));
    base_accel = (Trb * this_accel')';
%     fprintf('base accel (%d) = <%.1f,%.1f,%.1f>\n', i, base_accel(1), base_accel(2), base_accel(3));
    
    if (base_accel(3)-GRAVITY > A_THRESH) && (detect_launch == false)
        continue
    end
    detect_launch = true;
    
    newpos = pos + -1*dt.*dt.*base_accel;
    rocket = line([pos(1) newpos(1)],[pos(2) newpos(2)],[pos(3) newpos(3)]);
    set(rocket,'Color',colors(i));
    plot3(newpos(1),newpos(2),newpos(3),['.' colors(i)],'MarkerSize',10);
    pos = newpos;
    pause(dt);
end

end
