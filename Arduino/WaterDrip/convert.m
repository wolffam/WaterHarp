function convert(file,ofile)
  if ischar(file)
    im=imread(file);
  else
    im=file;
  end
  assert(size(im,1)==16);
  setfig('im');clf;
  x=im;
  x(end+1,:)=nan;
  x(:,end+1)=nan;
  pcolor(x);
  shading flat
  axis ij;
  %  imshow(im);
  fd=fopen(ofile,'w');
  fprintf(fd,'unsigned short %s[]={\n',strrep(ofile,'.h',''));
  for i=1:size(im,2)
    v=0;
    for j=1:16
      v=v*2+im(j,i);
    end
    fprintf(fd,'0x%04x',v);
    if i<size(im,2)
      fprintf(fd,',\n');
    end
  end
  fprintf(fd,"};\n");
  fclose(fd);
end
