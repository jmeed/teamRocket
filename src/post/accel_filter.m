function [acc_f] = accel_filter(acc_in)

n = size(acc_in,1);
acc_f = zeros(n,3);

for i = 1:n
    acc_buf = update_buffer(acc_in, i);
    acc_f(i,:) = decide_value(acc_buf);
end

end

function acc_f = decide_value(acc_buf)

X = 1;
Y = 2;
Z = 3;

acc_f = zeros(1,3);

acc_thresh = 0.1;
n_nonzero = numel(find(acc_buf(:,1) ~= 0));

% If its above the threshold, we know it's good
if abs(acc_buf(1,X) > acc_thresh)
    acc_f(1,X) = acc_buf(1,X);
else % was there a recent good value?
    for i = 2:n_nonzero
        if abs(acc_buf(i,X)) > acc_thresh
            acc_f(1,X) = acc_buf(i,X);
            break;
        end
    end
end

if abs(acc_buf(1,Y) > acc_thresh)
    acc_f(1,Y) = acc_buf(1,Y);
else
    for i = 2:n_nonzero
        if abs(acc_buf(i,Y)) > acc_thresh
            acc_f(1,Y) = acc_buf(i,Y);
            break;
        end
    end
end

if abs(acc_buf(1,Z)) > acc_thresh
    acc_f(1,Z) = acc_buf(1,Z);
else
    for i = 2:n_nonzero
        if abs(acc_buf(i,Z)) > acc_thresh
            acc_f(1,Z) = acc_buf(i,Z);
            break;
        end
    end
end

end

function acc_buf = update_buffer(acc_i, i)

n_buf = 4;
acc_buf = zeros(n_buf,3);

endpt = min([5 i]);

for k = 1:endpt
    acc_buf(k,:) = acc_i(i-k+1,:);
end

end
