Data_rad=Data_cangle.*pi./180;
theta = Data_rad(30:60);
w = omega_smooth2(30:60).*pi./180;

for i=1:(length(w)-1)
    W(i) = (w(i+1) - w(i))/0.01;
end
W(length(w)) = W(length(w)-1);

A = zeros(length(w),3);
A(:,1) = W';
A(:,2) = 2*cos(theta).*W'-sin(2*theta).*sec(theta).*w';
A(:,3) = -w';

b = 0;

[U S V]=svd(A);
x=V(:,end);

sigma1 = x(1)    %-10.932019402580760
sigma2 = x(2)    %-39.402306014934040
sigma3 = x(3)