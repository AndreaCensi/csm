clear all
close all
disp('hola')

load datos.off -ascii

% ----------
% Try same scans
%scanRef=datos(1,:);
%scanNew=datos(1,:);

% ----------
% Try different scans
scanRef=datos(1,:);
scanNew=datos(2,:);

transf = MbICP(scanRef, scanNew, [0.3,0.3,0.7], false)

