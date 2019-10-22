im=zeros(16,800);
for i=1:size(im,2)
  v=randi(16);
  im(round(v),i:min(i+10,end))=1;
end
%im=1-im;
convert(im,'rain.h');
