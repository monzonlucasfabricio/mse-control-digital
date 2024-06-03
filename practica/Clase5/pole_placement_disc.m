% Define the system transfer function
R1 = 2e3;
C1 = 1e-6;
R2 = 1e3;
C2 = 1e-6;

% 
% num = [1/(R1*C1*R2*C2)];
% den = [1,(R1*C1+R1*C2+R2*C2)/(R1*C1*R2*C2), 1/(R1*C1*R2*C2)];
% sys_tf = tf(num, den);
% sys = ss(sys_tf);

A = [-(1/R1+1/R2)/C1 1/(R2*C1);1/(R2*C2) -1/(R2*C2)];
B = [1/(R1*C1); 0];
C = [0 1];

sys = ss(A,B,C,0);

h = 1e-3;
t = 0:h:0.1;


sysd = c2d(sys,h);

x = zeros(length(sysd.A),length(t));
y = zeros(1,length(t));
u = ones(1,length(t));

% x(:,1) = [1,2]

for i=1:length(t)
    x(:,i+1) = sysd.A*x(:,i) + sysd.B*u(i);
    y(i) = sysd.C*x(:,i);
end

close all
plot(t,y,t,u)
% close all
% plot(t,x(1,:),t,x(2,:))

% u = -K*x
%% Lazo cerrado
% Calculo los polos del sistema en tiempo continuo
P = pole(sys_tf);

Pd = exp(P*1.3*h);  % Hay que tener en cuenta que están en tiempo discreto

K = place(sysd.A,sysd.B,[Pd(1),Pd(2)]);

% u = r-Kx 

x_lc = zeros(length(sysd.A),length(t));
y_lc = zeros(1,length(t));
r    = ones(1,length(t));

% Simulo el sistema incluyendo la realimentación
for i=1:length(t)
    x_lc(:,i+1) = sysd.A*x_lc(:,i) - sysd.B*K*x_lc(:,i) + sysd.B*r(i);
    y_lc(i) = sysd.C*x_lc(:,i);
end

close all
plot(t,y,t,y_lc,'LineWidth',1)

%% Con ganancia

sysdlc = ss(sysd.A-sysd.B*K,sysd.B,sysd.C,sysd.D);

% g = dcgain(sysdlc); % Ganancia de estado estacionario
% C*(I – A)^(–1)*B
g = sysdlc.C*(eye(2)-sysdlc.A)^(-1)*sysdlc.B;  % GEE, para sist. discreto
K0 = 1/g; 

x_lcg = zeros(length(sysd.A),length(t));
y_lcg = zeros(1,length(t));


for i=1:length(t)
    x_lcg(:,i+1) = sysd.A*x_lcg(:,i) - sysd.B*K*x_lcg(:,i) + sysd.B*K0*r(i);
    y_lcg(i) = sysd.C*x_lcg(:,i);
end

close all
plot(t,y,t,y_lcg,'LineWidth',1)

%% Integrador
% La idea es agregar un integrador para eliminar el error de EE

Ae  = [sysd.A zeros(length(sysd.A),1);-sysd.C 1];
Beu = [sysd.B;0];
Ber = [zeros(length(sysd.B),1);1];
Ce  = [sysd.C 0];

pdi = exp(-10*max(abs(P))*h);  % Siempre la velocidad de los polos tiene que verse en tiempo continuo

K1 = place(Ae,Beu,[Pd;pdi]);

x_lci = zeros(length(sysd.A),length(t));
xi = zeros(1,length(t));
y_lci = zeros(1,length(t));
u = zeros(1,length(t));

for i=1:length(t)
    x_lci(:,i+1) = sysd.A*x_lci(:,i) + sysd.B*(r(i) - K1(1:2)*x_lci(:,i) - K1(3)*xi(i));
    u(i)         = r(i) - K1(1:2)*x_lci(:,i) - K1(3)*xi(i);
    xi(i+1)      = xi(i) + r(i)- sysd.C*x_lci(:,i); 
    y_lci(i)     = sysd.C*x_lci(:,i);
end

close all
plot(t,y,t,y_lci)
figure
plot(t,u)

