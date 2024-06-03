%% Observador
clc, clear
% Define the system transfer function
R1 = 2e3;
C1 = 1e-6;
R2 = 1e3;
C2 = 1e-6;

A = [-(1/R1+1/R2)/C1 1/(R2*C1);1/(R2*C2) -1/(R2*C2)];
B = [1/(R1*C1); 0];
C = [0 1];

% Sistema en variables de estado continuo
sys = ss(A,B,C,0);

% Paso de muestreo y tiempo de simulación
h = 1e-3;
t = 0:h:0.1;

% Sistema en variables de estado discretas
sysd = c2d(sys,h);

P = pole(sys);

Po = [-10*max(abs(P));-11*max(abs(P))];

Pod = exp(Po*h);

L = place(sysd.A',sysd.C',Pod)';

n = length(sysd.A);
N = length(t);
% Simulación del sistema
x = zeros(n,N);
x_est = zeros(n,N);
y = zeros(1,N);
y_est = zeros(1,N);
u = ones(1,N);

% x(:,1) = [0.02;0.01];
x_est(:,1) = [0.01;-0.02];

for i=1:length(t)
    % sist 'real'
    x(:,i+1) = sysd.A*x(:,i) + sysd.B*u(i);
    y(i) = sysd.C*x(:,i);
    % syst est
    x_est(:,i+1) = sysd.A*x_est(:,i) + sysd.B*u(i) + L*(y(i)-sysd.C*x_est(:,i));
    y_est(i) = sysd.C*x_est(:,i);
end

close all
plot(t,y,t,y_est,'LineWidth',1)
% figure
% plot(t,x_est(1,2:end),t,x_est(2,2:end))
% hold on
% plot(t,x_est(1,2:end),t,x_est(2,2:end))

%% Realimentación de estados con observador
Pd = exp(P*1.3*h);

K = place(sysd.A,sysd.B,Pd);

x_lc = zeros(n,N);
y_lc = zeros(1,N);
r    = ones(1,N);

sysdlc = ss(sysd.A-sysd.B*K,sysd.B,sysd.C,sysd.D);
g = sysdlc.C*(eye(2)-sysdlc.A)^(-1)*sysdlc.B;  % GEE, para sist. discreto
Ko = 1/g;

for i=1:length(t)
    u(i) = Ko*r(i) - K*x_est(:,i);
    % Sistema 'real'
    x_lc(:,i+1) = sysd.A*x_lc(:,i) + sysd.B*u(i);
    y_lc(i)     = sysd.C*x_lc(:,i);
    % Observador
    x_est(:,i+1) = sysd.A*x_est(:,i) + sysd.B*u(i) + L*(y_lc(i)-sysd.C*x_est(:,i));
    y_est(i)     = sysd.C*x_est(:,i);
end

close all
plot(t,y,t,y_lc,t,y_est,'LineWidth',1)

