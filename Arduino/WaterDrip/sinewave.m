smim=zeros(16,100);
period=50;
for i=1:size(im,2)
  v=8*sin(i*2*pi/period)+8;
  v=min(v,15);
  im(round(v)+1,i)=1;
end
%im=1-im;
convert(im,'sine.h');
