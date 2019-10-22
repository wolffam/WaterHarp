function staff(ofile,bpm,tsnum,tsdec)
  framePeriod = 0.010;   % 10msec/frame
  period=4;
  nframes=period/framePeriod;
  barwidth=5;
  dotspacing=10;
  im=zeros(16,nframes);
  im(4:2:12,1:dotspacing:end)=1;
  im(4:12,1:barwidth)=1;
  im=1-im;
  convert(im,ofile);
end
