%% Control Digital en Sistemas Embebidos - MSE - PRACTICA 1

%% pkginstall -forge package_name

pkg load signal
pkg load control

clc
clear all

R1 = 10e3
C1 = 10e-6
R2 = 1e3
C2 = 10e-6

%%
% 1) Crear una planta mediante los coeficientes del denominador y numerador 
%    en el dominio continuo.
%

%% 
% Opcion 1: Realizar el desarrollo matematico de la ecuación diferencial 
% del circuito y obtener la función transferencia en el dominio de Laplace
% Hs = 1 / ((R1*C1*R2*C2)*s^2 + (R1*C1+R1*C2+R2*C2)*s + 1)
s = tf('s');
Hs_1 = 1 / ((R1*C1*R2*C2)*s^2 + (R1*C1+R1*C2+R2*C2)*s + 1)
[num_1 den_1] = tfdata(Hs_1, 'v')

%% 
% Opcion 2: Realizar el desarrollo matematico de la ecuación diferencial 
% del circuito para obtener los coeficientes del numerador y denominador
num_2 = [1]
den_2 = [(R1*C1*R2*C2) (R1*C1+R1*C2+R2*C2) 1]
Hs_2 = tf(num_2, den_2)

%%
% 2) Crear una planta mediante las matrices de estado (A, B, C y D) que 
%    la definen.
%%

%% 
% Se pueden obtener los coeficientes de las ecuaciones de estado a partir 
% de los coeficientes del numerador y denominador, y la funcion 'tf2ss'.
[A, B, C, D] = tf2ss(num_1, den_1)

% Se pueden obtener los coeficientes del numerador y denominador de la 
% funcion transferencia a partir de los coeficientes de estado con 'ss2tf'.
[num_3, den_3] = ss2tf(A, B, C, D)

% También puede obtenerse la función transferencia a partir de los coeficientes
% de las ecuaciones de estado con: Hs = C (sI − A)^(−1) B + D
Hs_3 = C*(s*eye(length(A))-A)^(-1)*B + D

%%
% 3) Discretizar las anteriores plantas
%%

h = 0.1

%% 
% Opcion 1: Discretizar utilizando la funcion 'c2d'.

Hz_1 = c2d(Hs_1, h, 'zoh')

[numz_1, denz_1] = tfdata(Hz_1, 'v')

[Phi_1, Rho_1, Cz_1, Dz_1] = tf2ss(numz_1, denz_1)

%% 
% Opcion 2: Discretizar utilizando las funciones para obtener las ecuaciones 
% de estado discretas a partir de los coeficientes de las ecuaciones en 
% tiempo continuo.

Phi_2 = expm(A*h)
Rho_2 = (expm(A*h) - expm(A*0))/A*B
Cz_2 = C
Dz_2 = 0

[numz_2, denz_2] = ss2tf(Phi_2, Rho_2, Cz_2, Dz_2)

%%
% 4) Evaluar la distribución de polos y ceros de las plantas continuas y 
%    discretas, también su respuesta en frecuencia.
%%

figure;
bode(Hs_1)

zero(Hs_1)
pole(Hs_1)

[Zz, Pz, k] = tf2zp(numz_1, denz_1)

figure;
pzmap(Hz_1)


%%
% 5) Evaluar la respuesta al escalón de todas las plantas (continuas y 
%    discretas).
%%

figure;
step(Hs_1, Hz_1)

%%
% 6) Crear una función que aplique la ecuación de recurrencia de un filtro a 
%    una entrada cualquiera. (Ver función filter de Matlab).
%%

function [y] = funcion_recurrencia(b, a, u)
  n_inicial = length(a) - 1;
  y = zeros(length(u) + n_inicial, 1);
  u = [ zeros(n_inicial, 1); u ];
  
  for k = n_inicial + 1 : 1 : length(u)
    
    for m = 1 : 1 : length(b)
      y(k) = y(k) + b(m) * u(k-m+1);
    end
    
    for n = 2 : 1 : length(a)
      y(k) = y(k) - a(n) * y(k-n+1);
    end
  
    y(k) = y(k) / a(1);
  end
  
  y = y(n_inicial+1:end);
end

%%
% 7) Compare las respuestas al escalón discretas con el comando step frente a 
%    la creada con la ecuación de recurrencia
%%

T = 40;
t = 1:h:T-h;

u = [ zeros(length(t)/2, 1); ones(length(t)/2, 1) ];

y_1 = funcion_recurrencia(numz_1, denz_1, u);
y_2 = filter(numz_1, denz_1, u);

figure;
hold on;
stairs(t, u, 'r', 'LineWidth', 2);
stairs(t, y_1, 'g', 'LineWidth', 2);
stairs(t, y_2, 'b', 'LineWidth', 2);
title('Respuesta al Escalon');
legend('u(t)', 'y1(t)', 'y2(t)');