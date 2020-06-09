Data_rad=Data_cangle.*pi./180;
ang=buf(:,2)./3.45;
theta = Data_rad(35:60);
speed_l=ang(35:60);
w = omega_smooth2(35:60).*pi./180;

for i=1:(length(w)-1)
    W(i) = (w(i+1) - w(i))/0.01;
end
for i=1:(length(speed_l)-1)
    acceleration(i)=(speed_l(i+1)-speed_l(i))/0.01; 
end

W(length(w)) = W(length(w)-1);
acceleration(length(speed_l))=acceleration(length(speed_l)-1);
A = zeros(length(w),5);

A(:,1) = -W';
A(:,2) = -2*cos(theta).*W'+sin(2*theta).*sec(theta).*w';
A(:,3) = w';
A(:,4) = -speed_l;
A(:,5) = 90;

b = transpose(acceleration);

x = inv(transpose(A)*A)*transpose(A)*b;
sigma1 = x(1)
sigma2 = x(2)
sigma3 = x(3)    %-10.932019402580760
sigma4 = x(4)    %-39.402306014934040
sigma5 = x(5)