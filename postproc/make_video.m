function make_video(foldername)

drtn = dir(fullfile(foldername,'*.png'));

%%
integers = regexp(foldername,'/( \d+)_(\d+)_(\d+)_(\d+)_(\d+)','match')
%%
rrtn = regexp(foldername,'/(?<alpha>\d+)_(?<N>\d+)_(?<last>\d+)_(?<skip>\d+)','names')
rrtn.alpha
%%
[~,token] = fileparts(foldername);
vname = sprintf('%s.mp4',token)
%%
vd = VideoWriter(vname,'MPEG-4');
vd.open();
%%
N = length(drtn);
%%
for i = 1:N
    img = imread(fullfile(drtn(i).folder,drtn(i).name));
    vd.writeVideo(img);
end
vd.close();
exit();
end