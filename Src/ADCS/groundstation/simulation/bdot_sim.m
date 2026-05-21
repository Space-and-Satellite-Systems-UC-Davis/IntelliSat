matlab_to_c = fopen('matlab_to_c', 'w+')
c_to_matlab = fopen('c_to_matlab', 'r+')

duration = 660; % Duration of simulation, seconds
dt = .03; % time step of simulation, seconds
steps = 1:ceil(duration/dt);

% Moment of inertia matrix, body frame.
% gram*mm^2

MOI = [7936448.62    -4767.01    -21257.03 ;
    -4767.01    8201944.04    -398.56 ;
    -21257.03    -398.56    3302998.27 ];

% Convert to kg*m^2
MOI = 10^(-9)*MOI;

% angular velocity, rad/s, inertial frame.
w = 2*pi*[1 0 0]';

% Quaternion storing attitude, scalar last. Should always be normalized.
% Maps from bod -> inertial
q = [0 0 0 1]';
% Maps from inertial -> bod
q_inv = [-q(1:3) ; q(4)];

% Vector storing the current magnetic field reading in microteslas in the inertial frame of reference. TODO: make this change over time realistically.
mag_inertial = [5000 5000 5000]';

% Vectors storing the current and previous magnetic field reading 
mag_bod = quat_apply(q_inv, mag_inertial); 
mag_bod_prev = mag_bod;

data_out = zeros(length(steps), 12);

for t = steps 

	R_earth = 6371;

	% dq = [0 0 0];
	q_dot = 0.5 * quat_mult([w ; 0], q); % Could also do it this way
	% q = quat_mult(dq, q);
	q = q + dt*q_dot;
	q = q/norm(q);

	q_inv = [-q(1:3) ; q(4)];

	% Wrong coords? No I think they're right but you should double check.
	w_bod = quat_apply(q_inv, w);
	L_prev = (MOI*w_bod);

	mag_bod_prev = mag_bod; 
	mag_bod = quat_apply(q_inv, mag_inertial); 

	% c_bdot_control(mag_bod, mag_bod_prev, dt, matlab_to_c, c_to_matlab);

	torque_bod = [0 0 0]';
	
	w_bod_dot = MOI\((torque_bod - cross(w_bod, MOI*w_bod)));
	w_bod = w_bod + dt*w_bod_dot;
	w = quat_apply(q, w_bod);

	L_new = (MOI*w_bod);


	% Normalize angular velocity to conserve angular momentum
	w = w*norm(L_prev)/norm(L_new);

	x = quat_apply(q, [1, 0, 0]');
	y = quat_apply(q, [0, 1, 0]');
	z = quat_apply(q, [0, 0, 1]');

	%MOI_inertial = [quat_apply(q, MOI(:, 1)), quat_apply(q, MOI(:, 2)), quat_apply(q, MOI(:, 3))];
	L_new_bod = quat_apply(q, L_new);
	%L_inertial = MOI_inertial*w; 

	data_out(t, :) = [x', y', z', L_new_bod'];
end

plot3(data_out(:, 1), data_out(:, 2), data_out(:, 3))
hold on
plot3(data_out(:, 4), data_out(:, 5), data_out(:, 6))
plot3(data_out(:, 7), data_out(:, 8), data_out(:, 9))
plot3(data_out(:, 10), data_out(:, 11), data_out(:, 12))
ax = gca

cla(ax)

ax.XLimMode = 'manual';
ax.YLimMode = 'manual';
ax.ZLimMode = 'manual';
ax.XLim = [-1, 1];
ax.YLim = [-1, 1];
ax.ZLim = [-1, 1]

hold on 

daspect([1 1 1])
xplot = line(ax, [0, data_out(1, 1)], [0, data_out(1, 2)], [0, data_out(1, 3)], 'color', 'r')
yplot = line(ax, [0, data_out(1, 4)], [0, data_out(1, 5)], [0, data_out(1, 6)], 'color', 'g')
zplot = line(ax, [0, data_out(1, 7)], [0, data_out(1, 8)], [0, data_out(1, 9)], 'color', 'b')
lplot = line(ax, [0, data_out(1, 10)], [0, data_out(1, 11)], [0, data_out(1, 12)], 'color', 'y')

for t = steps
	xplot.XData = [0, data_out(t, 1)];
 	xplot.YData = [0, data_out(t, 2)];
	xplot.ZData = [0, data_out(t, 3)];

	yplot.XData = [0, data_out(t, 4)];
 	yplot.YData = [0, data_out(t, 5)];
	yplot.ZData = [0, data_out(t, 6)];

	zplot.XData = [0, data_out(t, 7)];
 	zplot.YData = [0, data_out(t, 8)];
	zplot.ZData = [0, data_out(t, 9)];

	lplot.XData = [0, data_out(t, 10)];
 	lplot.YData = [0, data_out(t, 11)];
	lplot.ZData = [0, data_out(t, 12)];

	drawnow
	t*dt
end


% ---------------------------------------------------------------------------------------
% 										FUNCTIONS
% ---------------------------------------------------------------------------------------

function q_out = quat_mult(q1, q2)
	r1 = q1(4); % scalar parts of 
	r2 = q2(4); % q1 and q2
	v1 = q1(1:3); % vector parts of
	v2 = q2(1:3); % v1 and v2

	q_out = [(r1*v2 + r2*v1 + cross(v1, v2)) ; r1*r2 - dot(v1, v2)];
end

% Rotate vector v by the quaternion q
function v_out = quat_apply(q, v)
	v = [v ; 0];

	q_inv = [-q(1:3) ; q(4)];
	v_out = quat_mult(quat_mult(q, v), q_inv);
	v_out(4) = [];
end

function v = vec_c_to_mlab(vec_struct)
	v = [vec_struct.x, vec_struct.y, vec_struct.z]';
end

function coils_cmd = c_bdot_control(mag_curr, mag_prev, delta_t, matlab_to_c, c_to_matlab)
	fprintf(matlab_to_c, "%d %f %f %f %f %f %f %f", ...
			mag_curr(1), mag_curr(2), mag_curr(3), ...
			mag_prev(1), mag_prev(1), mag_prev(1), ...
			delta_t);

	disp("reading")
	line = fgets(c_to_matlab);
	coils_cmd = sscanf(line, "%f %f %f")
end
