% Mary had a little lamb
notes='EDCDEEEDDDEGGEDCDEEEEDDEDC';
nnums=zeros(size(notes));
for i=1:length(notes)
  nnums(i)=notes(i)-'C'+60;
end
durs=ones(size(nnums));
notes2file('mary.h',nnums,durs);
