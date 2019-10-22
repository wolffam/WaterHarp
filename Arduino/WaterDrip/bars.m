im=zeros(16,100);
period=20;
for i=1:size(im,2)
  if rem(i,period)==0
    im(:,i)=1;
  end
end
im=1-im;
convert(im,'bars.h');
