function convert(file,ofile)
  im=imread(file);
  assert(size(im,1)==16);
  setfig('im');clf;
  imshow(im);
  fd=fopen(ofile,'w');
  fprintf(fd,"unsigned short shd[]={\n");
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
