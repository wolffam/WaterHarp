function notes2file(ofile,notes,beatdurs)
% Notes is an array of midi note numbers (0 for rest)
% Durs is an array of durations in beats
framePeriod=0.01;
bpm=120;
middleC=60;
basenote=middleC-2;
durs=beatdurs*60/bpm/framePeriod;   % Sample times
im=zeros(16,sum(durs));
for i=1:length(notes)
  start=sum(durs(1:i-1))+1;
  finish=sum(durs(1:i));
  im((notes(i)-basenote)+1,start:round((start*3+finish)/4))=1;
end
im=1-im;
convert(im,ofile);
end
