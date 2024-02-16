f0 = 141;
FS = 44100; %Must always be 44100
omega = 2*pi*f0/FS;
BW = 1;

alpha = sin(omega) *sinh(((log(2)/2)*BW*omega/sin(omega)));
Qinv = 2*sinh(log(2)/2*BW);

b0 = alpha;
b1 = 0;
b2 = -alpha;
a0 = 1 + alpha;
a1 = -2*cos(omega);
a2 = 1 - alpha;

tranFunc = tf([b0,b1,b2],[a0,a1,a2]);
%s = (1/tan(omega/2))*tf([1, -1],[1,1])

%digFuncDigital = (Qinv*s/(s*s + 1 + s*Qinv))
%digFuncAnalog = tf([(Qinv*omega),0],[1,(Qinv*omega),(omega*omega)])
%pole(digFuncDigital)
b = [b0,b1,b2];
a = [a0,a1,a2];
freqz(b,a,44100);
%bode(digFuncDigital)
%hold on
%bode(digFuncAnalog)