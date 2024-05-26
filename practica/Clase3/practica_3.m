%% Control Digital en Sistemas Embebidos - MSE - PRACTICA 3

pkg load signal
pkg load control

clc
clear all

%%
% 1) Crear una función en Matlab que aplique la función del controlador PID y 
%    obtenga la señal de control para una determinada señal de referencia y 
%    señal de salida.
%%

function [data] = pid_controller(data)
  # Proporcional
  P = data.Kp * (data.b * data.R - data.Y);
  # Integral
  I = data.futureI;
  # Derivativo
  D = (data.Td / (data.Td - data.N * data.h)) * \
    (data.pastD - data.N * data.Kp * (data.Y - data.pastY));
  # PID
  data.output = P + I + D;
  # Integral futura
  data.futureI = I + data.Kp * data.h / data.Ti * (data.R - data.Y);
  data.pastD = D;
  data.pastY = data.Y;
end

function [y] = filter_pid(b, a, r, data)
  y_inicial = length(a) - 1;
  y = zeros(y_inicial + length(r), 1);
  r = [ zeros(y_inicial, 1); r ];
  u = [ zeros(y_inicial, 1); r ];
  
  for k = y_inicial + 1 : 1 : length(r)
    data.Y = y(k-1);
    data.R = r(k);
    data = pid_controller(data);
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

%%
% 2) Discretizar la planta
%%

h = 0.01;

s = tf('s');
Hs = (0.2*s+1) / (0.1*s*s + 0.4*s +1 );
Hz = c2d(Hs, h, 'zoh')

[numz, denz] = tfdata(Hz, 'v')

%%
% 3) Obtener la respuesta al escalón en tiempo continuo y discreto.
%%

figure;
step(Hs, Hz)

%%
% 4) Aplicar un control PID utilizando la función creada anteriormente. 
%    Obtener señal de control y graficarla.
%%

t = h:h:40;
r = [ zeros(length(t)/2, 1); ones(length(t)/2, 1) ];

pid_data.Kp = 2.6

% Ki = Kp / Ti => Ti = Kp/Ki
pid_data.Ti = pid_data.Kp / (0.08/h)

% Kd = Kp * Td => Td = Kd / Kp
pid_data.Td = 10 * h / pid_data.Kp

pid_data.h = h
pid_data.N = 20
pid_data.b = 1

pid_data.futureI = 0;
pid_data.pastD = 0;
pid_data.pastY = 0;

y_pid = filter_pid(numz, denz, r, pid_data);

figure;
hold on;
stairs(t, r)
stairs(t, y_pid)
legend("Señal de entrada", "Salida controlada");

%%
% 5) Comparar la respuesta al escalón del punto 3 con la respuesta obtenida 
%    aplicando el control PID anterior.
%%

y_open = filter(numz, denz, r);

figure;
hold on;
stairs(t, r)
stairs(t, y_open)
stairs(t, y_pid)
legend("Señal de entrada", "Respuesta natural del sistema", "Salida controlada");
