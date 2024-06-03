%% Control Digital en Sistemas Embebidos - MSE - PRACTICA 4

pkg load signal
pkg load control

clc
clear all

%%
% 1) Discretizar la planta
%%

h = 0.1;

s = tf('s');
Hs = (0.2*s+1) / (0.1*s*s + 0.4*s +1 );
Hz = c2d(Hs, h, 'zoh')

[numz, denz] = tfdata(Hz, 'v')
[Phi, Gamma, C, D] = tf2ss(numz, denz)

%%
% 2) Obtener la respuesta al escalón en tiempo continuo y discreto.
%%

figure;
step(Hs, Hz)

pole(Hz)

%%
% 3) Aplicar un control diseñado por el método de Pole Placement. 
% Obtener K0 y de forma que el sistema ubique ambos polos en (0,5 +- j 0.3).
%%

% Primero probamos sin obtener K0

poles = [0.6 + 0.1i; 0.6 - 0.1i]

K = place(Phi, Gamma, poles)

Phi_LC = Phi - Gamma * K

%%
% 4) Comparar la respuesta al escalón en lazo abierto con la respuesta obtenida 
%    aplicando el control por Pole Placement anterior.
%%

t = h:h:40;
r = [ zeros(length(t)/2, 1); ones(length(t)/2, 1) ];

y_open = filter(numz, denz, r);
[numz_pole_placement_1, denz_pole_placement_1] = ss2tf(Phi_LC, Gamma, C, D);
y_pole_placement = filter(numz_pole_placement_1, denz_pole_placement_1, r);

figure;
hold on;
stairs(t, r)
stairs(t, y_open, "LineWidth", 3)
stairs(t, y_pole_placement, "LineWidth", 3)
legend("Entrada", "Respuesta natural", "Salida controlada")

%%
% Ahora probamos obteneniendo K0
%%

K = place(Phi, Gamma, poles)

Phi_LC = Phi - Gamma * K

K0 = (C * (eye(2) - Phi_LC)^(-1) * Gamma)^(-1)

Gamma_LC = Gamma * K0

%%
% 4) Comparar la respuesta al escalón en lazo abierto con la respuesta obtenida 
%    aplicando el control por Pole Placement anterior.
%%

t = h:h:40;
r = [ zeros(length(t)/2, 1); ones(length(t)/2, 1) ];

y_open = filter(numz, denz, r);
[numz_pole_placement, denz_pole_placement] = ss2tf(Phi_LC, Gamma_LC, C, D);
y_pole_placement = filter(numz_pole_placement, denz_pole_placement, r);

figure;
hold on;
stairs(t, r)
stairs(t, y_open, "LineWidth", 3)
stairs(t, y_pole_placement, "LineWidth", 3)
legend("Entrada", "Respuesta natural", "Salida controlada")


%%
% Funcion Pole Placement 
% Prueba para planta de primer orden en donde y(k) = x(k)
%%

function [data] = pole_placement_controller(data)
  data.output = data.K0 * data.R - data.K * data.X;
end

function [y] = filter_pole_placement(b, a, r, data)
  y_inicial = length(a) - 1;
  y = zeros(y_inicial + length(r), 1);
  r = [ zeros(y_inicial, 1); r ];
  u = [ zeros(y_inicial, 1); r ];
  
  for k = y_inicial + 1 : 1 : length(r)
    data.X = y(k-1);
    data.R = r(k);
    data = pole_placement_controller(data);
    u(k) = data.output;
    
    for m = 1 : 1 : length(b)
      y(k) = y(k) + b(m) * u(k-m+1);
    end
    
    for n = 2 : 1 : length(a)
      y(k) = y(k) - a(n) * y(k-n+1);
    end
  
    y(k) = y(k) / a(1);
  end
  
  y = y(y_inicial+1:end);
end




